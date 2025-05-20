#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "treasure.h"

int terminate_flag = 0;
int process_command_flag = 0;

void handle_sigusr1(int sig) {
    process_command_flag = 1;
}

void handle_sigusr2(int sig) {
    terminate_flag = 1;
}

int count_treasures(const char *hunt_dir) {
    char treasure_path[256];
    snprintf(treasure_path, sizeof(treasure_path), "%s/treasures.dat", hunt_dir);
    int fd = open(treasure_path, O_RDONLY);
    if (fd == -1) return 0;

    int count = 0;
    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        count++;
    }
    close(fd);
    return count;
}

void list_hunts() {
    DIR *dir = opendir(".");
    if (!dir) {
        perror("Failed to open current directory");
        return;
    }

    char buffer[512];
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0 && 
            strcmp(entry->d_name,".git")!=0 &&
            strcmp(entry->d_name,".vscode")!=0) {
            int count = count_treasures(entry->d_name);
            snprintf(buffer, sizeof(buffer), "%s: %d treasures\n", entry->d_name, count);
            write(STDOUT_FILENO, buffer, strlen(buffer));
        }
    }
    closedir(dir);
}

void monitor_loop() {
    while (!terminate_flag) {
        if (process_command_flag) {
            process_command_flag = 0;

            FILE *cmd_file = fopen(".hub_cmd", "r");
            if (!cmd_file) {
                perror("Cannot open .hub_cmd");
                continue;
            }

            char command[256];
            if (fgets(command, sizeof(command), cmd_file)) {
                command[strcspn(command, "\n")] = '\0'; // strip newline

                if (strncmp(command, "list_hunts", 10) == 0) {
                    list_hunts();
                }
                else if (strncmp(command, "list_treasures", 14) == 0) {
                    char hunt[128];
                    if (sscanf(command, "list_treasures %s", hunt) == 1) {
                        char buffer[512];
                        snprintf(buffer, sizeof(buffer), "./treasure_manager --list %s", hunt);
                        FILE *fp = popen(buffer, "r");
                        if (fp) {
                            char line[256];
                            while (fgets(line, sizeof(line), fp)) {
                                write(STDOUT_FILENO, line, strlen(line));
                            }
                            pclose(fp);
                        }
                    }
                }
                else if (strncmp(command, "view_treasure", 13) == 0) {
                    char hunt[128];
                    int id;
                    if (sscanf(command, "view_treasure %s %d", hunt, &id) == 2) {
                        char buffer[512];
                        snprintf(buffer, sizeof(buffer), "./treasure_manager --view %s %d", hunt, id);
                        FILE *fp = popen(buffer, "r");
                        if (fp) {
                            char line[256];
                            while (fgets(line, sizeof(line), fp)) {
                                write(STDOUT_FILENO, line, strlen(line));
                            }
                            pclose(fp);
                        }
                    }
                }
                else if (strncmp(command, "calculate_score", 15) == 0) {
                    char hunt[128];
                    if (sscanf(command, "calculate_score %s", hunt) == 1) {
                        char buffer[512];
                        snprintf(buffer, sizeof(buffer), "./calculate_score %s", hunt);
                        FILE *fp = popen(buffer, "r");
                        if (fp) {
                            char line[256];
                            while (fgets(line, sizeof(line), fp)) {
                                write(STDOUT_FILENO, line, strlen(line));
                            }
                            pclose(fp);
                        }
                    }
                }
                else {
                    char msg[256];
                    snprintf(msg, sizeof(msg), "Unknown command: %.230s\n", command); // limit to avoid buffer overflow
                    write(STDOUT_FILENO, msg, strlen(msg));
                }
            }

            fclose(cmd_file);
        }
        usleep(100000); 
    }

    printf("Monitor is shutting down...\n");
    usleep(500000);
    exit(0);
}

int main() {
    struct sigaction sa1, sa2;
    sa1.sa_handler = handle_sigusr1;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    sigaction(SIGUSR1, &sa1, NULL);

    sa2.sa_handler = handle_sigusr2;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = 0;
    sigaction(SIGUSR2, &sa2, NULL);

    printf("Monitor running (PID: %d)\n", getpid());
    monitor_loop();
    return 0;
}
