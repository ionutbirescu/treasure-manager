#ifndef TREASURE_H
#define TREASURE_H

#define MAX_ID_LEN 32
#define MAX_USER_LEN 32
#define MAX_CLUE_LEN 128

typedef struct {
    char treasure_id[MAX_ID_LEN];    // Unique ID
    char username[MAX_USER_LEN];     // Owner username
    float latitude;                  // GPS latitude
    float longitude;                 // GPS longitude
    char clue[MAX_CLUE_LEN];         // Clue description
    int value;                       // Treasure value
} Treasure;

#endif