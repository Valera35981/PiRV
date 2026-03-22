#include <iostream>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <condition_variable>

using namespace std;

class Semaphore {
private:
    mutex mtx;
    condition_variable cv;
    int count;

public:
    Semaphore(int initial) : count(initial) {}

    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

    void acquire() {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this]() { return count > 0; });
        count--;
    }

    bool try_acquire_for(chrono::milliseconds timeout) {
        unique_lock<mutex> lock(mtx);
        if (cv.wait_for(lock, timeout, [this]() { return count > 0; })) {
            count--;
            return true;
        }
        return false;
    }

    void release() {
        unique_lock<mutex> lock(mtx);
        count++;
        cv.notify_one();
    }

    int get_count() {
        lock_guard<mutex> lock(mtx);
        return count;
    }
};

struct FileChunk {
    int chunk_id;
    int file_id;
    size_t size;

    void download() {
        this_thread::sleep_for(chrono::milliseconds(100 + rand() % 200));
    }
};

struct FileDownload {
    int file_id;
    vector<FileChunk> chunks;
    atomic<int> downloaded_chunks{ 0 };

    FileDownload() : file_id(0) {}

    FileDownload(const FileDownload& other) : file_id(other.file_id), chunks(other.chunks), downloaded_chunks(other.downloaded_chunks.load()) {}

    bool is_complete() const {
        return downloaded_chunks.load() == chunks.size();
    }

    void mark_chunk_downloaded() {
        downloaded_chunks++;
    }
};

class DownloadManager {
private:
    queue<FileChunk> download_queue;
    Semaphore active_downloads;
    Semaphore chunk_downloads;
    mutex queue_mutex;
    atomic<int> completed_files{ 0 };
    vector<FileDownload> files;
    mutex files_mutex;

public:
    DownloadManager(int max_files, int max_chunks) : active_downloads(max_files), chunk_downloads(max_chunks) {}

    void add_file(const FileDownload& file) {
        {
            lock_guard<mutex> lock(files_mutex);
            files.push_back(file);
        }

        lock_guard<mutex> lock(queue_mutex);
        for (const auto& chunk : file.chunks) {
            download_queue.push(chunk);
        }

        cout << "Файл " << file.file_id << " добавлен. Всего частей: " << file.chunks.size() << endl;
    }

    void download_worker() {
        thread::id thread_id = this_thread::get_id();

        while (true) {
            FileChunk current_chunk;
            bool has_chunk = false;

            {
                lock_guard<mutex> lock(queue_mutex);
                if (!download_queue.empty()) {
                    current_chunk = download_queue.front();
                    download_queue.pop();
                    has_chunk = true;
                }
            }

            if (!has_chunk) {
                this_thread::sleep_for(chrono::milliseconds(100));
                continue;
            }

            if (active_downloads.try_acquire_for(chrono::milliseconds(500))) {
                if (chunk_downloads.try_acquire_for(chrono::milliseconds(500))) {

                    cout << "Thread: " << thread_id
                        << " | Файл " << current_chunk.file_id
                        << " | Часть " << current_chunk.chunk_id
                        << " | НАЧАЛО загрузки (размер: " << current_chunk.size << ")" << endl;

                    process_chunk(current_chunk);

                    chunk_downloads.release();

                    cout << "Thread: " << thread_id
                        << " | Файл " << current_chunk.file_id
                        << " | Часть " << current_chunk.chunk_id
                        << " | ЗАВЕРШЕНА" << endl;

                    {
                        lock_guard<mutex> lock(files_mutex);
                        for (auto& file : files) {
                            if (file.file_id == current_chunk.file_id) {
                                file.mark_chunk_downloaded();

                                if (file.is_complete()) {
                                    completed_files++;
                                    cout << "Файл " << current_chunk.file_id << " полностью скачан!" << endl;
                                }
                                break;
                            }
                        }
                    }

                    active_downloads.release();
                }
                else {
                    active_downloads.release();

                    lock_guard<mutex> lock(queue_mutex);
                    download_queue.push(current_chunk);

                    cout << "Thread: " << thread_id
                        << " | Файл " << current_chunk.file_id
                        << " | Часть " << current_chunk.chunk_id
                        << " | НЕ УДАЛОСЬ (лимит частей)" << endl;
                }
            }
            else {
                lock_guard<mutex> lock(queue_mutex);
                download_queue.push(current_chunk);

                cout << "Thread: " << thread_id
                    << " | Файл " << current_chunk.file_id
                    << " | Часть " << current_chunk.chunk_id
                    << " | НЕ УДАЛОСЬ (лимит файлов)" << endl;
            }

            this_thread::yield();
        }
    }

    inline void process_chunk(FileChunk& chunk) {
        chunk.download();
    }

    int get_completed_files() const {
        return completed_files.load();
    }
};

void downloader(DownloadManager& dm, int id) {
    dm.download_worker();
}

int main() {
    setlocale(LC_ALL, "ru_RU");
    srand((unsigned int)time(nullptr));

    DownloadManager dm(2, 3);

    cout << "Менеджер загрузок" << endl;
    cout << "Максимум одновременно скачиваемых файлов: 2" << endl;
    cout << "Максимум одновременно скачиваемых частей: 3" << endl << endl;

    vector<FileDownload> files;

    for (int f = 1; f <= 3; f++) {
        FileDownload file;
        file.file_id = f;

        int num_chunks = 4 + rand() % 3;
        for (int c = 0; c < num_chunks; c++) {
            FileChunk chunk;
            chunk.chunk_id = c;
            chunk.file_id = f;
            chunk.size = 1024 * (1 + rand() % 5);
            file.chunks.push_back(chunk);
        }

        files.push_back(file);
        dm.add_file(file);
    }

    cout << endl << "Запуск загрузчиков" << endl;

    vector<thread> workers;
    for (int i = 0; i < 3; i++) {
        workers.emplace_back(downloader, ref(dm), i);
    }

    for (auto& w : workers) {
        w.detach();
    }

    this_thread::sleep_for(chrono::milliseconds(8000));

    cout << endl << "Статистика" << endl;
    cout << "Полностью скачано файлов: " << dm.get_completed_files() << " из " << files.size() << endl;

    return 0;
}