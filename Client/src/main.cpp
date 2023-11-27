#include "networking.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <getopt.h>
#include <unistd.h>
#include "transfer.hpp"
#include <csignal>
#include <thread>

using namespace std;

/**
 * @brief Parse command line arguments for Receiver IP Address and Port Number
 *
 * @param argc Program argument count
 * @param argv Program argument values
 * @param networkingOptions Networking options struct
 * @return void
 */
static void parse_arguments(int argc, char * argv[], struct networking_options& networkingOptions);
/**
 * @brief Print the programs usage, clean resources and exits
 * @param networkingOptions Networking options struct
 * @return void
 */
static void print_program_usage(struct networking_options& networkingOptions);
/**
 * @brief Display error message, clean resources and exits
 * @param networkingOptions Networking options struct
 * @return void
 */
static void display_error(struct networking_options& networkingOptions);
/**
 * @brief Check if the IP address is IPv4 or IPv6 and Sets up the networking options receiver struct
 * @param networkingOptions Networking options struct
 * @return void
 */
static void check_ip_address(struct networking_options &networkingOptions);
/**
 * @brief Clean resources and exit
 * @param networkingOptions Networking options struct
 * @return void
 */
void clean_resources(struct networking_options& networkingOptions);
/**
 * @brief Setup the connection
 * @param networkingOptions Networking options struct
 * @return bool True if connection was successful, false otherwise
 */
bool setup_connection(struct networking_options& networkingOptions);

volatile int exit_flag;

void sigHandler([[maybe_unused]] int signal) {
    std::cout << std::endl << "Exiting Program " << std::endl;
    exit_flag = true;
}

int main(int argc, char * argv[]) {
    exit_flag = false;
    // Get cmd line arguments
    struct networking_options networkingOptions{};
    struct header_field header{};

    header.sequence_number = -1;
    header.ack_number = 0;
    header.flags = 1;
    header.data_length = 0;
    header.sent_counter = 0;

    networkingOptions.header = &header;
    networkingOptions.socket_fd = -1;
    networkingOptions.program_name = argv[0];
    networkingOptions.stats_file = fopen("output.txt", "w");
    networkingOptions.time_started = time(nullptr);
    if (networkingOptions.stats_file == nullptr) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    if (isatty(fileno(stdin))) {
        networkingOptions.terminal_input = true;
    } else {
        networkingOptions.terminal_input = false;
    }

    parse_arguments(argc, argv, networkingOptions);

    if (!setup_connection(networkingOptions)) {
        display_error(networkingOptions);
    }

    // Register signal interrupt
    signal(SIGINT, sigHandler);

    // Create sending and receiver threads
    std::thread send_input_thread(send_input, std::ref(networkingOptions), std::ref(exit_flag));
    std::thread read_ack_response_thread(read_response, std::ref(networkingOptions), std::ref(exit_flag));
    // Wait for both threads to finish
    send_input_thread.join();
    read_ack_response_thread.join();

    // Startup sending and receiving threads
    clean_resources(networkingOptions);

    return EXIT_SUCCESS;
}

bool setup_connection(struct networking_options& networkingOptions) {
    // Create a socket
    int socket_fd = create_udp_socket(networkingOptions);

    if (socket_fd == -1) {
        networkingOptions.message = "Failed to create socket";
        return false;
    }
    // Bind the socket to an IP address and port
    networkingOptions.socket_fd = socket_fd;

    if (!bind_udp_socket(networkingOptions)) {
        return false;
    }

    return true;
}

void parse_arguments(int argc, char * argv[], struct networking_options& networkingOptions) {
    opterr = 0;

    if(argc != 3)
    {
        networkingOptions.message = "Please give Receiver IP address, and port.";
        print_program_usage(networkingOptions);
    }

    // Handle IP
    networkingOptions.receiver_ip_address = argv[optind];
    if (!validate_ip_address(networkingOptions.receiver_ip_address)) {
        networkingOptions.message = "Invalid IP Address";
        display_error(networkingOptions);
    }

    // Handle Port Number
    char * end_ptr;
    long port = std::strtol(argv[optind+1], &end_ptr, 10);

    if (*end_ptr != '\0' || errno == ERANGE || port < 0 || port > 65535) {
        networkingOptions.message = "Invalid Port Number";
        display_error(networkingOptions);
    }

    networkingOptions.receiver_port = static_cast<in_port_t>(port);

    check_ip_address(networkingOptions);

    cout << "Sending to Ip Address: " << networkingOptions.receiver_ip_address << endl;
    cout << "Sending to Port: " << networkingOptions.receiver_port << endl;
}

static void check_ip_address(struct networking_options& networkingOptions) {
    struct sockaddr_storage receiver_struct{};
    if (inet_pton(AF_INET, networkingOptions.receiver_ip_address.c_str(), &receiver_struct) == 1) {
        networkingOptions.ip_family = AF_INET;
        networkingOptions.ipv4_addr = *reinterpret_cast<struct sockaddr_in*>(&receiver_struct);
        networkingOptions.ipv4_addr.sin_addr.s_addr = inet_addr(networkingOptions.receiver_ip_address.c_str());
        networkingOptions.ipv4_addr.sin_family = AF_INET;
        networkingOptions.ipv4_addr.sin_port = htons(networkingOptions.receiver_port);
    } else if (inet_pton(AF_INET6, networkingOptions.receiver_ip_address.c_str(), &receiver_struct) == 1) {
        networkingOptions.ip_family = AF_INET6;
        networkingOptions.ipv6_addr = *reinterpret_cast<struct sockaddr_in6*>(&receiver_struct);
        networkingOptions.ipv6_addr.sin6_family = AF_INET6;
//        networkingOptions.ipv6_addr.sin6_addr = inet_addr(networkingOptions.receiver_ip_address.c_str());
        networkingOptions.ipv6_addr.sin6_port = htons(networkingOptions.receiver_port);
    }
}

static void print_program_usage(struct networking_options& networkingOptions) {
    if (!networkingOptions.message.empty()) {
        cerr << networkingOptions.message << endl;
    }

    cerr << "Usage: " << networkingOptions.program_name << " <receiver ip address>, <receiver port number>" << endl;

    clean_resources(networkingOptions);
}

static void display_error(struct networking_options& networkingOptions) {
    if (!networkingOptions.message.empty()) {
        cerr << networkingOptions.message << endl;
    }

    clean_resources(networkingOptions);
}

void clean_resources(struct networking_options& networkingOptions) {
    if (networkingOptions.socket_fd > 0) {
        close(networkingOptions.socket_fd);
    }

    if (networkingOptions.stats_file != nullptr) {
        fclose(networkingOptions.stats_file);
    }

    exit(EXIT_SUCCESS);
}
