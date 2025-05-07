#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>     
#include <sys/types.h> 
#include <unistd.h>    
#include <sys/stat.h>  
#include <time.h>      
#include <dirent.h>   
#include <unistd.h>    
#include <stdlib.h>    
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

    write(log_file, action_msg, strlen(action_msg));
    write(log_file, "\n", 1);

    close(log_file);
}

void create_symlink(const char *hunt_id)
{
    char target[256], linkname[256];
    snprintf(target, sizeof(target), "%s/logged_hunt", hunt_id);
    snprintf(linkname, sizeof(linkname), "logged_hunt-%s", hunt_id);
    symlink(target, linkname);
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

    if(write(file, &t, sizeof(Treasure)) !=sizeof(Treasure)) perror("Failed to write treasure");
    else printf("Treasure added with ID %d in %s \n", t.treasure_id, treasurePath);

    char msg[256];
    snprintf(msg, sizeof(msg), "Added treasure id %d by user %s", t.treasure_id, t.username);
    log_action(hunt_id, msg);
    create_symlink(hunt_id);
    close(file);
}

void list(const char *hunt_id) {
    char treasure_path[256];
    snprintf(treasure_path, sizeof(treasure_path), "%s/%s", hunt_id, TREASURE_FILE);

    struct stat st;
    if (stat(treasure_path, &st) == -1) {
        perror("Error getting file info");
        return;
    }

    printf("Hunt: %s\n", hunt_id);
    printf("File size: %ld bytes\n", st.st_size);

    char timebuf[64];
    struct tm *modif_time = localtime(&st.st_mtime);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", modif_time);
    printf("🕒 Last modified: %s\n\n", timebuf);

    int file = open(treasure_path, O_RDONLY);
    if (file == -1) {
        perror("Failed to open treasure file");
        return;
    }

    Treasure t;
    printf("💎 Treasures:\n");
    while (read(file, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        printf("ID: %d\n", t.treasure_id);
        printf("User: %s\n", t.username);
        printf("Location: (%.2f, %.2f)\n", t.latitude, t.longitude);
        printf("Clue: %s\n", t.clue);
        printf("Value: %d\n", t.value);
        printf("------------------------------\n");
    }

    close(file);
}


void view_treasure(const char *hunt_id, int id_to_find){

    char treasure_path[256];
    snprintf(treasure_path, sizeof(treasure_path), "%s/%s" , hunt_id, TREASURE_FILE);

    int file= open(treasure_path, O_RDONLY);
    if(file==-1){
        perror("Failed to open treasure file");
        return;
    }

    Treasure t;
    int found=0;

    while (read(file, &t, sizeof(Treasure))==sizeof(Treasure)){
        if(t.treasure_id==id_to_find){
            printf("ID: %d\n", t.treasure_id);
            printf("username: %s\n",t.username);
            printf("location: (%.2f, %.2f)\n", t.latitude, t.longitude);
            printf("clue: %s\n",t.clue);
            printf("value: %d\n\n",t.value);
            found=1;
            char msg[128];
            snprintf(msg, sizeof(msg), "Viewed treasure ID %d", t.treasure_id);
            log_action(hunt_id, msg);

            break;
        }
    }

    if(!found)
        printf("The treasure with id %d does not exist in hunt %s\n",id_to_find,hunt_id);
    close(file);
 }

void load_treasures_from_file(const char *hunt_id, const char *filename) {
    FILE *input = fopen(filename, "r");
    if (!input) {
        perror("Failed to open input file");
        return;
    }

    if (mkdir(hunt_id, 0777) == -1 && errno != EEXIST) {
        perror("mkdir failed");
        fclose(input);
        return;
    }

    char treasure_path[256];
    snprintf(treasure_path, sizeof(treasure_path), "%s/%s", hunt_id, TREASURE_FILE);

    int file = open(treasure_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (file == -1) {
        perror("Failed to open treasure binary file");
        fclose(input);
        return;
    }

    off_t filesize = lseek(file, 0, SEEK_END);
    int current_id = (filesize == -1) ? 0 : filesize / sizeof(Treasure);

    Treasure t;
    while (fscanf(input, "%s %lf %lf \" %[^\"]\" %d", t.username, &t.latitude, &t.longitude, t.clue, &t.value) == 5) {
        t.treasure_id = current_id++;

        if (write(file, &t, sizeof(Treasure)) != sizeof(Treasure)) {
            perror("Write failed");
        } else {
            printf("Loaded treasure ID %d: %s\n", t.treasure_id, t.username);
            char msg[256];
            snprintf(msg, sizeof(msg), "Loaded from file: ID %d by %s", t.treasure_id, t.username);
            log_action(hunt_id, msg);
        }
    }

    create_symlink(hunt_id);
    close(file);
    fclose(input);
}

void remove_treasure(const char *hunt_id, int id_to_remove) {
    char treasure_path[256];
    snprintf(treasure_path, sizeof(treasure_path), "%s/%s", hunt_id, TREASURE_FILE);

    int file = open(treasure_path, O_RDONLY);
    if (file == -1) {
        perror("Failed to open treasure file");
        return;
    }

    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "%s/temp_treasures.dat", hunt_id);
    int temp_file = open(temp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_file == -1) {
        perror("Failed to create temporary file");
        close(file);
        return;
    }

    Treasure t;
    int found = 0;
    while (read(file, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.treasure_id != id_to_remove) {
            write(temp_file, &t, sizeof(Treasure));
        } else {
            found = 1;
        }
    }

    close(file);
    close(temp_file);

    if (found) {
        rename(temp_path, treasure_path);
        printf("Removed treasure ID %d from hunt %s.\n", id_to_remove, hunt_id);

        char msg[256];
        snprintf(msg, sizeof(msg), "Removed treasure ID %d", id_to_remove);
        log_action(hunt_id, msg);
    } else {
        printf("Treasure ID %d not found in hunt %s.\n", id_to_remove, hunt_id);
        remove(temp_path); 
    }
}

void remove_hunt(const char *hunt_id) {
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", hunt_id);

    if (system(command) == 0) {
        printf("Removed hunt directory: %s\n", hunt_id);
    } else {
        perror("Failed to remove hunt directory");
    }

    char symlink_name[256];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    if (unlink(symlink_name) == 0) {
        printf("Removed symlink: %s\n", symlink_name);
    } else {
        perror("Failed to remove symlink");
    }
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
    else if (strcmp(command, "--remove_treasure") == 0)
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
    }
    return 0;
}