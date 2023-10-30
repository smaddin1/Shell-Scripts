#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

void execute_with_execlp(char *command, char *args[]) {
    if (execlp(command, command, args[0], args[1], args[2], args[3], NULL) == -1) {
        perror("execlp");
        exit(EXIT_FAILURE);
    }
}

void execute_with_execvp(char *command, char *args[]) {
    if (execvp(command, args) == -1) {
        perror("execvp");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-execlp|-execvp] <command> [args]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int use_execlp = 0;
    char *command;
    char *args[5] = {NULL};

    if (strcmp(argv[1], "-execlp") == 0) {
        use_execlp = 1;
        command = argv[2];
        for (int i = 0; i < 4 && i + 3 < argc; i++) {
            args[i] = argv[i + 3];
        }
    } else if (strcmp(argv[1], "-execvp") == 0) {
        command = argv[2];
        args[0] = argv[2];
        for (int i = 1; i < 4 && i + 2 < argc; i++) {
            args[i] = argv[i + 2];
        }
    } else {
        command = argv[1];
        args[0] = argv[1];
        for (int i = 1; i < 4 && i + 2 < argc; i++) {
            args[i] = argv[i + 2];
        }
    }


    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
        if (use_execlp) {
            execute_with_execlp(command, args);
        } else {
            execute_with_execvp(command, args);
        }
    } else { // Parent process
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
        printf("-- The end of shell_v1 --\n");
    }

    return EXIT_SUCCESS;
}
