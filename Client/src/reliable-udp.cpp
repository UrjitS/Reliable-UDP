#include "reliable-udp.hpp"
#include "networking.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <cstring>
#include <mutex>
#include <iostream>
#include <sys/time.h>

std::mutex modifying_global_variables;
std::vector<header_field> sent_packets = std::vector<header_field>();
int window_size = 0;

std::string pack_header(struct header_field* header);
void increment_sent_counter();
ssize_t send_packet_over(struct networking_options& networkingOptions, const std::string& packet);
void check_need_for_retransmission(struct networking_options& networkingOptions);
struct header_field decode_string(char * packet_raw);

std::string pack_header(struct header_field * header) {
    header->data_length = header->data.length() + 3;
    uint32_t seq_number  = htonl(header->sequence_number);
    uint32_t ack_number  = htonl(header->ack_number);
    uint8_t flags        = htons(header->flags);
    uint16_t data_length = htons(header->data_length);

    std::string packet;
    packet.append(reinterpret_cast<const char *>(&seq_number), sizeof(seq_number));
    packet.append(reinterpret_cast<const char *>(&ack_number), sizeof(ack_number));
    packet.append(reinterpret_cast<const char *>(&flags), sizeof(flags));
    packet.append(reinterpret_cast<const char *>(&data_length), sizeof(data_length));
    packet.append(header->data);
    packet.append("\0", 1);        // Append a null character with length 1
    packet.append("\x03\x03", 2);  // Append two ETX characters

    return packet;
}

void increment_sent_counter() {
    // Iterate over the elements up to a maximum of the first five
    for (size_t i = 0; i < std::min(sent_packets.size(), static_cast<size_t>(5)); ++i) {
        auto& sent_packet = sent_packets[i];
        sent_packet.sent_counter++;

    }
}

ssize_t send_packet_over(struct networking_options& networkingOptions, const std::string& packet) {
    ssize_t ret_status;

    if (networkingOptions.ip_family == AF_INET) {
        ret_status = sendto(networkingOptions.socket_fd, packet.c_str(), packet.length(), 0, (struct sockaddr *) &networkingOptions.ipv4_addr, sizeof(networkingOptions.ipv4_addr));
    } else {
        ret_status = sendto(networkingOptions.socket_fd, packet.c_str(), packet.length(), 0, (struct sockaddr *) &networkingOptions.ipv6_addr, sizeof(networkingOptions.ipv6_addr));
    }

    return ret_status;
}

int send_packet(struct networking_options& networkingOptions) {
    ssize_t ret_status;

    // Make sure the window size is not exceeded
    if (window_size >= MAX_WINDOW) {
        return 1;
    }

    std::string packet = pack_header(networkingOptions.header);

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


struct header_field decode_string(char * packet_raw) {
    struct header_field decoded_header;

    // Extract fields from the packet
    uint32_t seq_number;
    uint32_t ack_number;
    uint8_t flags;
    uint16_t data_length;

    std::memcpy(&seq_number, &packet_raw[0], sizeof(seq_number));
    std::memcpy(&ack_number, &packet_raw[4], sizeof(ack_number));
    std::memcpy(&flags, &packet_raw[8], sizeof(flags));
    std::memcpy(&data_length, &packet_raw[9], sizeof(data_length));

    // Convert network byte order to host byte order
    decoded_header.sequence_number = ntohl(seq_number);
    decoded_header.ack_number = ntohl(ack_number);
    decoded_header.flags = ntohs(flags);
    decoded_header.data_length = ntohs(data_length);


    return decoded_header;
}


void check_need_for_retransmission(struct networking_options& networkingOptions) {
    // Iterate over the elements up to a maximum of the first five
    for (size_t i = 0; i < std::min(sent_packets.size(), static_cast<size_t>(5)); ++i) {
        auto& sent_packet = sent_packets[i];

        if (sent_packet.sent_counter >= 20) {
            printf("Retransmitting packet with sequence number %d\n", sent_packet.sequence_number);
            // Retransmit packet
            std::string packet = pack_header(&sent_packet);
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


uint32_t receive_acknowledgements(struct networking_options& networkingOptions, int timeout_seconds) {
    ssize_t ret_status;

    if (window_size >= MAX_WINDOW) {
        increment_sent_counter();
    }

    // Set up fd_set for select
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(networkingOptions.socket_fd, &read_fds);

    // Set up timeout using timeval struct
    struct timeval timeout{};
    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_seconds;

    // Use select to wait for data or timeout
    ret_status = select(networkingOptions.socket_fd + 1, &read_fds, nullptr, nullptr, &timeout);

    if (ret_status == 0) {
        // Timeout occurred
        modifying_global_variables.unlock();
        increment_sent_counter();
        return 1;
    } else if (ret_status < 0) {
        // Error in select
        perror("Select failed");
        modifying_global_variables.unlock();
        return 0;
    }

    // Receive the acknowledgement
    char buffer[HEADER_LENGTH];
    ret_status = recvfrom(networkingOptions.socket_fd, buffer, sizeof(buffer), 0, nullptr, nullptr);

    if (ret_status < 0) {
        perror("Receive Failed");
        modifying_global_variables.unlock();
        return 0;
    }

    modifying_global_variables.lock();

    // Check if any packets need to be retransmitted
    check_need_for_retransmission(networkingOptions);

    // Decode the acknowledgement
    struct header_field decoded_header = decode_string(buffer);
    std::cout << "----------RECEIVING----------" << std::endl;

    std::cout << "Seq: " << decoded_header.ack_number << std::endl;
    std::cout << "Ack: " << decoded_header.ack_number << std::endl;
    // Remove the packet from the list of sent packets
    remove_packet_from_sent_packets(decoded_header);

    modifying_global_variables.unlock();

    // Return the acknowledgement number except for the first packet
    return decoded_header.ack_number == 0 ? 1 : decoded_header.ack_number;
}
