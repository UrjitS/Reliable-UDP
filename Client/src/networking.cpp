#include "networking.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <arpa/inet.h>

std::vector<header_field> * packets = new std::vector<header_field>();


bool validate_ip_address(const std::string& ip_address) {
    struct sockaddr_in sa{};
    sa.sin_addr.s_addr = inet_addr(ip_address.c_str());

    if (sa.sin_addr.s_addr == (in_addr_t) -1) {
        return false;
    }
    return true;
}

bool create_udp_socket() {
    // Create a socket
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("socket creation failed");
        return false;
    }

    return true;
}

bool bind_udp_socket(struct networking_options * networkingOptions) {
    // Bind the socket to an IP address and port
    struct sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(networkingOptions->ip_address.c_str());
    server_address.sin_port = htons(networkingOptions->port_number);

    if (bind(networkingOptions->socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("Bind Failed");
        return false;
    }

    return true;
}



