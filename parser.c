#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#define MEM_ERROR 1

static void parse_string(char *input, int *index, char ***parsed_strings, size_t* size);
static void extract_string(char ***array, size_t *arr_size, char *string, int *iterator, char stop_char, size_t buffer);

static int new_lines_counter(char *input, int index);
static int sections_counter(char *input, int index);
static int lines_counter(char *input, int *index);

jmp_buf savebuf;

char **filter_input(char *input, size_t input_size, size_t *filter_input_size){
    char **filtered_input = NULL;
    *filter_input_size = 0;

    //If malloc or realloc fails, jump back here and free all the memory
    if (setjmp(savebuf) != 0){
        for(int i = 0; i < *filter_input_size; i++){
            free(filtered_input[i]);
        } 
        free(filtered_input);

        perror("malloc or realloc failed");

        return NULL;
    }

    for(int i = 0; i < input_size; i++){

        if(input[i] == '\n'){

            int newl_count = new_lines_counter(input, i);

            if (newl_count == 3){
                i += 2;
            }
            else if (newl_count == 2){
                i++;

                parse_string(input, &i, &filtered_input, filter_input_size);
            }
        }
    }

    return filtered_input;
}

void parse_string(char *input, int *index, char ***parsed_strings, size_t* size){

    const int start = *index + 1;
    const int rows = lines_counter(input, index);

    const int end = *index + 1;
    const int columns = sections_counter(input, start + 1);

    char** tmp = (char**)realloc(*parsed_strings, (*size + (rows * 3)) * sizeof(char**));
    if (tmp == NULL){
        longjmp(savebuf, MEM_ERROR);
    }

    *parsed_strings = tmp;

    for (int i = start, current_column = 0, first_row = true; i < end; i++){

        if (input[i] == ' ' && input[i + 1] != ' '){
            current_column++;
        }
        else if (input[i] == '\n'){
            current_column = 0;
            first_row = false;
            continue;
        }

        //This "size <= 3" makes the program take only the first header and ignore the other headers 
        if (first_row == false || *size <= 3 ){

            if (current_column == 0 && input[i] != ' '){

                extract_string(parsed_strings, size, input, &i, ' ', 32);
            }
            else if (current_column == columns - 1 && input[i] != ' '){

                extract_string(parsed_strings, size, input, &i, ' ', 32);
            }
            else if (current_column == columns && input[i] != ' '){

                extract_string(parsed_strings, size, input, &i, '\n', 64);
            }
        }
    }
}

void extract_string(char ***array, size_t *arr_size, char *string, int *iterator, char stop_char, size_t buffer){

    size_t string_size = 0;

    (*array)[*arr_size] = (char*)malloc(buffer * sizeof(char));
    if ((*array)[*arr_size] == NULL){
        longjmp(savebuf, MEM_ERROR);
    }

    while(true){
        if (string[*iterator] == stop_char || string[*iterator] == '\0'){
            (*iterator)--;
            break;
        }

        (*array)[*arr_size][string_size] = string[*iterator];

        (*iterator)++;
        string_size++;
    }

    (*array)[*arr_size][string_size] = '\0';
    (*arr_size)++;
}


//helpers
int new_lines_counter(char *input, int index){

    int i = 1;
    while(input[index + i] == '\n' || input[index + i] == '\0'){ 
        i++;
    }

    return i;
}

int lines_counter(char *input, int *index){
    int lines = 0;
    int newl_count = 0;

    while(true){

        if (input[*index] == '\n'){
            newl_count = new_lines_counter(input, *index);

            if (newl_count == 3 || input[*index + 1] == '\0'){

                (*index)--;
                break;
            }
            else if (newl_count == 1){
                lines++; 
            }
        }

        (*index)++;
    }

    return lines;
}

int sections_counter(char *input, int index){
    int sections = 0;

    while(true){
        if (input[index] == '\n' || input[index] == '\0'){
            break;
        }
        else if (input[index] == ' ' && input[index + 1] != ' '){
            sections++; 
        }

        index++;
    }

    return sections;
}
