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

} Client_Settings;

int   parse_arguments(int argc, char *args[], Client_Settings *settings);
void  write_string_to_fd(char *input, const int *fd, int *err);
void  read_string_from_fd(unsigned long length, const int *fd, int *err);
char *initialize_input_string(const char *filter_type, const char *message, int *err);

#endif    // CLIENT_H
