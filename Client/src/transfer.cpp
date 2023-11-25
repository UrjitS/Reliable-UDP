#include <iostream>
#include <chrono>
#include <thread>
#include "reliable-udp.hpp"
#include "networking.hpp"
#include "transfer.hpp"

void send_input(struct networking_options& networkingOptions, volatile int& exit_flag) {
    while (!exit_flag) {
        std::string input;

        // Read up to 1010 bytes or until a newline is encountered
        for (int i = 0; i < MAX_PACKET_LENGTH; ++i) {
            int ch = std::cin.get();
            if (ch == '\n') {
                break;
            } else if (ch == EOF) {
                exit_flag = true;
                return;
            }
            input.push_back(static_cast<char>(ch));
        }

        if (input.empty()) {
            continue;
        }
        printf("Sending: %s\n", input.c_str());
        // Set the data field in the header to the input
        networkingOptions.header->sequence_number++;
        networkingOptions.header->data = input;

        if (exit_flag) {
            return;
        }

        // Loop send_packet until it is able to be sent
        int ret_status;
        while ((ret_status = send_packet(networkingOptions)) != 0) {
            if (exit_flag) {
                return;
            }
            if (ret_status == -1) {
                std::cerr << "Failed to Send." << std::endl;
            }
        }
    }
}





void read_response(struct networking_options& networkingOptions, volatile int& exit_flag) {
    while (!exit_flag) {
        // Call receive_acknowledgements
        receive_acknowledgements(networkingOptions, 10);

        // Sleep for a certain duration before rechecking for acknowledgments
        std::chrono::milliseconds sleep_duration(100); // Adjust the duration as needed
        std::this_thread::sleep_for(sleep_duration);
    }
}
