#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <semaphore>
#include <chrono>
#include <random>
#include <atomic>
#include <condition_variable>

using namespace std;
using namespace chrono;

struct DataPacket {
    int id;                 
    int priority;           
    int size;               
    bool isCritical;        
    int stationId;          
    bool operator<(const DataPacket& other) const {
        if (isCritical != other.isCritical)
            return !isCritical;
        return priority > other.priority;
    }
};
const int INITIAL_SERVERS = 3;           
const int MAX_SERVERS = 6;               
const int TOTAL_STATIONS = 10;           
const int PACKETS_PER_STATION = 5;       
const double LOAD_THRESHOLD = 0.8;       

atomic<int> activeServers(INITIAL_SERVERS);     
atomic<int> totalActivePackets(0);             
atomic<int> packetCounter(0);                   
atomic<int> packetsSent(0);                     
atomic<int> packetsProcessed(0);                
atomic<bool> systemRunning(true);               
atomic<bool> emergencyMode(false);              

counting_semaphore<MAX_SERVERS> serverSem(INITIAL_SERVERS);
mutex consoleMutex;        
mutex queueMutex;          
priority_queue<DataPacket> packetQueue; 

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> priorityDist(1, 10);      
uniform_int_distribution<> sizeDist(10, 100);        
uniform_int_distribution<> criticalChance(1, 10);    
void processPacket(DataPacket packet) {
    serverSem.acquire();
    totalActivePackets++;
    {
        lock_guard<mutex> lock(consoleMutex);
        cout << "[СЕРВЕР] Обрабатывает пакет #" << packet.id
            << " | приоритет: " << packet.priority
            << (packet.isCritical ? " | КРИТИЧЕСКИЙ" : "")
            << " | станция: " << packet.stationId
            << " | размер: " << packet.size << " усл.ед." << endl;
    }
    int processTime = packet.size / 20 + 1; 
    this_thread::sleep_for(seconds(processTime));
    {
        lock_guard<mutex> lock(consoleMutex);
        cout << "[ГОТОВО] Пакет #" << packet.id << " обработан за "
            << processTime << " сек" << endl;
    }
    totalActivePackets--;
    packetsProcessed++;
    serverSem.release();  
}
void monitorLoad() {
    while (systemRunning) {
        this_thread::sleep_for(seconds(3));
        int currentServers = activeServers.load();
        int activePackets = totalActivePackets.load();
        double load = (double)activePackets / (currentServers * 2);
        {
            lock_guard<mutex> lock(consoleMutex);
            cout << "\n [МОНИТОРИНГ] Загрузка: " << (load * 100) << "%"
                << " | Активных пакетов: " << activePackets
                << " | Серверов: " << currentServers
                << " | Обработано: " << packetsProcessed.load()
                << " / " << packetsSent.load() << endl;
        }
        if (load > LOAD_THRESHOLD && currentServers < MAX_SERVERS) {
            {
                lock_guard<mutex> lock(consoleMutex);
                cout << " [АДАПТАЦИЯ] Загрузка превысила 80%!" << endl;
                cout << " Включаем дополнительный сервер..." << endl;
            }

            activeServers++;
            serverSem.release();  
        }
        if (load > 1.2 && !emergencyMode) {
            emergencyMode = true;
            {
                lock_guard<mutex> lock(consoleMutex);
                cout << " [АВАРИЯ] Критическая перегрузка системы!" << endl;
                cout << " Низкоприоритетные данные будут отброшены" << endl;
            }
            lock_guard<mutex> lock(queueMutex);
            priority_queue<DataPacket> newQueue;
            int dropped = 0;
            while (!packetQueue.empty()) {
                DataPacket p = packetQueue.top();
                packetQueue.pop();
                if (p.isCritical || p.priority <= 3) {
                    newQueue.push(p);
                }
                else {
                    dropped++;
                    lock_guard<mutex> lock(consoleMutex);
                    cout << " Отброшен пакет #" << p.id
                        << " (приоритет: " << p.priority << ")" << endl;
                }
            }
            packetQueue = newQueue;
            {
                lock_guard<mutex> lock(consoleMutex);
                cout << "Отброшено пакетов: " << dropped << endl;
            }
            this_thread::sleep_for(seconds(2));
            emergencyMode = false;
            {
                lock_guard<mutex> lock(consoleMutex);
                cout << " Аварийный режим завершен" << endl;
            }
        }
    }
}
void distributePackets() {
    while (systemRunning) {
        DataPacket packet;
        bool hasPacket = false;
        {
            lock_guard<mutex> lock(queueMutex);
            if (!packetQueue.empty()) {
                packet = packetQueue.top();
                packetQueue.pop();
                hasPacket = true;
            }
        }
        if (hasPacket) {
            thread t(processPacket, packet);
            t.detach();
        }
        else {
            this_thread::sleep_for(milliseconds(100));
        }
    }
}
void monitoringStation(int stationId) {
    for (int i = 0; i < PACKETS_PER_STATION; i++) {
        if (!systemRunning) break;
        DataPacket packet;
        packet.id = packetCounter++;
        packet.priority = priorityDist(gen);
        packet.size = sizeDist(gen);
        packet.stationId = stationId;
        packet.isCritical = (criticalChance(gen) <= 2) || (packet.priority <= 2);
        {
            lock_guard<mutex> lock(queueMutex);
            packetQueue.push(packet);
        }
        packetsSent++;
        {
            lock_guard<mutex> lock(consoleMutex);
            cout << " [СТАНЦИЯ " << stationId << "] Отправлен пакет #" << packet.id
                << " | приоритет: " << packet.priority
                << (packet.isCritical ? " [КРИТИЧЕСКИЙ]" : "")
                << " | размер: " << packet.size << " усл.ед." << endl;
        }
        int delay = 500; 
        if (stationId == 5) {
            delay = 200;
        }
        this_thread::sleep_for(milliseconds(delay));
    }
    {
        lock_guard<mutex> lock(consoleMutex);
        cout << " [СТАНЦИЯ " << stationId << "] Завершила отправку пакетов" << endl;
    }
}
int main() {
    setlocale(LC_ALL, "Russian");
    cout << "АДАПТИВНАЯ СИСТЕМА МОНИТОРИНГА ЭНЕРГОСЕТИ" << endl;
    cout << "Пакетов от каждой станции: " << PACKETS_PER_STATION << endl;
    cout << "Всего пакетов: " << TOTAL_STATIONS * PACKETS_PER_STATION << endl;
    cout << "Начальное количество серверов: " << INITIAL_SERVERS << endl;
    cout << "Максимум серверов: " << MAX_SERVERS << endl;
    cout << "Порог загрузки для добавления сервера: 80%" << endl;
    cout << "Станция #5 создает повышенную нагрузку" << endl;
    thread monitor(monitorLoad);
    vector<thread> distributors;
    for (int i = 0; i < 3; i++) {
        distributors.emplace_back(distributePackets);
    }
    vector<thread> stations;
    for (int i = 1; i <= TOTAL_STATIONS; i++) {
        stations.emplace_back(monitoringStation, i);
        this_thread::sleep_for(milliseconds(20));
    }
    for (auto& t : stations) {
        t.join();
    }

    cout << "\n Все станции завершили отправку пакетов" << endl;
    cout << " Ожидаем обработки оставшихся пакетов..." << endl;
    while (packetsProcessed.load() < packetsSent.load()) {
        this_thread::sleep_for(seconds(1));
        {
            lock_guard<mutex> lock(consoleMutex);
            cout << "Обработано: " << packetsProcessed.load()
                << " / " << packetsSent.load() << endl;
        }
    }

    systemRunning = false;
    this_thread::sleep_for(seconds(2));
    for (auto& t : distributors) {
        if (t.joinable()) t.detach();
    }
    if (monitor.joinable()) monitor.detach();
    cout << "СИСТЕМА ОСТАНОВЛЕНА" << endl;
    cout << "Всего отправлено пакетов: " << packetsSent.load() << endl;
    cout << "Всего обработано пакетов: " << packetsProcessed.load() << endl;
    return 0;
}