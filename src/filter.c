#include "../include/filter.h"
#include <ctype.h>
#include <stdlib.h>

char upper_filter(char character)
{
    return (char)toupper(character);
}

char lower_filter(char character)
{
    return (char)tolower(character);
}

char null_filter(char character)
{
    return character;
}

char *filter_string(const char *input_string, const size_t length, volatile int *exit_flag, filter_func filter)
{
    char *output_string = (char *)malloc((length * sizeof(char)) + 1);
    if(output_string == NULL)
    {
        *exit_flag = -2;
        return NULL;
    }
    for(int i = 0; (size_t)i < length; i++)
    {
        output_string[i] = filter(input_string[i]);
    }
    output_string[length] = '\0';
    return output_string;
}
