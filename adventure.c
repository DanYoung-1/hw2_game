#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <dirent.h>

// intialize the mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void* writeTime() {
    // lock the mutex
    pthread_mutex_lock(&mutex);
    
    // Path
    int file_descr = open("currentTime.txt", O_CREAT | O_WRONLY | O_TRUNC, 0600);
    
    // catch error
    if (file_descr < 0) {
        fprintf(stderr, "Couldn't open %s\n", "currentTime.txt");
        perror("Error in main()");
        exit(1);
    }
    
    char timestamp[255];
    memset(timestamp,0,strlen(timestamp));
    
    // get raw and local time
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime( &rawtime );
    
    //1:03pm, Tuesday, September 13, 2016
    strftime(timestamp, 255,"  %I:%M%p, %A, %B, %d, %Y", info);
    ssize_t nwritten;
    nwritten = write(file_descr, timestamp, strlen(timestamp) * sizeof(char));
    
    // unlock mutex
    pthread_mutex_unlock(&mutex);
    return 0;
}

int main(int argc, const char * argv[]) {

    // declare time thread
    pthread_t timeThread;
    // lock mutex
    pthread_mutex_lock(&mutex);
    // create timeThread with writetime function parameter
    pthread_create(&timeThread, NULL, writeTime, NULL);
    
    // read in game room files
    // use stat() on directories and use the one with most recent st_mtime component
    int newestDirTime = -1; // Modified timestamp of newest subdir examined
    char targetDirPrefix[32] = "youndani.rooms."; // Prefix we're looking for
    char newestDirName[256]; // Holds the name of the newest dir that contains prefix
    memset(newestDirName, '\0', sizeof(newestDirName));
    
    DIR* dirToCheck; // Holds the directory we're starting in
    struct dirent *fileInDir; // Holds the current subdir of the starting dir
    struct stat dirAttributes; // Holds information we've gained about subdir
    
    dirToCheck = opendir("."); // Open up the directory this program was run in
    
    
    if (dirToCheck > 0) // Make sure the current directory could be opened
    {
        while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
        {
            if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
            {
                stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry
                
                if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
                {
                    newestDirTime = (int)dirAttributes.st_mtime;
                    memset(newestDirName, '\0', sizeof(newestDirName));
                    strcpy(newestDirName, fileInDir->d_name);
                }
            }
        }
    }
    
    // Close the directory we opened
    closedir(dirToCheck);
    
    //declare file reading variables
    DIR *d;
    struct dirent *dir;
    char filepath[256];
    int file_descr;
    
    d = opendir(newestDirName);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            
            memset(filepath,0,strlen(filepath)); // create file path
            sprintf(filepath + strlen(filepath), "%s", newestDirName);
            if (dir->d_name[0] == '.') continue; // skip unix dir switches
            if (dir->d_name[0] == '-') { // > indicates start room ( >RoomName )
                sprintf(filepath + strlen(filepath), "/%s", dir->d_name);
                
                // open with read only flag
                file_descr = open(filepath, O_RDONLY);
                if (file_descr < 0) {
                    fprintf(stderr, "Could not open %s\n", filepath);
                }
            }
            
        }
    }
    
    // initialize game variables
    int stepCount = 0;
    char nextRoom[30];
    int atEndRoom = 0;
    char path[64][25]  = {0};
/*
     CURRENT LOCATION: abc
     POSSIBLE CONNECTIONS: abc, def, 123. *note commas period on end of 2nd line
     WHERE TO? > *cursor*
     */
    // game loop
    while (!atEndRoom){
        if (stepCount) { // read in file if not start room
            
            d = opendir(newestDirName);
            if (d) {
                while ((dir = readdir(d)) != NULL) {
                    // create file path
                    memset(filepath,0,strlen(filepath));
                    sprintf(filepath + strlen(filepath), "%s", newestDirName);
                    // try reading all files and comparing to nextroom
                    if (!strcmp(dir->d_name,nextRoom)) {
                        sprintf(filepath + strlen(filepath), "/%s", dir->d_name);
                        
                        file_descr = open(filepath, O_RDONLY);
                        if (file_descr < 0) {
                            fprintf(stderr, "Could not open %s\n", filepath);
                        }
                    } else { // next try ignoring the first character (indicating start or end)
                        char subbuff[30] = {0};
                            memcpy( subbuff, &dir->d_name[1], strlen(nextRoom));
                            subbuff[29] = '\0';
                            
                            if (!strcmp(subbuff,nextRoom)) { // start and end rooms
                                char startRoomChar = '-';
                                char endRoomChar = '_';
                                if ((dir->d_name[0] == startRoomChar)  || (dir->d_name[0] == endRoomChar) ) {
                                    if (dir->d_name[0] == endRoomChar) atEndRoom = 1; // break game loop
                                
                                    sprintf(filepath + strlen(filepath), "/%s", dir->d_name);
                                    file_descr = open(filepath, O_RDONLY);
                                    if (file_descr < 0) {
                                        fprintf(stderr, "Could not open %s\n", filepath);
                                    }
                                }
                            }
                    }
                }
            }
        }
        
    // read room - initalize outside variables
        int lineCount = 0;
        char *lines[8] = {0};
        { // intitialize local variables
            char *line = {0};
            char readBuffer[256] = {0};
            char *savePtr1;
            char buffTemp[256] = {0};
            ssize_t nread;

            nread = read(file_descr, readBuffer, sizeof(readBuffer));
            strcpy(buffTemp,readBuffer);
            line = strtok_r(buffTemp, "\n", &savePtr1);
            // read line by line into lines
            do {
                        lines[lineCount] = line;
                        lineCount++;

            } while ((line = strtok_r(NULL, "\n", &savePtr1)) != NULL);
        }
    
        // save name
        char name[30];
        {
            // locals
            char tempLine[30] = {0};
            char *savePtr2 = {0};
            strcpy(tempLine, lines[0]);
            char *p = strtok_r(tempLine, " ", &savePtr2);
            char *arr[3] = {0};
            int c = 0;

            // after 3 spaces is the data we want
            while(p != NULL) {
                arr[c++] = p;
                p = strtok_r(NULL, " ", &savePtr2);
            }
            strcpy(name,arr[2]);
        }
        // print victory message and path, exit(0)
        if (atEndRoom) {
            printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepCount);
            int i;
            for(i=0; i<stepCount; i++) {
                printf("%s\n", path[i]);
            }
            exit(0);
        }
    
        // save connections variables
        char connectionArr[6][25] = {0};
        {
            char *savePtr3 = {0};
            char tempLine[50] = {0};
            int i;
            for (i= 1; i < (lineCount-1); i++) {
                strcpy(tempLine, lines[i]);
                char *p = strtok_r(tempLine, " ", &savePtr3);
                char *arr[3] = {0};
                int c = 0;
        
                // after 3 spaces is the data we want
                while(p != NULL) {
                    arr[c++] = p;
                    p = strtok_r(NULL, " ", &savePtr3);
                }
                strcpy(connectionArr[i-1],arr[2]);
            }
        }
    
    
        /*
         CURRENT LOCATION: abc
         POSSIBLE CONNECTIONS: abc, def, 123. *note commas period on end of 2nd line
         WHERE TO? > *cursur*
         */
    
        // Output Loop
    
        int printToggle = 1;
        while (printToggle) { // this loop repeats whenever user chooses a nonexistant room
            {
                printf("CURRENT LOCATION: %s\n", name);
                printf("POSSIBLE CONNECTIONS: ");
                int i;
                for(i=0; i< lineCount-2; i++) {
                    if (i == (lineCount-3)) { // output connections format using number of lines in room file
                        printf("%s.\n", connectionArr[i]); // end
                    } else {
                        printf("%s, ", connectionArr[i]);
                    }
                }
                printf("WHERE TO >");
            }
            // get input
            char input[64];
            scanf("%s", input);
            
            int timeToggle = 1;
            while(timeToggle) { // check if time is input, if not - break while immediatly
                if(!strcmp(input,"time")) {                    
                    //unlock mutex
                    pthread_mutex_unlock(&mutex);
                    // join timethread
                    pthread_join(timeThread, NULL);
                    
                    // now that mutex is unlocked and mainthread has control open time file
                    int file_descr = open("currentTime.txt", O_RDONLY);
                    
                    if (file_descr < 0) {
                        fprintf(stderr, "Couldn't open %s\n", "currentTime.txt");
                        perror("Error in main()");
                        exit(1);
                    }
                    ssize_t nread;

                    // read timestamp
                    char timestamp[255] = {0};
                    memset(timestamp,0,strlen(timestamp));
                    nread = read(file_descr, timestamp, sizeof(timestamp));
                    // output stamp and prompt
                    printf("%s\n\nWHERE TO? >",timestamp);
                    
                    // lock mutex
                    pthread_mutex_lock(&mutex);
                    
                    // break time while
                    timeToggle = 1;
                    // get input
                    scanf("%s", input);
                }
                else timeToggle = 0;
            }
            
            int i; // check the input against connections.
            for (i=0; i<(lineCount-2); i++) {
                if(!strcmp(input,connectionArr[i])) {
                    strcpy(nextRoom,connectionArr[i]); // save nextRoom
                    printf("\n");
                    strcpy(path[stepCount], nextRoom); // save path
                    stepCount++; // inc step
                    printToggle = 0;
                }
            }

            if(printToggle == 1) { // error message
                printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
            }
        }
    }
    return 0;
}
