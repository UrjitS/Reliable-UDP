#include <stdbool.h>
#include "server.h"

int volatile exit_flag = false;

void sigHandler(int signal) {
    exit_flag = true;
    printf("exit_flag set to %d\n", exit_flag);
}

static int compare_transitions(const void *a, const void *b);
static enum state_codes lookup_transitions(enum state_codes current_state, enum ret_codes return_code);

int (* state[])(void *arg) = {entry_state, parse_args, set_up,
                              do_read, print_error, clean_up, end_state}; //function pointers go here

struct transition state_transitions[] = {
        {ENTRY, ok, PARSEARGS},
        {PARSEARGS, ok, SETUP},
        {PARSEARGS, error, FATALERROR},
        {SETUP, ok, READ},
        {SETUP, error, FATALERROR},
        {READ, ok, READ},
        {READ, error, FATALERROR},
        {READ, done, CLEANUP},
        {FATALERROR, ok, CLEANUP},
        {CLEANUP, ok, END}
};

int main (int argc, char *argv[])
{
    enum state_codes cur_state = ENTRY;
    enum ret_codes rc;
    int (* state_fun)(void *arg);

    struct server_opts opts;
    memset(&opts, 0 , sizeof(struct server_opts));
    opts.argc = argc;
    opts.argv = argv;

//    signal(SIGINT, sigHandler); //start the thread in SETUP, change the state to END when exit_flag is true

    while (1) {
        state_fun = state[cur_state];
        rc = state_fun(&opts);
        if (END == cur_state)
            break;
        cur_state = lookup_transitions(cur_state, rc);
    }

    return EXIT_SUCCESS;
}

/* Comparison function for binary search */
static int compare_transitions(const void *a, const void *b) {
    struct transition *transitionA = (struct transition *)a;
    struct transition *transitionB = (struct transition *)b;

    if (transitionA->src_state == transitionB->src_state) {
        return (int)(transitionA->ret_code) - (int)(transitionB->ret_code);
    } else {
        return (int)(transitionA->src_state) - (int)(transitionB->src_state);
    }
}

/* Function to look up the next state based on current state and return code using binary search */
static enum state_codes lookup_transitions(enum state_codes current_state, enum ret_codes return_code) {
    struct transition key = {current_state, return_code, 0};
    struct transition *result;

    result = (struct transition *)bsearch(&key, state_transitions, sizeof(state_transitions) / sizeof(struct transition),
                                          sizeof(struct transition), compare_transitions);

    if (result != NULL) {
        return result->dst_state;
    } else {
        printf("Invalid transition!\n");
        return END;
    }
}

int do_read(void *arg)
{
    struct server_opts *opts = (struct server_opts *) arg;
    struct sockaddr from_addr;
    socklen_t from_addr_len = sizeof(struct sockaddr_in);
    char header[HEADER_LEN];
    struct stash window[WIN_SIZE];
    ssize_t rbytes;
    uint32_t client_seq_num;
    uint32_t server_seq_num;

    client_seq_num = 0;
    server_seq_num = 0;

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    if (setsockopt(opts->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        perror("Error setting socket timeout");
        close(opts->sock_fd);
        exit(EXIT_FAILURE);
    }

    rbytes = 0;
    while (rbytes < HEADER_LEN) {
        printf("Reading header...\n");
        ssize_t recv_result = recvfrom(opts->sock_fd, &header[rbytes], HEADER_LEN - rbytes, 0, &from_addr, &from_addr_len);
        if (recv_result >= 0) {
            rbytes += recv_result;
        }
        printf("rbytes: %zdd", rbytes);
    }
    printf("first rbytes: %zd\n", rbytes);

    struct packet *pkt = malloc(sizeof(struct packet));
    pkt->header = malloc(sizeof(struct packet_header));
    deserialize_header(header, pkt);
    pkt->data = malloc(pkt->header->data_len);
    print_packet_header(pkt);
    rbytes = 0;
    while (rbytes < pkt->header->data_len) {
        printf("Reading data...\n");
        ssize_t recv_result = recvfrom(opts->sock_fd, &pkt->data[rbytes], pkt->header->data_len - rbytes, 0, &from_addr, &from_addr_len);
        if (recv_result >= 0) {
            rbytes += recv_result;
        }
        printf("rbytes: %zdd", rbytes);
    }
    printf("second rbytes: %zd\n", rbytes);
    printf("pkt->data: %s\n", pkt->data);

    if(pkt->header->seq_num < client_seq_num)
    {
        printf("Unexpected data arrived...\n");
        //RETURN ACK
        return_ack(opts->sock_fd, &server_seq_num, pkt->header->seq_num, &from_addr, from_addr_len);
        //IGNORE PACKET
    }
    else if(pkt->header->seq_num <= client_seq_num && pkt->header->seq_num < client_seq_num+WIN_SIZE)
    {
        printf("Expected data arrived...\n");
        //RETURN ACK
        return_ack(opts->sock_fd, &server_seq_num, pkt->header->seq_num, &from_addr, from_addr_len);
        //STASH AND DELIVER LOGIC
        manage_window(&client_seq_num, window, pkt);
    }
    else
    {
        printf("Ignore...\n");
    }
    free(pkt->header);
    free(pkt->data);
    free(pkt);


    for(size_t i = 0; i < WIN_SIZE; i++)
    {
        reset_stash(&window[i]);
    }

    if(opts->msg)
    {
        return error;
    }

    return ok;
}

void print_packet_header(struct packet *pkt)
{
    printf("----------Packet Info----------\n");
    printf("Seq Num: %d\n", pkt->header->seq_num);
    printf("Ack Num: %d\n", pkt->header->ack_num);
    printf("Flags: %d\n", pkt->header->flags);
    printf("Data Len: %d\n", pkt->header->data_len);
}
