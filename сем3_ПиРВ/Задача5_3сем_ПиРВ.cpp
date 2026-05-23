#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <clocale>
#include <windows.h>

using namespace std;

void bubbleSort(vector<int>& arr) {
    int n = arr.size();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]);
            }
        }
    }
}

void insertionSort(vector<int>& arr) {
    int n = arr.size();
    for (int i = 1; i < n; ++i) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

void merge(vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<int> L(n1), R(n2);

    for (int i = 0; i < n1; ++i)
        L[i] = arr[left + i];
    for (int j = 0; j < n2; ++j)
        R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        }
        else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

void mergeSort(vector<int>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

void mergeSortWrapper(vector<int>& arr) {
    mergeSort(arr, 0, arr.size() - 1);
}

int partition(vector<int>& arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; ++j) {
        if (arr[j] <= pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[high]);
    return i + 1;
}

void quickSort(vector<int>& arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

void quickSortWrapper(vector<int>& arr) {
    quickSort(arr, 0, arr.size() - 1);
}

long long measureTime(void (*sortFunc)(vector<int>&), vector<int> arr) {
    auto start = chrono::high_resolution_clock::now();
    sortFunc(arr);
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    const int SIZE = 100000;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(1, 10000);

    vector<int> original(SIZE);
    for (int i = 0; i < SIZE; ++i) {
        original[i] = dist(gen);
    }

    cout << "=== Сравнение алгоритмов сортировки ===" << endl;
    cout << "Размер массива: " << SIZE << " элементов" << endl;
    cout << endl;

    cout << "Пузырьковая сортировка (Bubble Sort): ";
    cout << measureTime(bubbleSort, original) << " миллисекунд" << endl;

    cout << "Сортировка вставками (Insertion Sort): ";
    cout << measureTime(insertionSort, original) << " миллисекунд" << endl;

    cout << "Сортировка слиянием (Merge Sort): ";
    cout << measureTime(mergeSortWrapper, original) << " миллисекунд" << endl;
    cout << "Быстрая сортировка (Quick Sort): ";
    cout << measureTime(quickSortWrapper, original) << " миллисекунд" << endl;

    cout << "Стандартная сортировка (std::sort): ";
    cout << measureTime([](vector<int>& arr) { sort(arr.begin(), arr.end()); }, original) << " миллисекунд" << endl;

    return 0;
}
