#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>     //O_WRONLY, O_CREAT, O_APPEND
#include <sys/types.h> //off_t
#include <unistd.h>    //lseek
#include <sys/stat.h>  //mkdir,stat
#include <time.h>      //%Y-%m-%d %H:%H:%S
#include <dirent.h>    //opendir,readdir
#include <unistd.h>    //unlink,rmdir
#include <stdlib.h>    //atoi
#include <stdint.h>
#include "treasure.h"

#define MAX_USERNAME 32
#define TREASURE_FILE "treasures.dat"

void log_action(const char *hunt_id, const char *action_msg)
{
    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%s/logged_hunt", hunt_id);

    int log_file = open(log_path, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (log_file == -1)
    {
        perror("Failed to open log file");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "[%Y-%m-%d %H:%M:%S]", t);
    write(log_file, timebuf, strlen(timebuf));

    // action message
    write(log_file, action_msg, strlen(action_msg));
    write(log_file, "\n", 1);

    close(log_file);
}

void create_symlink(const char *hunt_id)
{
    char target[256], linkname[256];
    snprintf(target, sizeof(target), "%s/logged_hunt", hunt_id);
    snprintf(linkname, sizeof(linkname), "logged_hunt-%s", hunt_id);
    symlink(target, linkname); // ignore errors if it already exists
}

void add_treasure(const char *hunt_id)
{
    if (mkdir(hunt_id, 0777) == -1 && errno != EEXIST)
    { // 0777 - read, write, execute
        perror("Failed to create directory");
        return;
    }

    char treasurePath[256];
    snprintf(treasurePath, sizeof(treasurePath), "%s/%s", hunt_id, TREASURE_FILE);
    int file = open(treasurePath, O_RDWR | O_CREAT | O_APPEND, 0644); // 0644->owner can read/write, others can read
    if (file < 0)
    {
        perror("Failed to open treasure file");
        return;
    }

    uint32_t fileSize = lseek(file, 0, SEEK_END);
    if (fileSize < 0)
    {
        perror("Lseek failed");
        close(file);
        return;
    }

    Treasure t;
    t.treasure_id = fileSize / sizeof(Treasure);
    printf("username: ");
    scanf("%s", t.username);
    printf("latitude: ");
    scanf("%lf", &t.latitude);
    printf("longitude: ");
    scanf("%lf", &t.longitude);
    printf("clue: ");
    getchar();
    fgets(t.clue, MAX_CLUE_LEN, stdin);
    printf("value: ");
    scanf("%d", &t.value);

    printf("Treasure added with ID %d in %s \n", t.treasure_id, treasurePath);

    char msg[256];
    snprintf(msg, sizeof(msg), "Added treasure id %d by user %s", t.treasure_id, t.username);
    log_action(hunt_id, msg);
    create_symlink(hunt_id);
    close(file);
}

void list(const char *hunt_id)
{
    char treasure_path[256];
    snprintf(treasure_path, sizeof(treasure_path), "%s/%s", hunt_id, TREASURE_FILE);
    // printf("Full path: %s\n", treasure_path);

    // retrieve usefull file information using stat(size of hunt file, last modification)
    struct stat st; // st-> stat buffer
    if (stat(treasure_path, &st) == -1)
    {
        perror("Error getting file info");
        return;
    }

    printf("Hunt: %s\n", hunt_id);
    printf("File size: %ld bytes\n", st.st_size);

    // time formatting
    char timebuf[64]; // buffer for result
    // tm-> C struct representing a borken-down date and time in hrf
    struct tm *modif_time = localtime(&st.st_mtime);                     // localtime-> converts timestamp to tm structure
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%H:%S", modif_time); // formats tm str into astring
    printf("The last time when the file was modified %s:\n ", timebuf);

    int file = open(treasure_path, O_RDWR);
    if (file == -1)
    {
        perror("Failed to open treasure file");
        return;
    }

    Treasure t;
    printf("\nTreasures:\n");
    while (read(file, &t, sizeof(Treasure)))
    {
        printf("ID: %d\n", t.treasure_id);
        printf("username: %s\n", t.username);
        printf("location: (%.2f, %.2f)\n", t.latitude, t.longitude);
        printf("clue: %s\n", t.clue);
        printf("value: %d\n\n", t.value);
    }
    close(file);
}




int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s --<command> <hunt_id> [<args>]\n", argv[0]);
        return 1;
    }

    const char *command = argv[1];
    const char *hunt_id = argv[2];

    if (strcmp(command, "--add") == 0)
    {
        add_treasure(hunt_id);
    }
    else if (strcmp(command, "--list") == 0)
    {
        list(hunt_id);
    }
    else if (strcmp(command, "--view") == 0)
    {
        if (argc < 4)
        {
            fprintf(stderr, "Usage: %s --view <hunt_id> <id>\n", argv[0]);
            return 1;
        }
        int id = atoi(argv[3]);
        view_treasure(hunt_id, id);
    }
    /*else if (strcmp(command, "--remove_treasure") == 0)
    {
        if (argc < 4)
        {
            fprintf(stderr, "Usage: %s --view <hunt_id> <id>\n", argv[0]);
            return 1;
        }
        int id = atoi(argv[3]);
        remove_treasure(hunt_id, id);
    }
    else if (strcmp(command, "--remove_hunt") == 0)
    {
        remove_hunt(hunt_id);
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }*/

    return 0;
}