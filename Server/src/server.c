//
// Created by Colin Lam on 2023-11-13.
//

#include "server.h"

int entry_state(void *arg)
{
    printf("Starting server...\n");
    return ok;
}

int parse_args(void *arg)
{
    struct server_opts *opts = (struct server_opts *) arg;

    if(opts->argc != SERVER_ARGS)
    {
        opts->msg = strdup("Invalid number of arguments\n");
        return error;
    }

    opts->host_ip = strdup(opts->argv[IP_INDEX]);
    opts->ip_family = get_ip_family(opts->host_ip);
    if(opts->ip_family == -1)
    {
        opts->msg = strdup("IP address is not a valid IPv4 or IPv6 address\n");
        return error;
    }

    if(parse_in_port_t(opts) == -1)
    {
        return error;
    }

    printf("---------------------------- Server Options ----------------------------\n");
    printf("Server IP Address: %s\n", opts->host_ip);
    printf("Server Domain: %d\n", opts->ip_family);
    printf("Server Port: %hu\n", opts->host_port);
    printf("---------------------------- Server Options ----------------------------\n");

    return ok;

}

int get_ip_family(const char *ip_addr)
{
    int domain;
    domain = -1;

    if(strstr(ip_addr, ":"))
    {
        domain = AF_INET6;
    }
    else if (strstr(ip_addr, "."))
    {
        domain = AF_INET;
    }

    return domain;

}

int parse_in_port_t(struct server_opts *opts)
{
    char *endptr;
    uintmax_t parsed_value;

    parsed_value = strtoumax(opts->argv[PORT_INDEX], &endptr, 10);

    if (errno != 0)
    {
        opts->msg = strdup("Error parsing in_port_t\n");
        return -1;
    }

    // Check if there are any non-numeric characters in the input string
    if (*endptr != '\0')
    {
        opts->msg = strdup("Invalid characters in input.\n");
        return -1;
    }

    // Check if the parsed value is within the valid range for in_port_t
    if (parsed_value > UINT16_MAX)
    {
        opts->msg = strdup("in_port_t value out of range.\n");
        return -1;
    }

    opts->host_port = (in_port_t)parsed_value;
    return 0;
}

int set_up(void *arg) {
    struct server_opts *opts = (struct server_opts *) arg;
    struct sockaddr_in addr;
    int ret;

    opts->sock_fd = socket(opts->ip_family, SOCK_DGRAM, 0);
    if (opts->sock_fd == -1) {
        opts->msg = strdup("socket failed\n");
        return error;
    }

    addr.sin_family = opts->ip_family;
    addr.sin_port = htons(opts->host_port);
    addr.sin_addr.s_addr = inet_addr(opts->host_ip);
    if (addr.sin_addr.s_addr == (in_addr_t) -1) {
        opts->msg = strdup("inet_addr failed\n");
        return error;
    }

    int option = 1;
    setsockopt(opts->sock_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    ret = bind(opts->sock_fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
    if (ret == -1) {
        opts->msg = strdup("bind failed\n");
        return error;
    }

    printf("Finished Set up\n");
    return ok;
}

void manage_window(uint32_t *client_seq_num, struct stash *window, struct packet *pkt)
{
    uint32_t pkt_seq_num;

    //store_packet
    pkt_seq_num = pkt->header->seq_num - *client_seq_num;
    window[pkt_seq_num].cleared = 1;
    window[pkt_seq_num].rel_num = pkt_seq_num;
    window[pkt_seq_num].seq_num = pkt->header->seq_num;
    window[pkt_seq_num].data = strdup(pkt->data); //malloc

    //check_window
    check_window(client_seq_num, window);
    //order_window
    order_window(client_seq_num, window);
}

void check_window(uint32_t *client_seq_num, struct stash *window)
{
    for(size_t i = 0; i < WIN_SIZE; i++)
    {
        if(window[i].seq_num == *client_seq_num)
        {
            deliver_data(window[i].data);
            reset_stash(&window[i]);
            (*client_seq_num)++;
        }
    }
}

void order_window(const uint32_t *client_seq_num, struct stash *window)
{
    //order window
    for(size_t i = 0; i < WIN_SIZE; i++)
    {
        if(window[i].cleared == 1)
        {
            window[i].rel_num = window[i].seq_num - *client_seq_num;
            window[window[i].rel_num] = window[i];
        }
    }

}

void deliver_data(char *data)
{
    size_t len;

    len = strlen(data);
    len -= ACK_DATA_LEN;

    for(size_t i = 0; i < len; i++)
    {
        printf("%c", data[i]);
    }
}

void reset_stash(struct stash *stash)
{
    stash->cleared = 0;
    stash->rel_num = 0;
    stash->seq_num = 0;
    if(stash->data)
    {
        free(stash->data);
    }
}

void deserialize_header(char *header, struct packet *pkt)
{
    size_t count;

    count = 0;
    memcpy(&pkt->header->seq_num, &header[count], sizeof(pkt->header->seq_num));
    count += sizeof(pkt->header->seq_num);
    memcpy(&pkt->header->ack_num, &header[count], sizeof(pkt->header->ack_num));
    count += sizeof(pkt->header->ack_num);
    memcpy(&pkt->header->flags, &header[count], sizeof(pkt->header->flags));
    count += sizeof(pkt->header->flags);
    memcpy(&pkt->header->data_len, &header[count], sizeof(pkt->header->data_len));

    pkt->header->seq_num = ntohl(pkt->header->seq_num);
    pkt->header->ack_num = ntohl(pkt->header->ack_num);
    pkt->header->data_len = ntohs(pkt->header->data_len);
}

void return_ack(int sock_fd, uint32_t *server_seq_num, uint32_t pkt_seq_num,
                struct sockaddr *from_addr, socklen_t from_addr_len)
{
    uint8_t *ack;

    ack = malloc(ACK_SIZE);
    generate_ack(ack, *server_seq_num, pkt_seq_num, ACK, ACK_DATA_LEN);
    sendto(sock_fd, ack, ACK_SIZE, 0, from_addr, from_addr_len);
    (*server_seq_num)++;
}

void generate_ack(uint8_t *ack, uint32_t server_seq_num, uint32_t pkt_seq_num, uint8_t flags, uint16_t data_len)
{
    size_t count;
    server_seq_num = htonl(server_seq_num);
    pkt_seq_num = htonl(pkt_seq_num);
    data_len = htons(data_len);

    count = 0;
    memcpy(&ack[count], &server_seq_num, sizeof(uint32_t));
    count += sizeof(uint32_t);
    memcpy(&ack[count], &pkt_seq_num, sizeof(uint32_t));
    count += sizeof(uint32_t);
    memcpy(&ack[count], &flags, sizeof(uint8_t));
    count += sizeof(uint8_t);
    memcpy(&ack[count], &data_len, sizeof(uint16_t));
    count += sizeof(uint16_t);
    ack[count] = ETX;
    count++;
    ack[count] = ETX;
}

int print_error(void *arg)
{
    struct server_opts *opts = (struct server_opts *) arg;
    printf("%s\n", opts->msg);

    return ok;
}

int clean_up(void *arg)
{
    struct server_opts *opts = (struct server_opts *) arg;
    if(opts->msg)
    {
        free(opts->msg);
    }

    if(opts->host_ip)
    {
        free(opts->host_ip);
    }

    close(opts->sock_fd);

    printf("Finished clean up\n");
    return ok;
}

int end_state(void *arg)
{
    printf("Server closing...\n");
    return ok;
}
