#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "treasure.h"

void add_treasure(const char* hunt_id);
void list_treasures(const char* hunt_id);
void view_treasure(const char* hunt_id, const char* treasure_id);
void remove_treasure(const char* hunt_id, const char* treasure_id);
void remove_hunt(const char* hunt_id);

void add_treasure(const char* hunt_id){
    char dir_path[128]; //Here we create the hunt directory if it doesn't exist
    snprintf(dir_path, sizeof(dir_path), "%s", hunt_id);
    if(mkdir(dir_path,0700)==-1){
        perror("Failed to create directory");
        return 1;
    }

    Treasure t; //Now we create the treasure with the data from stdin

    printf("Enter Treasure ID: ");
    fgets(t.treasure_id,MAX_ID_LEN,stdin);

    printf("Enter username: ");
    fgets(t.username,MAX_USER_LEN,stdin);

    printf("Enter Latitude: ");
    if(scanf("%f",&t.latitude)!=1){
        fprintf(stderr,"Invalid Latitude\n");
        return ;
    }
    
    printf("Enter Longitude: ");
    if(scanf("%f",&t.longitude)!=1){
        fprintf(stderr,"Invalid Longitude\n");
        return ;
    }

    printf("Enter clue: ");
    fgets(t.clue,MAX_CLUE_LEN,stdin);

    printf("Enter value: ");
    if(scanf("%d",&t.value)!=1){
        fprintf(stderr,"Invalid Value\n");
        return ;
    }

    char treasure_file[256];
    snprintf(treasure_file,sizeof(treasure_file),"%s",hunt_id);
    int fd=open(filename,O_RDWR | O_CREAT,0666);
    if(fd<0){
        perror("Error opening treasure file\n");
        return ;
    }

    write(fd,treasure,sizeof(Treasure));
    close(fd);

}


int main(int argc, char* argv[]){
    if(argc<3){
        printf("Usage: %s <command> <hunt_id> [treasure_id]\n",argv[0]);
        return 1;
    }

    const char* command = argv[1];
    const char* hunt_id=argv[2];

    if (strcmp(command, "add") == 0) {
        add_treasure(hunt_id);
    } /*else if (strcmp(command, "list") == 0) {
        list_treasures(hunt_id);
    } else if (strcmp(command, "view") == 0 && argc == 4) {
        int treasure_id = atoi(argv[3]);
        view_treasure(hunt_id, treasure_id);
    } else if (strcmp(command, "remove_treasure") == 0 && argc == 4) {
        int treasure_id = atoi(argv[3]);
        remove_treasure(hunt_id, treasure_id);
    } else if (strcmp(command, "remove_hunt") == 0) {
        remove_hunt(hunt_id);
    } else {
        printf("Unknown command or incorrect arguments\n");
    }*/
    return 0;
}