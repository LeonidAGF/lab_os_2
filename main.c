#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
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

int main() {
    long cap_name = 16;
    long len_name = 0;
    char* filename = (char*)malloc(cap_name * sizeof(char));
    if (!filename) {
        perror("Ошибка выделения памяти");
        return 1;
    }
    filename[0] = '\0';

    printf("Введите имя файла для результатов: ");
    char c = getchar();

    while (c != '\n' && c != EOF) {
        plus_char(&filename, c, &cap_name, &len_name);
        c = getchar();
    }

    while (len_name > 0 && (filename[len_name - 1] == ' ' || filename[len_name - 1] == '\r')) {
        len_name--;
        filename[len_name] = '\0';
    }

    if (len_name == 0) {
        printf("Имя файла не может быть пустым.\n");
        free(filename);
        return 1;
    }

    int pipe1[2];
    int pipe2[2];

    if (pipe(pipe1) < 0 || pipe(pipe2) < 0) {
        perror("Ошибка создания pipe");
        free(filename);
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        free(filename);
        return 1;
    }

    if (pid == 0) {
        close(pipe1[1]);
        close(pipe2[0]);

        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);

        dup2(pipe2[1], 3);
        close(pipe2[1]);

        char fd_str[10];
        snprintf(fd_str, sizeof(fd_str), "%d", 3);

        execl("./child", "child", filename, fd_str, NULL);

        exit(1);
    } else {
        close(pipe1[0]);
        close(pipe2[1]);

        char response[4];
        printf("Введите числа:\n");

        while (1) {
            printf("> ");

            long cap_line = 16;
            long len_line = 0;
            char* line = (char*)malloc(cap_line * sizeof(char));
            if (!line) {
                break;
            }
            line[0] = '\0';

            c = getchar();
            while (c != '\n' && c != EOF) {
                plus_char(&line, c, &cap_line, &len_line);
                c = getchar();
            }

            if (len_line == 0) {
                free(line);
                if (c == EOF) break;
                continue;
            }

            if (strcmp(line, "exit") == 0) {
                free(line);
                break;
            }

            plus_char(&line, '\n', &cap_line, &len_line);

            write(pipe1[1], line, len_line);

            ssize_t bytes_read = read(pipe2[0], response, 3);

            if (bytes_read > 0) {
                response[3] = '\0';
                if (strncmp(response, "ERR", 3) == 0) {
                    free(line);
                    break;
                }
            } else {
                free(line);
                break;
            }

            free(line);
        }

        close(pipe1[1]);
        close(pipe2[0]);
        waitpid(pid, NULL, 0);

        printf("Работа завершена.\n");
        free(filename);
    }

    return 0;
}