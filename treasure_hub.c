#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

pid_t monitor_pid = -1;
int monitor_exited = 0;

void sigchld_handler(int sig) {
    int status;
    pid_t pid;

    // Reap all terminated children
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (pid == monitor_pid) {
            monitor_exited = 1;
        }
    }
}

void write_command(const char *command) {
    int fd = open(".hub_cmd", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Cannot open .hub_cmd");
        return;
    }
    write(fd, command, strlen(command));
    write(fd, "\n", 1);
    close(fd);
}

void strip_newline(char *str) {
    for (int i = 0; str[i]; ++i) {
        if (str[i] == '\n') {
            str[i] = '\0';
            break;
        }
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    char input[256];

    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(input, sizeof(input), stdin)) break;

        strip_newline(input);

        if (strcmp(input, "start_monitor") == 0) {
            if (monitor_pid != -1 && !monitor_exited) {
                printf("Monitor already running.\n");
                continue;
            }
            monitor_pid = fork();
            if (monitor_pid == 0) {
                execl("./monitor", "./monitor", NULL);
                perror("Failed to exec monitor");
                exit(1);
            } else {
                monitor_exited = 0;
                printf("Started monitor (PID: %d)\n", monitor_pid);
            }
        }
        else if (strcmp(input, "stop_monitor") == 0) {
            if (monitor_pid == -1 || monitor_exited) {
                printf("No monitor running.\n");
                continue;
            }
            kill(monitor_pid, SIGUSR2);
            printf("Sent termination signal to monitor.\n");
        }
        else if (strcmp(input, "exit") == 0) {
            if (monitor_pid != -1 && !monitor_exited) {
                printf("Monitor still running! Stop it first.\n");
            } else {
                break;
            }
        }
        else if (strncmp(input, "list_hunts", 10) == 0 ||
                 strncmp(input, "list_treasures", 14) == 0 ||
                 strncmp(input, "view_treasure", 13) == 0 ||
                 strncmp(input, "calculate_score", 15) == 0) {

            if (monitor_pid == -1 || monitor_exited) {
                printf("No active monitor.\n");
                continue;
            }

            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("pipe");
                continue;
            }

            pid_t helper = fork();
            if (helper == 0) {
                // CHILD: redirect STDOUT to write-end of pipe
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);

                write_command(input);
                kill(monitor_pid, SIGUSR1);
                exit(0);
            } else {
                // PARENT: read from read-end
                close(pipefd[1]);
                char buffer[256];
                int bytes;
                while ((bytes = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
                    buffer[bytes] = '\0';
                    printf("%s", buffer);
                }
                close(pipefd[0]);
                waitpid(helper, NULL, 0);
            }
        }
        else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}
