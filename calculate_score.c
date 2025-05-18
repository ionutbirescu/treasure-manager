#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "treasure.h"
#define MAX_USERNAME 256
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_id>\n", argv[0]);
        return 1;
    }

    char treasure_path[256];
    snprintf(treasure_path, sizeof(treasure_path), "%s/treasures.dat", argv[1]);
    int fd = open(treasure_path, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open treasure file");
        return 1;
    }

    typedef struct {
        char username[MAX_USERNAME];
        int score;
    } UserScore;

    UserScore users[100]; 
    int user_count = 0;

    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        int found = 0;
        for (int i = 0; i < user_count; ++i) {
            if (strcmp(users[i].username, t.username) == 0) {
                users[i].score += t.value;
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(users[user_count].username, t.username);
            users[user_count].score = t.value;
            user_count++;
        }
    }

    for (int i = 0; i < user_count; ++i) {
        printf("%s: %d points\n", users[i].username, users[i].score);
    }

    close(fd);
    return 0;
}
