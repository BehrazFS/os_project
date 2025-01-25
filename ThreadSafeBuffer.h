//
// Created by behraz on 12/29/2024.
//

#ifndef THREADSAFEBUFFER_H
#define THREADSAFEBUFFER_H

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;
// just for simplifying the size types :)
typedef unsigned long long int ulli;

template<typename T>

class ThreadSafeBuffer {
    string name;
    std::queue<T> buffer;
    ulli b_size;
    mutex mtx;
    condition_variable not_empty;
    condition_variable not_full;
    bool logEn = false;

public:
    // Constructor: Initializes the buffer with a given size
    explicit ThreadSafeBuffer(const ulli size , const string& name , bool logEn): name(name), b_size(size) {
        this->logEn = logEn;
        if (this->b_size <= 0) {
            throw invalid_argument("Buffer size must be greater than 0");

        }
    }

    // Add an element to the buffer
    void add(const T &item) {
        unique_lock<mutex> lock(mtx);
        not_full.wait(lock, [this]() { return buffer.size() < b_size; });
        buffer.push(item);
        if (logEn) {
            cout << "Added: " << item << " to buffer (size: " << buffer.size() << ")\n";
        }
        not_empty.notify_one();
    }

    // Add an element to the buffer (dropped if buffer is full)
    void add_drop(const T &item) {
        std::unique_lock<mutex> lock(mtx);
        if (buffer.size() >= b_size) {
            if (logEn) {
                cout << "Buffer full! Dropping data: " << item << "\n";
            }
            not_empty.notify_one();
            return;
        }
        buffer.push(item);
        if (logEn) {
            cout << "Added: " << item << " to buffer (size: " << buffer.size() << ")\n";
        }
        not_empty.notify_one();
    }

    // Remove and return an element from the buffer
    T remove() {
        std::unique_lock<mutex> lock(mtx);
        not_empty.wait(lock, [this]() { return !buffer.empty(); });
        T item = buffer.front();
        buffer.pop();
        if (logEn) {
            cout << "Removed: " << item << " from buffer (size: " << buffer.size() << ")\n";
        }
        not_full.notify_one();
        return item;
    }
    // Remove and return an element from the buffer (no wait if buffer is empty)
    T remove_no_wait() {
        std::unique_lock<mutex> lock(mtx);
        if (buffer.empty()) {
            if (logEn) {
                cout << "Buffer empty! no data"  << "\n";
            }
            not_full.notify_one();
            return "-1";
        }
        T item = buffer.front();
        buffer.pop();
        if (logEn) {
            cout << "Removed: " << item << " from buffer (size: " << buffer.size() << ")\n";
        }
        not_full.notify_one();
        return item;
    }
    // Check if the buffer is empty
    bool is_empty() {
        lock_guard<mutex> lock(mtx);
        return buffer.empty();
    }

    // Check if the buffer is full
    bool is_full() {
        lock_guard<mutex> lock(mtx);
        return buffer.size() >= b_size;
    }

    // Get the current size of the buffer
    ulli size() {
        std::lock_guard<mutex> lock(mtx);
        return buffer.size();
    }

    // Get the maximum size of the buffer
    [[nodiscard]] ulli max_size() const {
        return this->b_size;
    }
};


#endif //THREADSAFEBUFFER_H
