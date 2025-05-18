#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>

pid_t monitor_pid = -1;
int monitor_exited = 0;

void sigchld_handler(int sig)
{
    int status;
    waitpid(monitor_pid, &status, WNOHANG);
    monitor_exited = 1;
}

void write_command(const char *command)
{
    int fd = open(".hub_cmd", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("Cannot open .hub_cmd");
        return;
    }
    write(fd, command, strlen(command));
    write(fd, "\n", 1);
    close(fd);
}

void strip_newline(char *str)
{
    for (int i = 0; str[i]; ++i)
    {
        if (str[i] == '\n')
        {
            str[i] = '\0';
            break;
        }
    }
}

int main()
{
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    char input[256];

    while (1)
    {
        printf("> ");
        fflush(stdout);
        if (!fgets(input, sizeof(input), stdin))
            break;

        strip_newline(input);

        if (strcmp(input, "start_monitor") == 0)
        {
            if (monitor_pid != -1)
            {
                printf("Monitor already running.\n");
                continue;
            }
            monitor_pid = fork();
            if (monitor_pid == 0)
            {
                execl("./monitor", "./monitor", NULL);
                perror("Failed to exec monitor");
                exit(1);
            }
            else
            {
                printf("Started monitor (PID: %d)\n", monitor_pid);
            }
        }
        else if (strcmp(input, "stop_monitor") == 0)
        {
            if (monitor_pid == -1)
            {
                printf("No monitor running.\n");
                continue;
            }
            kill(monitor_pid, SIGUSR2);
            printf("Sent termination signal to monitor.\n");
        }
        else if (strcmp(input, "exit") == 0)
        {
            if (monitor_pid != -1 && !monitor_exited)
            {
                printf("Monitor still running! Stop it first.\n");
            }
            else
            {
                break;
            }
        }
        else if (strncmp(input, "list_hunts", 10) == 0 ||
                 strncmp(input, "list_treasures", 14) == 0 ||
                 strncmp(input, "view_treasure", 13) == 0 ||
                 strncmp(input,"calculate_score", 15) == 0)
        {
            if (monitor_pid == -1 || monitor_exited)
            {
                printf("No active monitor.\n");
                continue;
            }
            int pipefd[2];
            pipe(pipefd);

            FILE *pf = fopen(".pipe_fd", "w");
            if (pf) {
                fprintf(pf, "%d\n", pipefd[1]); 
                fclose(pf);
            }

            write_command(input);
            kill(monitor_pid, SIGUSR1);

            char buffer[256];
            int bytes;
            while ((bytes = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytes] = '\0';
                printf("%s", buffer);
            }

            close(pipefd[0]);
            close(pipefd[1]);
        
        }
        else
        {
            printf("Unknown command.\n");
        }
    }

    return 0;
}
