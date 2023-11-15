#include "reliable-udp.hpp"
#include "networking.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <cstring>
#include <mutex>

std::mutex modifying_global_variables;
std::vector<header_field> sent_packets = std::vector<header_field>();
int window_size = 0;

std::string pack_header(struct header_field * header);
void increment_sent_counter();
ssize_t send_packet_over(struct networking_options& networkingOptions, std::string& packet);
void check_need_for_retransmission(struct networking_options& networkingOptions);
struct header_field decode_string(const std::string& packet_raw);

std::string pack_header(struct header_field * header) {
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
    packet.append("\03\03");

    return packet;
}

void increment_sent_counter() {
    for (auto & sent_packet : sent_packets) {
        sent_packet.sent_counter++;
    }
}

ssize_t send_packet_over(struct networking_options& networkingOptions, std::string& packet) {
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
            return;
        }
    }
}

int receive_acknowledgements(struct networking_options& networkingOptions) {
    ssize_t ret_status;
    modifying_global_variables.lock();

    // Check if any packets need to be retransmitted
    check_need_for_retransmission(networkingOptions);

    // Receive the acknowledgement
    char buffer[MAX_PACKET_SIZE];
    ret_status = recvfrom(networkingOptions.socket_fd, buffer, sizeof(buffer), 0, nullptr, nullptr);

    if (ret_status < 0) {
        perror("Receive Failed");
        return -1;
    }

    // Decode the acknowledgement
    struct header_field decoded_header = decode_string(std::string(buffer));

    // Remove the packet from the list of sent packets
    remove_packet_from_sent_packets(decoded_header);

    modifying_global_variables.unlock();
    return false;
}