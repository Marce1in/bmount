#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

/**
* Filter and parse the string into a array of strings divided in Device, Size and Type.
* */
char **filter_input(char *input, size_t input_size, size_t *filter_input_size);


#endif
