#include "reliable-udp.hpp"
#include "networking.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <cstring>
#include <mutex>
#include <iostream>

std::mutex modifying_global_variables;
std::vector<header_field> sent_packets = std::vector<header_field>();
int window_size = 0;

char* pack_header(struct header_field* header);
void increment_sent_counter();
ssize_t send_packet_over(struct networking_options& networkingOptions, char * packet);
void check_need_for_retransmission(struct networking_options& networkingOptions);
struct header_field decode_string(const std::string& packet_raw);

char* pack_header(struct header_field* header) {
    header->data_length = header->data.length() + 3;
    std::cout << "Header: " << header->sequence_number << " " << header->ack_number << " " << static_cast<int>(header->flags) << " " << header->data_length << " " << header->data << std::endl;

    uint32_t seq_number = htonl(header->sequence_number);
    uint32_t ack_number = htonl(header->ack_number);
    uint8_t flags = htons(header->flags); // assuming flags is a uint8_t
    uint16_t data_length = htons(header->data_length);

    // Calculate the total size needed for the buffer
    size_t total_size = sizeof(seq_number) + sizeof(ack_number) + sizeof(flags) + sizeof(data_length) + header->data.length() + 1 + 2;

    // Allocate memory for the buffer
    char* packet = new char[total_size];

    // Copy data into the buffer
    std::memcpy(packet, &seq_number, sizeof(seq_number));
    std::memcpy(packet + sizeof(seq_number), &ack_number, sizeof(ack_number));
    std::memcpy(packet + sizeof(seq_number) + sizeof(ack_number), &flags, sizeof(flags));
    std::memcpy(packet + sizeof(seq_number) + sizeof(ack_number) + sizeof(flags), &data_length, sizeof(data_length));
    std::memcpy(packet + sizeof(seq_number) + sizeof(ack_number) + sizeof(flags) + sizeof(data_length), header->data.c_str(), header->data.length());
    packet[total_size - 3] = '\0'; // null terminator
    packet[total_size - 2] = '\x03'; // ETX character
    packet[total_size - 1] = '\x03'; // ETX character

    std::cout << "Packet: " << packet << std::endl;

    return packet;
}


void increment_sent_counter() {
    for (auto & sent_packet : sent_packets) {
        sent_packet.sent_counter++;
    }
}

ssize_t send_packet_over(struct networking_options& networkingOptions, char * packet) {
    ssize_t ret_status;

    if (networkingOptions.ip_family == AF_INET) {
        ret_status = sendto(networkingOptions.socket_fd, packet, sizeof(packet), 0, (struct sockaddr *) &networkingOptions.ipv4_addr, sizeof(networkingOptions.ipv4_addr));
    } else {
        ret_status = sendto(networkingOptions.socket_fd, packet, sizeof(packet), 0, (struct sockaddr *) &networkingOptions.ipv6_addr, sizeof(networkingOptions.ipv6_addr));
    }

    return ret_status;
}

int send_packet(struct networking_options& networkingOptions) {
    ssize_t ret_status;

    // Make sure the window size is not exceeded
    if (window_size >= MAX_WINDOW) {
        return 1;
    }

    char * packet = pack_header(networkingOptions.header);

    modifying_global_variables.lock();

    // Add to sent packets
    sent_packets.push_back(*networkingOptions.header);

    // Send the packet
    ret_status = send_packet_over(networkingOptions, packet);

    if (ret_status < 0) {
        perror("Send Failed");
        return -1;
    }

    window_size++;
    increment_sent_counter();

    modifying_global_variables.unlock();

    return 0;
}


struct header_field decode_string(const std::string& packet_raw) {
    struct header_field decoded_header;

    // Extract fields from the packet
    uint32_t seq_number;
    uint32_t ack_number;
    uint8_t flags;

    std::memcpy(&seq_number, packet_raw.data(), sizeof(seq_number));
    std::memcpy(&ack_number, packet_raw.data() + sizeof(seq_number), sizeof(ack_number));
    std::memcpy(&flags, packet_raw.data() + sizeof(seq_number) + sizeof(ack_number), sizeof(flags));

    // Convert network byte order to host byte order
    decoded_header.sequence_number = ntohl(seq_number);
    decoded_header.ack_number = ntohl(ack_number);
    decoded_header.flags = ntohs(flags);


    return decoded_header;
}


void check_need_for_retransmission(struct networking_options& networkingOptions) {
    for (auto & sent_packet : sent_packets) {
        if (sent_packet.sent_counter >= 5) {
            // Retransmit packet
            char * packet = pack_header(&sent_packet);
            ssize_t ret_status = send_packet_over(networkingOptions, packet);
            if (ret_status < 0) {
                perror("Retransmission Failed To Send");
                return;
            }
            sent_packet.sent_counter = 0;
        }
    }
}

void remove_packet_from_sent_packets(struct header_field& header) {
    for (auto & sent_packet : sent_packets) {
        if (sent_packet.sequence_number == header.ack_number) {
            sent_packets.erase(sent_packets.begin());
            window_size--;
            return;
        }
    }
}

#include <sys/time.h>

int receive_acknowledgements(struct networking_options& networkingOptions, int timeout_seconds) {
    ssize_t ret_status;
    modifying_global_variables.lock();

    // Check if any packets need to be retransmitted
    check_need_for_retransmission(networkingOptions);

    // Set up fd_set for select
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(networkingOptions.socket_fd, &read_fds);

    // Set up timeout using timeval struct
    struct timeval timeout{};
    timeout.tv_sec = timeout_seconds;
    timeout.tv_usec = 0;

    // Use select to wait for data or timeout
    ret_status = select(networkingOptions.socket_fd + 1, &read_fds, nullptr, nullptr, &timeout);

    if (ret_status == 0) {
        // Timeout occurred
//        printf("Timeout: No data received within the specified time.\n");
        modifying_global_variables.unlock();
        return -1;
    } else if (ret_status < 0) {
        // Error in select
        perror("Select failed");
        modifying_global_variables.unlock();
        return -1;
    }

    // Receive the acknowledgement
    char buffer[HEADER_LENGTH];
    ret_status = recvfrom(networkingOptions.socket_fd, buffer, sizeof(buffer), 0, nullptr, nullptr);

    if (ret_status < 0) {
        perror("Receive Failed");
        modifying_global_variables.unlock();
        return -1;
    }

    // Decode the acknowledgement
    struct header_field decoded_header = decode_string(std::string(buffer));

    // Remove the packet from the list of sent packets
    remove_packet_from_sent_packets(decoded_header);

    modifying_global_variables.unlock();
    return false;
}
