
#ifndef SERVER_H
#define SERVER_H

typedef struct
{

    // cppcheck-suppress unusedStructMember
    char *input_fifo;
    // cppcheck-suppress unusedStructMember
    char *output_fifo;
} thread_data_t;

char *get_message_content(const char *msg);
char *get_denied_message(void);
void *process_client(void *arg);
int   parse_server_arguments(int argc, char *args[], thread_data_t *data);
/*
 * Author: D'Arcy Smith
 */
void  sigint_handler(int a);
void  process_client_with_thread(thread_data_t *data);

#endif    // SERVER_H
