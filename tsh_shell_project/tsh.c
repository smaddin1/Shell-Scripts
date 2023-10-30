#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARG_SIZE 100
#define BUILT_IN_COMMANDS 4
#define MAX_PATH_SIZE 10
#define HISTORY_SIZE 50

char *built_in[] = {"exit", "cd", "path", "history"};
char *path[MAX_PATH_SIZE];  // default paths initialized to empty
int pathCount = 0;
char *history[HISTORY_SIZE];
int historyCount = 0;

void execute_command(char **args, char *exec_type);
void handle_builtin_command(char **args);
void print_history(int n);
void add_to_history(char *cmd);
char *search_in_path(char *cmd);

int main(int argc, char **argv) {
    char *exec_type = "execlp";

    if (argc == 2 && strcmp(argv[1], "-execvp") == 0) {
        exec_type = "execvp";
        printf("*Based on your choice, execvp() will be used *\n");
    }

    char input[MAX_INPUT_SIZE];
    printf("tsh> ");
    while (fgets(input, sizeof(input), stdin)) {
        input[strlen(input) - 1] = '\0';  // remove newline

        add_to_history(input);

        char *args[MAX_ARG_SIZE];
        int arg_count = 0;
        char *token = strtok(input, " ");
        while (token != NULL) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;

        if (arg_count > 0) {
            int built_in_flag = 0;
            for (int i = 0; i < BUILT_IN_COMMANDS; i++) {
                if (strcmp(args[0], built_in[i]) == 0) {
                    handle_builtin_command(args);
                    built_in_flag = 1;
                    break;
                }
            }
            if (!built_in_flag) {
                execute_command(args, exec_type);
            }
        }

        printf("tsh> ");
    }
    return 0;
}

void execute_command(char **args, char *exec_type) {
    int in = 0;
    char *input_file = NULL;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            input_file = args[i + 1];
            args[i] = NULL;
            break;
        }
    }
    if (input_file) {
        in = open(input_file, O_RDONLY);
    }

    pid_t pid, wpid;
    int status;

    if ((pid = fork()) == 0) {  // Child process
        if (in) {
            dup2(in, STDIN_FILENO);
            close(in);
        }

        char *full_cmd = search_in_path(args[0]);
        if (full_cmd) {
            args[0] = full_cmd;
            execvp(full_cmd, args);
            free(full_cmd);
        } else {
            if (strcmp(exec_type, "execvp") == 0) {
                execvp(args[0], args);
            } else {
                execlp(args[0], args[0], NULL);
            }
        }
        char error_message[] = "An error has occurred (from JD)\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("tsh");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

void handle_builtin_command(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL || args[2] != NULL) {
            char error_message[] = "An error has occurred (from JD)\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
        } else {
            chdir(args[1]);
        }
    } else if (strcmp(args[0], "path") == 0) {
        if (args[1] == NULL) {
            printf("path is set to:\n");
            for (int i = 0; i < pathCount; i++) {
                printf("%s\n", path[i]);
            }
            return;
        }

        pathCount = 0;
        char *token = strtok(args[1], ":");
        while (token != NULL && pathCount < MAX_PATH_SIZE) {
            path[pathCount++] = strdup(token);  // Added strdup to allocate memory for each path
            token = strtok(NULL, ":");
        }
    } else if (strcmp(args[0], "history") == 0) {
        int n = HISTORY_SIZE;
        if (args[1] != NULL) {
            n = atoi(args[1]);
        }
        print_history(n);
    }
}

void print_history(int n) {
    for (int i = 0; i < n && i < historyCount; i++) {
        printf("%s\n", history[i]);
    }
}

void add_to_history(char *cmd) {
    if (historyCount < HISTORY_SIZE) {
        history[historyCount++] = strdup(cmd);
    } else {
        free(history[0]);
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            history[i] = history[i + 1];
        }
        history[HISTORY_SIZE - 1] = strdup(cmd);
    }
}

char *search_in_path(char *cmd) {
    char exec_path[MAX_INPUT_SIZE];
    for (int i = 0; i < pathCount; i++) {
        snprintf(exec_path, sizeof(exec_path), "%s/%s", path[i], cmd);
        if (access(exec_path, X_OK) == 0) {
            return strdup(exec_path);
        }
    }
    return NULL;
}
