#ifndef TREASURE_H
#define TREASURE_H

#define MAX_ID_LEN 32
#define MAX_USER_LEN 32
#define MAX_CLUE_LEN 128

typedef struct {
    int treasure_id;   
    char username[MAX_USER_LEN];     // Owner username
    double latitude;                  // GPS latitude
    double longitude;                 // GPS longitude
    char clue[MAX_CLUE_LEN];         // Clue description
    int value;                       // Treasure value
} Treasure;

#endif