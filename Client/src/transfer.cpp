#include <iostream>
#include <chrono>
#include <thread>
#include "reliable-udp.hpp"
#include "networking.hpp"
#include "transfer.hpp"

/**
 * @brief Boolean to check if a file has been sent entirely
 */
bool sent_file = false;
/**
 * @brief Last acknowledgement number received
 */
uint32_t last_ack = 0;

void send_input(struct networking_options& networkingOptions, volatile int& exit_flag) {
    while (!exit_flag) {
        std::string input;

        // Read up to 1010 bytes or until Enter is pressed
        for (int i = 0; i < MAX_PACKET_LENGTH; ++i) {
            int ch = std::cin.get();

            if (networkingOptions.terminal_input) {
                if (ch == '\n') {
                    // Enter key pressed, break the loop
                    break;
                }
                input.push_back(static_cast<char>(ch));
            } else {
                if (ch == EOF) {
                    // End of file reached
                    sent_file = true;
                    last_ack = networkingOptions.header->sequence_number;
                    break;
                }
                input.push_back(static_cast<char>(ch));
            }
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
        uint32_t ack_number = receive_acknowledgements(networkingOptions, 10);

        if (ack_number == 0) {
            std::cerr << "Failed to Receive Acknowledgement." << std::endl;
            break;
        }

        if (sent_file && (ack_number == last_ack) && (networkingOptions.current_window_size == 0)) {
            std::cout << "File Sent Successfully." << std::endl;
            exit_flag = true;
        }

        // Sleep for a certain duration before rechecking for acknowledgments
        std::chrono::milliseconds sleep_duration(100);
        std::this_thread::sleep_for(sleep_duration);
    }
}
