//
// Created by op on 03/10/2024
//

#ifndef CLIENT_H
#define CLIENT_H

typedef struct
{
    // cppcheck-suppress unusedStructMember
    char filter_type;
    // cppcheck-suppress unusedStructMember
    char *message;
    // cppcheck-suppress unusedStructMember
    char *server_input;
    // cppcheck-suppress unusedStructMember
    const char *inputFifo;
    // cppcheck-suppress unusedStructMember
    const char *outputFifo;
    // cppcheck-suppress unusedStructMember
    int fd;
    // cppcheck-suppress unusedStructMember
    int exit_flag;

} Client_Settings;

int   parse_arguments(int argc, char *args[], Client_Settings *settings);
void  initialize_fifos(Client_Settings *settings);
void  write_string_to_fd(char *input, const int *fd, int *err);
void  read_string_from_fd(unsigned long length, const int *fd, int *err);
char *initialize_input_string(const Client_Settings *settings, int *err);
void  send_server_request(Client_Settings *settings);
void  receive_server_response(Client_Settings *settings);
void  cleanup(Client_Settings *settings);

#endif    // CLIENT_H
