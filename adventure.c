//
// *****************************************************************************
// 
// Author:    Erik Ratcliffe
// Date:      May 4, 2015
// Project:   Program 2 - Adventure
// Filename:  ratclier.adventure.c
// Class:     CS 344 (Spring 2015)
//
//
// Overview:
//    Simple text "Adventure-style" game.
//
// Input:
//    User navigates a series of room seeking the "END ROOM." Adjacent
//    rooms are presented whenever the user enters a new room (one of the
//    adjacent room names is input to continue navigation). 
//
// Output:
//    When the user reaches the END ROOM, the number of steps that it took
//    to get there is presented along with the sequence of visited rooms.
//    If the user does not reach the END ROOM after a certain number of
//    steps (MAX_PATH) the game ends with a condolence message.
//
// *****************************************************************************
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#define MAX_ROOMS 7           // Total rooms, <= number of names in roomNames[]
#define MAX_PATH 14           // The most steps allowed
#define MAX_ROOM_CHARS 15     // Max number of chars in room names


enum roomType { START_ROOM, END_ROOM, MID_ROOM };   // Room classifications


// struct Room: Holds all the information needed for each room:
//
// name   -> Name of the room
//
// adj[]  -> Indicates which rooms connect to it: 0 = no, 1 = yes
//           The indices of the array reference rooms in two ways:
//
//               1st: When setting up the room files, the indices
//                    reflect the order in which the room data is
//                    generated before being output to files.
//
//               2nd: When reading data in from the files, the indices
//                    match the order in which the files were read from
//                    the filesystem.
//
// type   -> The type of room (START, END, or MID)
//
struct Room {
    char name[30];
    int adj[MAX_ROOMS];
    enum roomType type;
};


// *****************************************************************************
// 
// int setupRooms(struct Room *prooms[], char dirName[], char *roomNames[])
//
//    Entry:   struct Room *prooms[]
//                Pointer array containing all Room structs.
//             char dirName[]
//                Array containing the name of the file output directory.
//             char *readRoomNames[]
//                Pointer array containing all Room names read in from files.
//
//    Exit:    Returns 0 for successful execution, 1 for error.
//
//    Purpose: Create Rooms and output the data to files.
//
// *****************************************************************************
//
int setupRooms(struct Room *prooms[], char dirName[], char *roomNames[]);


// *****************************************************************************
// 
// int readRooms(struct Room *prooms[], char dirName[], char *readRoomNames[])
//
//    Entry:   struct Room *prooms[]
//                Pointer array containing all Room structs.
//             char dirName[]
//                Array containing the name of the file output directory.
//             char *readRoomNames[]
//                Pointer array containing all Room names read in from files.
//
//    Exit:    Returns 0 for successful execution, 1 for error.
//
//    Purpose: Read Room data in from files.
//
// *****************************************************************************
//
int readRooms(struct Room *prooms[], char dirName[], char *readRoomNames[]);


// *****************************************************************************
// 
// int playGame(struct Room *prooms[], char *readRoomNames[])
//
//    Entry:   struct Room *prooms[]
//                Pointer array containing all Room structs.
//             char *readRoomNames[]
//                Pointer array containing all Room names read in from files.
//
//    Exit:    Returns 0 for successful execution, 1 for error.
//
//    Purpose: Perform the actual game functionality.
//
// *****************************************************************************
//
int playGame(struct Room *prooms[], char *readRoomNames[]);


// *****************************************************************************
// 
// void cleanRooms(struct Room *prooms[])
//
//    Entry:   struct Room *prooms[] 
//                Pointer array containing all Room structs.
//
//    Exit:    None
//
//    Purpose: Free all pointers used in Room structs.
//
// *****************************************************************************
//
void cleanRooms(struct Room *prooms[]); 


int main() {

    // Room names array. Static, but dynamically assigned to each room.
    // There must be more names here than MAX_ROOMS!
    //
    char *roomNames[10] = {  
        "Mozart",
        "Schubert",
        "Beethoven",
        "Bach",
        "Wagner",
        "Vivaldi",
        "Pachelbel",
        "Satie",
        "Berg",
        "Chopin" };

    // Array holding Room names read in from files (see readRooms()).
    //
    char *readRoomNames[MAX_ROOMS] = {0}; 

    // Array of pointers to Rooms. This array is first used to set up
    // Rooms to be output to files (see setupRooms()), then it is reused
    // to read in Rooms from files (see readRooms()).
    //
    struct Room *prooms[MAX_ROOMS];

    // Holds the name of the output file directory.
    //
    char dirName[30];       

    // Create the output file directory. The name is hardcoded to include
    // my ONID username (ratclier), then ".rooms.", then the PID of the
    // running program. This directory is not removed at the end of the
    // game, per assignment guidelines.
    //
    sprintf(dirName, "%s.%ld", "ratclier.rooms", (long) getpid());
    mkdir(dirName, 0755); // File permissions: u = rwx, g = r-x, o = r-x
                          //                       421      4-1      4-1
                          //                        7        5        5

    // Set up the Room files.
    //
    setupRooms(prooms, dirName, roomNames);

    // Read the Room data in from files.
    //
    readRooms(prooms, dirName, readRoomNames);

    // Play the game.
    //
    playGame(prooms, readRoomNames);

    // Clean up Room struct pointers.
    //
    cleanRooms(prooms);

    return 0;

}


// *****************************************************************************
// 
// setupRooms(struct Room *prooms[], char dirName[], char *roomNames[])
//
// Purpose: Create Rooms and output the data to files.
//
// *****************************************************************************
//
int setupRooms(struct Room *prooms[], char dirName[], char *roomNames[]) {

    int i, x;                    // loop iterators
    int seed;                    // srand seed
    int num;                     // holds number generated by rand()
    int roomNamesUsed[10] = {0}; // when a name is used, mark it as unavailable
    int adjCount;                // number of adjacent Rooms so far

    FILE *fp = NULL;             // output file pointer

    char fullPathName[50];       // holds the full path to a Room file

    // Seed the random number generator
    //
    seed = time(NULL);
    srand(seed);

    //
    // *******************************************************************
    // Set up the array of Room structs. Initially, all adjacent rooms are
    // set to 0 and all room types are set to MID_ROOM for simplicity.
    // *******************************************************************
    //

    for(i = 0; i < MAX_ROOMS; i++) {
        // Generate a random index number for the Room's name, making sure
        // to not reuse a name that has already been allocated. This index
        // will read a name from the roomNames[] array and assign it to
        // the Room.
        //
        do {
            num = (rand() % (10));
        } while(roomNamesUsed[num] == 1);

        // Create a new Room struct.
        //
        struct Room *newRoom = (struct Room *) malloc(sizeof(struct Room));

        // Copy Room name to room
        //
        strcpy(newRoom->name, roomNames[num]);

        // Mark the name as taken so it won't be reused.
        //
        roomNamesUsed[num] = 1;

        // Initialize the adjacent rooms to 0.
        //
        for(x = 0; x < MAX_ROOMS; x++) {
            newRoom->adj[x] = 0;
        }

        // Start off with all rooms set to type MID_ROOM.
        //
        newRoom->type = MID_ROOM;

        // Add newRoom to the prooms[] array.
        //
        prooms[i] = newRoom;
    }

    // Determine START and END rooms. All Rooms are currently set to
    // MID_ROOM so just pick one random Room each for the START and END.
    //
    num = (rand() % (MAX_ROOMS));
    prooms[num]->type = START_ROOM;
    do {
        num = (rand() % (MAX_ROOMS));
    } while(prooms[num]->type == START_ROOM);
    prooms[num]->type = END_ROOM;
 

    // Assign adjacent Rooms. An array, adj[], holds the values that flag
    // a room as being adjacent. The indices are arranged in the order in
    // which the Room structs are positioned in prooms[]: if a Room is
    // adjacent, a 1 is entered into its index in adj[], otherwise the
    // index contains a 0 (not adjacent).
    //
    for(i = 0; i < MAX_ROOMS; i++) {

        adjCount = 0; // Holds the number of adjacent Rooms assigned

        // Assign a minimum of 3 adjacent Rooms for each Room. When adding
        // an adjacent Room, be sure to update the adjacent Room's adj[]
        // array to reflect the two-way connection. By doing this, it's
        // possible that a maximum of MAX_ROOMS-1 Rooms can be adjacent.
        //
        // Be sure to not assign a Room as being adjacent to itself!
        //
        while(adjCount < 3) {
            // Keep calculating random numbers until the number doesn't match
            // the current Room number.
            //
            do {
                num = (rand() % (MAX_ROOMS));
            } while(num == i);

            // Assign 1 to each side of the Room connection.
            //
            prooms[i]->adj[num] = 1;
            prooms[num]->adj[i] = 1;

            adjCount++;
        }

    }

    // For each Room, write one Room file to the output directory.
    //
    for(i = 0; i < MAX_ROOMS; i++) {

        // Build the full path name for the Room file.
        //
        sprintf(fullPathName, "%s/%s", dirName, prooms[i]->name);

        // Open the file for writing.
        //
        fp = fopen(fullPathName, "w");

        // If there was a problem, exit with an error.
        //
        if(fp == NULL) {
            printf("Error opening %s: errno = %d\n", fullPathName, errno);
            exit(1);
        }

        // Write the Room name to the file.
        //
        fprintf(fp, "ROOM NAME: %s\n", prooms[i]->name);

        // Write the adjacent Rooms ("CONNECTIONS") to the file. Be sure
        // to add a count to each of the lines:
        //
        //    CONNECTION 1: Roomname
        //    CONNECTION 2: Roomname
        //    ...
        //
        adjCount = 0;
        for(x = 0; x < MAX_ROOMS; x++) {
            if(prooms[i]->adj[x] == 1) {
                fprintf(fp, "CONNECTION %d: %s\n", ++adjCount, prooms[x]->name);
            }
        }

        // Write the Room type to the file.
        //
        fprintf(fp, "ROOM TYPE: ");
        if(prooms[i]->type == START_ROOM) {
            fprintf(fp, "START_ROOM");
        } else if(prooms[i]->type == MID_ROOM) {
            fprintf(fp, "MID_ROOM");
        } else {
            fprintf(fp, "END_ROOM");
        }

        // To make things pretty when typing to the screen, add a
        // newline to the end of the Room file.
        //
        fprintf(fp, "\n");

        // Close the Room file.
        //
        fclose(fp);
    }

    // Delete all pointers from the prooms[] Room array
    //
    cleanRooms(prooms);

    return 0;

}


// *****************************************************************************
// 
// readRooms(struct Room *prooms[], char dirName[], char *readRoomNames[])
//
// Purpose: Read Room data in from files.
//
// *****************************************************************************
//
int readRooms(struct Room *prooms[], char dirName[], char *readRoomNames[]) {

    int i, x;                 // loop iterators
    int roomCount;            // number of Room files read so far

    FILE *fp = NULL;          // file pointer
    DIR  *dp = NULL;          // directory pointer
    struct dirent *dirFiles;  // struct that holds directory data

    char fullPathName[50];    // holds the full path to a room file

    struct stat filestat;     // stat() information on a directory entity

    // Fields used by fscanf() to read data from lines in the Room files.
    // All files have three fields per line. If any more fields are added,
    // another field# array needs to be added for each new field.
    //
    char field1[MAX_ROOM_CHARS];
    char field2[MAX_ROOM_CHARS];
    char field3[MAX_ROOM_CHARS];

    // Open the output file directory.
    //
    dp = opendir(dirName);

    // If there was a problem opening the output file directory, exit with
    // an error.
    //
    if(dp == NULL) {
        printf("Error opening %s: errno = %d\n", dirName, errno);
        exit(1);
    }

    //
    // *******************************************************************
    // Fill the readRoomNames array with filenames from the directory.
    // We need to do this to have an indexable list of rooms for
    // navigation.
    // *******************************************************************
    //
    
    // Start with no Rooms read.
    //
    roomCount = 0;

    // Read Room files from the output directory.
    //
    while(((dirFiles = readdir(dp)) != NULL) && (roomCount < MAX_ROOMS)) {

        // Each time readdir() runs, another entity is read from the
        // output directory. If the file is a regular file and not ".",
        // ".." add it to the array that holds room names. 
        //
        // I'm trying to be POSIX compliant here by not using DT_REG to
        // check the regular-ness of the file. Supposedly the S_ISREG()
        // macro is more portable.
        //
        sprintf(fullPathName, "%s/%s", dirName, dirFiles->d_name);
        stat(fullPathName, &filestat);
        if(S_ISREG(filestat.st_mode) != 0) {
            readRoomNames[roomCount] = dirFiles->d_name;
            roomCount++;
        }
    }

    //
    // The readRoomNames[] array has been populated. We now have a new
    // ordering for the rooms in readRoomNames[] (based on the order in
    // which they were read in from the output directory) so the adjacent
    // room mapping will need to be indexed based on the position of Room
    // names in readRoomNames[].
    //

    // For each file, open the file and read its contents.
    //
    for(i = 0; i < MAX_ROOMS; i++) {

       // Build the full path to the Room file.
       //
       sprintf(fullPathName, "%s/%s", dirName, readRoomNames[i]);

       // Open the Room file.
       //
       fp = fopen(fullPathName, "r");

       // If there was a problem opening the Room file, exit with an
       // error.
       //
       if(fp == NULL) {
           printf("Error opening %s: errno = %d\n", fullPathName, errno);
           exit(1);
       }

       // Create a new Room struct to hold Room data read in from the file
       // in the output directory.
       //
       struct Room *newRoom = (struct Room *) malloc(sizeof(struct Room));

       // Initialize all adjacent rooms in adj[] to 0 for simplicity.
       //
       for(x = 0; x < MAX_ROOMS; x++) {
           newRoom->adj[x] = 0;
       }

       //
       // Start reading in lines from the Room file. For each line, read
       // the three distinct fields into the three field arrays (field1[],
       // field2[], field3[]) and parse data from the fields according to
       // their positions.
       //
       
       // Read lines from the Room file until EOF.
       //
       while(fscanf(fp, "%s %s %s", field1, field2, field3) != EOF) {

           // If the first field contains "ROOM", move to the second field
           //
           if(strcmp(field1, "ROOM") >= 0) {
               // If the second field contains "NAME", field3 is the
               // Room name. If not, it's the Room type.
               // 
               if(strncmp(field2, "NAME", 4) == 0) {
                   strcpy(newRoom->name, field3);
               } else if(strncmp(field2, "TYPE", 4) == 0) {
                   if(strncmp(field3, "START_ROOM", 10) == 0) {
                       newRoom->type = START_ROOM;
                   } else if(strncmp(field3, "MID_ROOM", 8) == 0) {
                       newRoom->type = MID_ROOM;
                   } else if(strncmp(field3, "END_ROOM", 8) == 0) {
                       newRoom->type = END_ROOM;
                   }
               }
           } else if (strncmp(field1, "CONNECTION", 10) == 0) {
               // For each "CONNECTION", compare field3 (the adjacent Room
               // name) to the readRoomNames[] array and, if you get a
               // match, flag the identical index in adj[] to mark the
               // room as adjacent. 
               //
               // Do not mark the current Room as adjacent in the adjacent
               // room's adj[] array!  That was only necessary when
               // setting up the Room files in setupRooms().
               //
               for(x = 0; x < MAX_ROOMS; x++) {
                   if(strcmp(field3, readRoomNames[x]) == 0) {
                       newRoom->adj[x] = 1;
                   }
               }
           }

           // Add the new Room struct to the prooms[] Room array.
           //
           prooms[i] = newRoom;
       }
    
       // Close the file.
       //
       fclose(fp);

    }

    // Close the output directory.
    //
    closedir(dp);

    return 0;

}


// *****************************************************************************
// 
// playGame(struct Room *prooms[], char *readRoomNames[])
//
// Purpose: Perform the actual game functionality.
//
// *****************************************************************************
//
int playGame(struct Room *prooms[], char *readRoomNames[]) {

    // struct Path: Holds the rooms visited on the "path" to the END ROOM
    //
    // pathIDXs[]  -> Indices of readRoomNames[] that map to rooms visited
    //                along the path.
    //
    // pathCount   -> Tracks how many Rooms were added to the Path
    //
    struct Path {
        int pathIdxs[MAX_PATH];
        int pathCount;
    };

    int i, x;            // loop iterators
    int conn;            // tracks which "CONNECTION" is being processed.
                         //   Used to control display of commas in output.
    int userInputLen;    // holds length of user input. Used to get rid of 
                         //   trailing newline.

    char userInput[MAX_ROOM_CHARS]; // holds string entered by user

    // Initialize to -1 because 0 is a legitimate value.
    //
    int userInputIdx = -1;

    // Set up a new Room struct to always point to the current Room.
    //
    struct Room *currRoom = NULL;

    // Set up a new Path struct to track the rooms visited. Initialize the
    // path indices to all be -1 (0 is a legitimate value so it can not be
    // used to indicate an unvisited Room) and initialize the path count
    // to 0 because we're just starting.
    //
    struct Path path = { {-1}, 0};

    // Always start with START_ROOM. Keep looking until you find it.
    //
    i = 0;
    do {
        if(prooms[i]->type == START_ROOM) {
            currRoom = prooms[i];
        }
        i++;
    } while((currRoom == NULL) && (i < MAX_ROOMS));

    // Start the journey. As long as the user has not reached the END ROOM,
    // or as long as (s)he has not explored for too long without finding it
    // (controlled by MAX_PATH), keep presenting rooms.
    //
    while((currRoom->type != END_ROOM) && (path.pathCount < MAX_PATH)) {

        // Display the current Room name.
        //
        printf("CURRENT LOCATION: %s\n", currRoom->name);

        //
        // Display the adjacent ("CONNECTION") Rooms.
        //
        
        printf("POSSIBLE CONNECTIONS:");
        // The first connection will not be preceded by a comma. Make 
        // sure this does not happen by initializing the conn counter 
        // to 0.
        //
        conn = 0;

        // Walk through the current Room's adj[] array to find adjacent
        // room flags (1 = adjacent, 0 = not adjacent). When a flag is 
        // found, look up the identical index in readRoomNames[] and
        // present the Room name to the user as an adjacent Room.
        //
        for(x = 0; x < MAX_ROOMS; x++) {
            if(currRoom->adj[x] == 1) {
                if(conn == 0) {
                    printf(" %s", readRoomNames[x]);  // first adj Room
                } else {
                    printf(", %s", readRoomNames[x]); // all others
                }
                conn++;
            }
        }

        // Append a period to close out the adjacent room list, then
        // prompt the user for an adjacent Room to visit.
        //
        printf(".\nWHERE TO? >");

        // Read the user's Room input.
        //
        fgets(userInput, MAX_ROOM_CHARS, stdin);

        // Remove trailing newline by replacing it with a null terminator.
        //
        userInputLen = strlen(userInput) - 1; 
        if(userInput[userInputLen] == '\n') {
            userInput[userInputLen] = '\0';
        }

        // Did the user enter a valid adjacent Room name? First, look up
        // the userInput string in readRoomNames[] and, if found, assign
        // the index to userInputIdx...
        //
        for(x = 0; x < MAX_ROOMS; x++) {
            if(strcmp(readRoomNames[x], userInput) == 0) {
                userInputIdx = x;
            }
        }

        // ...next, see if userInputIdx points to an adjacent room in the
        // current Room's adj[] array (remember, the order of Rooms in
        // readRoomNames[], prooms[], and each Room's adj[] array
        // match). If the Room entered is adjacent to the current Room,
        // make the new Room the current Room, add the Room's index to the
        // path, and increment the path counter. Otherwise, start over
        // prompting the user for an adjacent room using the existing
        // current Room's data (don't change any path values. Wrong
        // answers do not count as part of the journey).
        //
        if(currRoom->adj[userInputIdx] == 1) {
            currRoom = prooms[userInputIdx];
            path.pathIdxs[path.pathCount] = userInputIdx;
            path.pathCount++;
        } else {
            printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
        }

        // Pretty things up with a newline.
        //
        printf("\n");
    }

    // At this point, the user has either found the END ROOM or has run
    // out of allowed steps. Figure out which and report accordingly. Note
    // that if the path only contains one Room, report the steps as "STEP"
    // (singular) and not "STEPS" (plural).
    //
    if(currRoom->type == END_ROOM) {
        printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
        printf("YOU TOOK %d STEP%s YOUR PATH TO VICTORY WAS:\n", path.pathCount, (path.pathCount > 1 ? "S." : "."));

        // Print out the path that lead to the END ROOM.
        //
        for(i = 0; i < path.pathCount; i++) {
            printf("%s\n", readRoomNames[path.pathIdxs[i]]);
        }
    } else {
        // The user ran out of allowed steps. Don't bother showing the
        // path. They lost (and it's not required, I believe).
        //
        printf("YOU DIDN'T FIND THE END ROOM IN %d MOVES! SO SORRY.\n", MAX_PATH);
    }

    return 0;

}


// *****************************************************************************
// 
// cleanRooms(struct Room *prooms[])
//
// Free all pointers used in Room structs.
//
// *****************************************************************************
//
void cleanRooms(struct Room *prooms[]) {

    int i;    // loop iterator

    // Step through the prooms[] array of Room pointers and free the
    // memory for each. Set the pointers to NULL afterward to finish them
    // off properly.
    //
    for(i = 0; i < MAX_ROOMS; i++) {
      if(prooms[i] != NULL) {
        free(prooms[i]);
        prooms[i] = 0;
      }
    }

}

