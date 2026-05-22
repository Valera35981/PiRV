#include <iostream>
#include <vector>
#include <algorithm>
#include <execution>
#include <numeric>
#include <random>
#include <chrono>
#include <mutex>
#include <atomic>
#include <future>
#include <cmath>
#include <iomanip>
#include <thread>
#include <condition_variable>
#include <queue>
#include <windows.h>

using namespace std;

const int NUM_SENSORS = 100;
const int NUM_BATCHES = 10;
const int READINGS_PER_SENSOR = 5;
const int NUM_PROCESSING_THREADS = 4;

struct SensorData {
    int sensor_id;
    double temperature;
    double pressure;
    double vibration;
    chrono::steady_clock::time_point timestamp;
};

class Sensor {
    int id;
    mt19937 rng;
    uniform_real_distribution<> temp_dist{ 20.0, 100.0 };
    uniform_real_distribution<> press_dist{ 0.8, 6.0 };
    uniform_real_distribution<> vib_dist{ 0.0, 5.0 };
public:
    explicit Sensor(int id) : id(id), rng(random_device{}()) {}
    SensorData read() {
        return SensorData{ id, temp_dist(rng), press_dist(rng), vib_dist(rng), chrono::steady_clock::now() };
    }
};

struct ProcessedMetrics {
    double avg_temperature = 0.0;
    double max_pressure = 0.0;
    double total_vibration = 0.0;
    double energy_score = 0.0;
    double prefix_sum_last = 0.0;
};

thread_local vector<double> tls_temps;
thread_local vector<double> tls_pressures;
thread_local vector<double> tls_vibrations;
thread_local vector<double> tls_prefix;

template<typename T>
class ThreadSafeQueue {
    mutex mtx;
    queue<T> queue;
    condition_variable cv;
public:
    void push(T value) {
        {
            lock_guard<mutex> lock(mtx);
            queue.push(move(value));
        }
        cv.notify_one();
    }

    bool pop(T& value) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this] { return !queue.empty(); });
        value = move(queue.front());
        queue.pop();
        return true;
    }

    bool try_pop(T& value) {
        lock_guard<mutex> lock(mtx);
        if (queue.empty()) return false;
        value = move(queue.front());
        queue.pop();
        return true;
    }

    size_t size() {
        lock_guard<mutex> lock(mtx);
        return queue.size();
    }

    bool empty() {
        lock_guard<mutex> lock(mtx);
        return queue.empty();
    }
};

class DataProcessor {
public:
    static ProcessedMetrics process_batch(const vector<SensorData>& batch) {
        if (batch.empty()) return {};
        ProcessedMetrics m;

        tls_temps.clear();
        tls_temps.reserve(batch.size());
        tls_pressures.clear();
        tls_pressures.reserve(batch.size());
        tls_vibrations.clear();
        tls_vibrations.reserve(batch.size());

        for (size_t i = 0; i < batch.size(); ++i) {
            tls_temps.push_back(batch[i].temperature);
            tls_pressures.push_back(batch[i].pressure);
            tls_vibrations.push_back(batch[i].vibration);
        }

        double temp_sum = reduce(execution::par_unseq, tls_temps.begin(), tls_temps.end(), 0.0);
        m.avg_temperature = temp_sum / batch.size();

        m.max_pressure = *max_element(execution::par_unseq, tls_pressures.begin(), tls_pressures.end());

        m.total_vibration = reduce(execution::par_unseq, tls_vibrations.begin(), tls_vibrations.end(), 0.0);

        double energy_sum = transform_reduce(execution::par_unseq, batch.begin(), batch.end(), 0.0, plus<double>(),
            [](const SensorData& d) { return d.temperature * d.pressure / (d.vibration + 0.1); });
        m.energy_score = energy_sum / batch.size();

        tls_prefix.resize(tls_temps.size());
        inclusive_scan(execution::par_unseq, tls_temps.begin(), tls_temps.end(), tls_prefix.begin(), plus<double>());
        m.prefix_sum_last = tls_prefix.empty() ? 0.0 : tls_prefix.back();

        return m;
    }

    static vector<SensorData> filter_critical(const vector<SensorData>& data) {
        vector<SensorData> result(data.size());
        auto it = copy_if(execution::par_unseq, data.begin(), data.end(), result.begin(),
            [](const SensorData& d) { return d.temperature > 85.0 || d.pressure > 5.0 || d.vibration > 4.0; });
        result.resize(distance(result.begin(), it));

        sort(execution::par, result.begin(), result.end(),
            [](const SensorData& a, const SensorData& b) { return a.timestamp < b.timestamp; });

        return result;
    }

    static double calculate_health_index(const SensorData& d) {
        double tf = max(0.0, 1.0 - (d.temperature - 20.0) / 80.0);
        double pf = max(0.0, 1.0 - (d.pressure - 0.8) / 5.2);
        double vf = max(0.0, 1.0 - d.vibration / 5.0);
        return (tf + pf + vf) / 3.0 * 100.0;
    }
};

class ResultAggregator {
    mutex metrics_mutex;
    mutex critical_mutex;
    ProcessedMetrics cumulative;
    atomic<size_t> processed_count{ 0 };
    vector<SensorData> all_critical;

public:
    void add_batch_result(const ProcessedMetrics& m, const vector<SensorData>& critical) {
        processed_count++;
        {
            lock_guard<mutex> lock(metrics_mutex);
            size_t count = processed_count.load();
            cumulative.avg_temperature = (cumulative.avg_temperature * (count - 1) + m.avg_temperature) / count;
            cumulative.max_pressure = max(cumulative.max_pressure, m.max_pressure);
            cumulative.total_vibration += m.total_vibration;
            cumulative.energy_score = (cumulative.energy_score * (count - 1) + m.energy_score) / count;
            cumulative.prefix_sum_last += m.prefix_sum_last;
        }
        if (!critical.empty()) {
            lock_guard<mutex> lock(critical_mutex);
            all_critical.insert(all_critical.end(), critical.begin(), critical.end());
        }
    }

    void print_summary() {
        size_t count = processed_count.load();
        ProcessedMetrics cum;
        vector<SensorData> crit;

        {
            lock_guard<mutex> lock(metrics_mutex);
            cum = cumulative;
        }
        {
            lock_guard<mutex> lock(critical_mutex);
            crit = all_critical;
        }

        cout << "\n========================================" << endl;
        cout << "         СВОДКА ПО ТЕЛЕМЕТРИИ           " << endl;
        cout << "========================================" << endl;
        cout << "Обработано пакетов: " << count << endl;
        cout << "Критических событий: " << crit.size() << endl << endl;

        cout << fixed << setprecision(2);
        cout << "Средняя температура: " << cum.avg_temperature << " °C" << endl;
        cout << "Максимальное давление: " << cum.max_pressure << " атм" << endl;
        cout << "Суммарная вибрация: " << cum.total_vibration << " мм/с" << endl;
        cout << "Энергетический показатель: " << cum.energy_score << endl;
        cout << "Префиксная сумма (inclusive_scan): " << cum.prefix_sum_last << endl;

        if (!crit.empty()) {
            cout << "\n========================================" << endl;
            cout << "     ПОСЛЕДНИЕ КРИТИЧЕСКИЕ СОБЫТИЯ     " << endl;
            cout << "========================================" << endl;
            size_t show = min(size_t(5), crit.size());
            for (size_t i = crit.size() - show; i < crit.size(); ++i) {
                const auto& e = crit[i];
                cout << "Датчик #" << setw(3) << e.sensor_id
                    << " | T=" << setw(6) << e.temperature << " °C"
                    << " | P=" << setw(5) << e.pressure << " атм"
                    << " | V=" << setw(5) << e.vibration << " мм/с"
                    << " | Здоровье=" << setw(3) << setprecision(0)
                    << DataProcessor::calculate_health_index(e) << "%" << endl;
            }
        }
        cout << "========================================" << endl;
    }
};

class Barrier {
    mutex mtx;
    condition_variable cv;
    size_t threshold;
    size_t count;
    size_t generation;
public:
    explicit Barrier(size_t n) : threshold(n), count(n), generation(0) {}

    void arrive_and_wait() {
        unique_lock<mutex> lock(mtx);
        size_t gen = generation;
        if (--count == 0) {
            generation++;
            count = threshold;
            cv.notify_all();
        }
        else {
            cv.wait(lock, [this, gen] { return gen != generation; });
        }
    }
};

class ProcessingPool {
    vector<thread> workers;
    ThreadSafeQueue<pair<vector<SensorData>, promise<ProcessedMetrics>>> task_queue;
    atomic<bool> stop{ false };
    Barrier sync_barrier;

public:
    ProcessingPool(size_t num_threads) : sync_barrier(num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] { worker_loop(); });
        }
    }

    ~ProcessingPool() {
        stop = true;
        for (auto& w : workers) {
            if (w.joinable()) w.join();
        }
    }

    future<ProcessedMetrics> submit(vector<SensorData> data) {
        promise<ProcessedMetrics> p;
        auto f = p.get_future();
        task_queue.push({ move(data), move(p) });
        return f;
    }

private:
    void worker_loop() {
        while (!stop) {
            pair<vector<SensorData>, promise<ProcessedMetrics>> task;
            if (task_queue.try_pop(task)) {
                auto metrics = DataProcessor::process_batch(task.first);
                sync_barrier.arrive_and_wait();  // Фаза 1
                task.second.set_value(metrics);
                sync_barrier.arrive_and_wait();  // Фаза 2
            }
            else {
                this_thread::sleep_for(chrono::milliseconds(1));
            }
        }
    }
};

class Dispatcher {
    ThreadSafeQueue<vector<SensorData>> input_queue;
    ProcessingPool& pool;
    atomic<bool> running{ true };
    thread dispatcher_thread;
    vector<future<ProcessedMetrics>> pending_results;

public:
    Dispatcher(ProcessingPool& p) : pool(p) {
        dispatcher_thread = thread([this] { dispatch_loop(); });
    }

    ~Dispatcher() {
        running = false;
        if (dispatcher_thread.joinable()) dispatcher_thread.join();
    }

    void submit_batch(vector<SensorData> data) {
        input_queue.push(move(data));
    }

    void collect_results(ResultAggregator& aggregator) {
        for (auto& f : pending_results) {
            if (f.wait_for(chrono::seconds(0)) == future_status::ready) {
                auto metrics = f.get();
                aggregator.add_batch_result(metrics, {});
            }
        }
    }

private:
    void dispatch_loop() {
        while (running) {
            vector<SensorData> batch;
            if (input_queue.try_pop(batch)) {
                auto future = pool.submit(move(batch));
                pending_results.push_back(move(future));
            }
            else {
                this_thread::sleep_for(chrono::milliseconds(1));
            }
        }
    }
};

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    cout << "========================================" << endl;
    cout << "   СИСТЕМА АНАЛИЗА ТЕЛЕМЕТРИИ      " << endl;
    cout << "========================================" << endl << endl;

    vector<Sensor> sensors;
    for (int i = 0; i < NUM_SENSORS; ++i) {
        sensors.emplace_back(i);
    }

    unsigned int num_cores = thread::hardware_concurrency();
    cout << "Доступно логических ядер: " << num_cores << endl;
    cout << "Потоков обработки: " << NUM_PROCESSING_THREADS << endl << endl;

    ProcessingPool pool(NUM_PROCESSING_THREADS);
    Dispatcher dispatcher(pool);
    ResultAggregator aggregator;

    for (int batch = 0; batch < NUM_BATCHES; ++batch) {
        cout << "Обработка пакета " << (batch + 1) << "/" << NUM_BATCHES << "..." << endl;
        vector<future<vector<SensorData>>> futures;
        for (int i = 0; i < NUM_SENSORS; ++i) {
            futures.push_back(async(launch::async, [&sensors, i]() {
                vector<SensorData> result;
                result.reserve(READINGS_PER_SENSOR);
                for (int j = 0; j < READINGS_PER_SENSOR; ++j) {
                    result.push_back(sensors[i].read());
                }
                return result;
                }));
        }

        vector<SensorData> raw_data;
        raw_data.reserve(NUM_SENSORS * READINGS_PER_SENSOR);
        for (auto& f : futures) {
            auto data = f.get();
            raw_data.insert(raw_data.end(), data.begin(), data.end());
        }

        auto metrics = DataProcessor::process_batch(raw_data);
        auto critical = DataProcessor::filter_critical(raw_data);

        aggregator.add_batch_result(metrics, critical);

        this_thread::sleep_for(chrono::milliseconds(50));
    }
    aggregator.print_summary();
    cout << "\nСистема успешно завершила работу." << endl;
    return 0;
} 