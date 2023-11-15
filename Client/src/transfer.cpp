#include <iostream>
#include <mutex>
#include "reliable-udp.hpp"
#include "networking.hpp"
#include "transfer.hpp"

std::mutex send_mutex;
std::mutex receive_mutex;

void send_input(struct networking_options& networkingOptions, volatile int& exit_flag) {
    while (!exit_flag) {
//        std::lock_guard<std::mutex> lock(send_mutex); // Lock the mutex

        std::string input;

        // Read a line of input from the terminal
        std::getline(std::cin, input);

        networkingOptions.header->sequence_number++;
        // Set the data length field in the header to the length of the input
        networkingOptions.header->data_length = input.length();
        // Set the data field in the header to the input
        networkingOptions.header->data = input;

        // Loop send_packet until it is able to be sent
        int ret_status;
        while ((ret_status = send_packet(networkingOptions)) != 0) {
            if (exit_flag) {
                return;
            }
            if (ret_status == 1) {
                std::cout << "Window Size Reached." << std::endl;
            } else {
                std::cerr << "Failed to Send." << std::endl;
            }
        }
    }
}


void read_response(struct networking_options& networkingOptions, volatile int& exit_flag) {
    while (!exit_flag) {
        {
//            std::lock_guard<std::mutex> lock(receive_mutex); // Lock the mutex

            // Call receive_acknowledgements
            receive_acknowledgements(networkingOptions);
        }
    }
}
