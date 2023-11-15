#include "networking.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <cstring>
#include <netdb.h>

/**
 * @brief Gets the devices IP address
 * @param networkingOptions Networking options struct
 * @return True if successful, false otherwise
 */
bool get_device_ip_address(struct networking_options& networkingOptions);


bool validate_ip_address(const std::string& ip_address) {
    struct sockaddr_in sa{};
    sa.sin_addr.s_addr = inet_addr(ip_address.c_str());

    if (sa.sin_addr.s_addr == (in_addr_t) -1) {
        return false;
    }
    return true;
}

int create_udp_socket(struct networking_options& networkingOptions) {
    // Create a socket
    int socket_fd = socket(networkingOptions.ip_family, SOCK_DGRAM, 0);

    if (socket_fd < 0) {
        perror("socket creation failed");
        return false;
    }

    return socket_fd;
}

bool get_device_ip_address(struct networking_options& networkingOptions) {
    struct ifaddrs *interfaces;
    struct ifaddrs *ifaddr;
    char            host[NI_MAXHOST];

    // Get the list of network interfaces
    if(getifaddrs(&interfaces) == -1)
    {
        perror("getifaddrs");
        return false;
    }

    for(ifaddr = interfaces; ifaddr != nullptr; ifaddr = ifaddr->ifa_next)
    {
        if(ifaddr->ifa_addr == nullptr)
        {
            continue;
        }

        if(ifaddr->ifa_addr->sa_family == AF_INET && strcmp(ifaddr->ifa_name, "lo") != 0)
        {
            struct sockaddr_in ipv4{};

            memcpy(&ipv4, ifaddr->ifa_addr, sizeof(struct sockaddr_in));
            inet_ntop(AF_INET, &(ipv4.sin_addr), host, NI_MAXHOST);
            std::cout << "Device IP: " << ifaddr->ifa_name << " : " << host << std::endl;
            networkingOptions.device_ip_address = host;
            freeifaddrs(interfaces);
            return true;
        }
        else
        {
            continue;
        }

    }

    freeifaddrs(interfaces);

    return false;
}

bool bind_udp_socket(struct networking_options& networkingOptions) {
    // Bind the socket to an IP address and port
    struct sockaddr_in server_address{};

    if (!get_device_ip_address(networkingOptions)) {
        networkingOptions.message = "Failed to get device IP address";
        return false;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(networkingOptions.device_ip_address.c_str());
    server_address.sin_port = htons(DEFAULT_PORT);

    if (bind(networkingOptions.socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("Bind Failed");
        return false;
    }

    return true;
}

