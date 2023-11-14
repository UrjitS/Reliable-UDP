#include "server.h"

static int compare_transitions(const void *a, const void *b);
static enum state_codes lookup_transitions(enum state_codes current_state, enum ret_codes return_code);

int (* state[])(void *arg) = {entry_state, parse_args, set_up,
                              print_error, clean_up, end_state}; //function pointers go here

struct transition state_transitions[] = {
        {ENTRY, ok, PARSEARGS},
        {PARSEARGS, ok, SETUP},
        {PARSEARGS, error, FATALERROR},
        {SETUP, ok, CLEANUP},
        {SETUP, error, FATALERROR},
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

    while (1) { //TODO: implement exit flag
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
