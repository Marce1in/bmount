#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include <sys/types.h>
#include <unistd.h> 
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/mount.h>

char *get_output(int *fd, size_t *outsize);
int menu(char **arr, size_t arr_size);

int main(void) {
    char* error = NULL;

    // Check if the user is root
    if (geteuid() != 0) {
        fprintf(stderr, "Error! You're not a root user :(\n");
        return -1;
    }

    int fd[2];

    if (pipe(fd) == -1) {
        error = "pipe error";
        goto print_error;
    }

    pid_t pid;

    if ((pid = fork()) == -1) {
        error = "fork error";
        goto print_error;
    }

    if (pid == 0) {

        close(fd[0]);

        //We don't use "goto print_error" here, the child process shouldn't go out
        //of his "if" statement. feels weird
        if (dup2(fd[1], STDOUT_FILENO) == -1) {
            close(fd[1]);
            perror("dup2 falied");
            exit(EXIT_FAILURE);
        }

        close(fd[1]);

        execlp("fdisk", "fdisk", "-l", NULL);

        perror("exec failed on child process");
        exit(EXIT_FAILURE);
    }
    close(fd[1]);

    int status = 0;
    waitpid(pid, &status, WUNTRACED);

    if (status != 0) {
        close(fd[0]);
        error = "fdisk or child process failed";
        goto print_error;
    }

    size_t output_size;
    char *output = get_output(fd, &output_size);

    close(fd[0]);

    if (output == NULL) {
        error = "get_output() failed";
        goto print_error;
    }

    size_t filtered_output_size;
    char **filtered_output = filter_input(output, output_size, &filtered_output_size);
    if (filtered_output == NULL){
        error = "filter_input() failed";
        goto print_error;
    }

    int user_input = menu(filtered_output, filtered_output_size);
    if (user_input == -1) {
        error = "menu() failed";
        goto free_output;
    }

    if ((pid = fork()) == -1) {
        error = "fork error";
        goto free_memory;
    }

    if (pid == 0) {
        char *args[] = {
            "mount", 
            "-o", 
            "nosuid,nodev,rw,user,noatime,exec,umask=0", 
            filtered_output[user_input * 3], 
            "/media/", 
            NULL
        };

        execvp(args[0], args);

        perror("exec failed on child process");
        exit(EXIT_FAILURE);
    }

    int retval = 0;


    free_memory:
        for (int i = 0; i < filtered_output_size; i++) {
            free(filtered_output[i]);
        }
        free(filtered_output);

    free_output:
        free(output);

    print_error:
        if (error != NULL) {
            perror(error);
            retval = -1;
        }

    
    return retval;
}

char *get_output(int *fd, size_t *out_size) {

    size_t buf_size = 2048;
    char *buffer = malloc(buf_size * sizeof(char));
    if (buffer == NULL){
        perror("Malloc error");
        return NULL;
    }

    int bytes_readed = 0;
    size_t buf_index = 0;

    do {
        bytes_readed = read(fd[0], buffer + buf_index, (buf_size - buf_index) * sizeof(char));

        if (bytes_readed == 0) {
            buffer[buf_index] = '\0';
            break;
        } 
        else {
            buf_index += bytes_readed;
            buf_size *= 2;

            void *tmp = realloc(buffer, buf_size * sizeof(char));
            if(tmp == NULL){

                perror("Realloc error");
                free(buffer);
                return NULL;
            }
            buffer = tmp;
        }
    } while (true);

    *out_size = buf_index;
    return buffer;
}

int menu(char **arr, size_t arr_size) {
    printf("+----------------------------------------------------------------+\n");
    printf(" nº  %-15s %-12s %s\n", arr[0], arr[1], arr[2]);
    printf("+----------------------------------------------------------------+\n");
    for (int i = 3; i < arr_size; i += 3) {
        printf(" %d.  %-15s %-12s %s\n", i / 3, arr[i], arr[i + 1], arr[i + 2]);
    }
    printf("+----------------------------------------------------------------+\n");

    printf("\nSelect the partition to mount: ");
    int input = 0;
    if (scanf("%i", &input) < 1) {
        fprintf(stderr, "Invalid input\n");
        errno = EINVAL;
        return -1;
    }

    if (input <= 0 || input * 3 >= arr_size) {
        return -1;
    }

    return input;
}

