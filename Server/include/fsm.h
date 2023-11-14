#ifndef RELIABLE_UDP_FSM_H
#define RELIABLE_UDP_FSM_H

int entry_state(void *arg);
int parse_args(void *arg);
int set_up(void *arg);
int wait_client(void *arg);
int do_read(void *arg);
int deliver_data(void *arg);
int stash_data(void *arg);
int check_stash(void *arg);
int clean_up(void *arg);
int print_error(void *arg);
int end_state(void *arg);

enum state_codes { ENTRY, PARSEARGS, SETUP,
    FATALERROR, CLEANUP, END}; //must be the same order as states

enum ret_codes { ok, error, repeat};

struct transition {
    enum state_codes src_state;
    enum ret_codes   ret_code;
    enum state_codes dst_state;
};

#endif
