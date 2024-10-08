//
// Created by op on 14/09/2024
//

#ifndef FILTER_H
#define FILTER_H

#include <ctype.h>
#include <stddef.h>

typedef char (*filter_func)(char);

char upper_filter(char character);

char lower_filter(char character);

char null_filter(char character);

char *filter_string(const char *input_string, const size_t length, volatile int *exit_flag, filter_func filter);

#endif    // FILTER_H
