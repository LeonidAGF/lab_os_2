#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void plus_char(char** str, char data, long* capacity, long* length) {
    if (*length + 2 > *capacity) {
        *capacity *= 2;
        char* new_str = (char*)realloc(*str, (*capacity) * sizeof(char));
        if (new_str == NULL) {
            exit(1);
        }
        *str = new_str;
    }
    (*str)[*length] = data;
    (*length)++;
    (*str)[*length] = '\0';
}

int main(int argc, char *argv[]) {

    if (argc < 3) {
        return 1;
    }

    const char *filename = argv[1];
    int pipe2_fd = atoi(argv[2]);

    FILE *file = fopen(filename, "w");
    if (!file) {
        return 1;
    }

    long capacity = 16;
    long length = 0;
    char* str = (char*)malloc(capacity * sizeof(char));
    if (!str) {
        return 0;
    }
    str[0] = '\0';

    char c = fgetc(stdin);
    if (c == EOF) {
        free(str);
        return 0;
    }

    while (c != '\n' && c != EOF) {
        plus_char(&str, c, &capacity, &length);
        c = fgetc(stdin);
    }

    char *ptr = str;
    char *endptr;

    float first_num = strtof(ptr, &endptr);

    if (ptr == endptr) {
        write(pipe2_fd, "OK\n", 3);
        free(str);
        return 0;
    }

    ptr = endptr;
    int div_by_zero = 0;

    while (*ptr != '\0') {
        float current_num = strtof(ptr, &endptr);

        if (ptr == endptr) {
            ptr++;
            continue;
        }

        if (current_num == 0.0f) {
            div_by_zero = 1;
            break;
        }

        first_num /= current_num;
        ptr = endptr;
    }

    if (div_by_zero) {
        write(pipe2_fd, "ERR", 3);
        fclose(file);
        free(str);
        exit(1);
    }

    fprintf(file, "Результат: %f\n", first_num);
    fflush(file);
    write(pipe2_fd, "OK\n", 3);
    free(str);

    fclose(file);
    return 0;
}