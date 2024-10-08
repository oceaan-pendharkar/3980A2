//
// Created by op on 14/09/2024
//

#ifndef FILTER_H
#define FILTER_H

#include <ctype.h>
#include <stddef.h>

typedef int (*filter_func)(char);

int upper_filter(char character);

int lower_filter(char character);

int null_filter(char character);

char *filter_string(const char *input_string, const size_t length, volatile int *exit_flag, filter_func filter);

#endif    // FILTER_H
