#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include "networking.hpp"
#include "transfer.hpp"

std::mutex iMutex; // Mutex to protect access to the shared variable i
int64_t i = 0;

void send_input(struct networking_options& networkingOptions, volatile int& exit_flag) {
    while (!exit_flag) {
        {
            std::lock_guard<std::mutex> lock(iMutex); // Lock the mutex
            std::cout << "Thread 1 is running, iteration " << i << std::endl;
            i++;
        } // Automatically unlocks the mutex when the lock_guard goes out of scope

        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Simulate some work outside the critical section
    }
}

void read_response(struct networking_options& networkingOptions, volatile int& exit_flag) {
    while (!exit_flag) {
        {
            std::lock_guard<std::mutex> lock(iMutex); // Lock the mutex
            std::cout << "Thread 2 is running, iteration " << i << std::endl;
            i++;
        } // Automatically unlocks the mutex when the lock_guard goes out of scope

        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Simulate some work outside the critical section
    }
}
