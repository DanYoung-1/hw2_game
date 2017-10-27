#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

// roomType enum
enum RoomType {
    startRoom,
    endRoom,
    midRoom,
};


// structure for Rooms
typedef struct Room {
    char *name;
    int numOutboundConnections;
    enum RoomType type;
    struct Room *outboundConnections[6];
}Room;

void printRoomOutboundConnections( Room* input)
{
    printf("The rooms connected to (%s) are:\n", input->name);
    
    int i;
    for (i = 0; i < input->numOutboundConnections; i++)
        printf("  %s\n", input->outboundConnections[i]->name);
    return;
}

int isGraphFull(Room *rooms, int nRooms) {
    int count = 0;
    int i;
    for (i=0; i < nRooms; i++) {
        count += rooms[i].numOutboundConnections;
        if (rooms[i].numOutboundConnections < 3) { // if a room has less than 3 return false
            return 0;
        }
    }
    if ((count >= (nRooms*3)) && (count <= (nRooms*6)) ) { // product math for number of rooms
        return 1;
    } else {
        return 0;
    }
}

Room* getRandomRoom(Room *rooms, int nRooms) {
    int r = rand() % nRooms; // get a number between 0 and n rooms
    return &rooms[r];
}

int canAddConnectionFrom(Room *x) { // < 6 can add
    if (x->numOutboundConnections <= 6) return 1;
    else return 0;
}

int connectionAlreadyExists(Room *x, Room *y) {
    int i;
    for (i = 0; i < x->numOutboundConnections; i++) {
        if (x->outboundConnections[i] == y) { // check connections against y
            return 1;
        }
    }
    return 0;
}

void connectRoom(Room *x, Room *y) {
    int position = x->numOutboundConnections;
    x->outboundConnections[position] = y;
    x->numOutboundConnections++;
}

int isSameRoom(Room *x, Room *y) { // check name and num of outbounds for equality
    if ((!strcmp(x->name, y->name) &&
         (x->numOutboundConnections == y->numOutboundConnections))) {
        return 1;
    } else {
        return 0;
    }
}


void addRandomConnection(Room *rooms, int nRooms) {
    struct Room *a;
    struct Room *b;
    
    int toggle = 1;
    while(toggle){ // while true
        a = getRandomRoom(rooms, nRooms);
        
        if (a->numOutboundConnections >= 6) { //do nothing
        } // try different room
        else if (canAddConnectionFrom(a) != 0) {
            toggle = 0;
        }
    }
    
    do {
        b = getRandomRoom(rooms, nRooms);
    } while(canAddConnectionFrom(b) == 0 || isSameRoom(a, b) != 0 || connectionAlreadyExists(a, b) != 0);
    
    // connect the forwards and backwards
    connectRoom(a, b);
    connectRoom(b, a);
}

int main(int argc, const char * argv[]) {
    
    // Great Pyramid Rooms
    char *names[255] = {
        "QueensChamber",
        "GrandGallery",
        "KingsChamber",
        "Davidsons",
        "Wellingtons",
        "Nelsons",
        "RobbersTunnel",
        "CampbellsChamber",
        "LadyArburthnot",
        "DescendingPassage"
    };
    
    // 1 if available, 0 if used
    int nameStatus[10] = {1,1,1,1,1,1,1,1,1,1};
    int r;
    
    // init the rand
    srand (time(NULL));
    int n = 7; // num rooms wwanted
    
    struct Room rooms[n];
    
    
    // generate start and end room values
    int rStart = rand() % n;
    int rEnd;
    int toggle = 1;
    while(toggle) {
        int attempt = rand() % n;
        if (attempt != rStart) {
            rEnd = attempt;
            toggle = 0;
        }
    }
    {
        int i;
        for(i=0; i < n; i++) {
            // Get Random name from array, check status,  mark corresponding status element, build string
            int toggle = 1;    // when toggle on, we attempt to select names
            while(toggle) {
                r = rand() % 10;
                if (nameStatus[r] == 1) {
                    nameStatus[r] = 0;
                    rooms[i].name = names[r];
                    toggle = 0;
                }
            }
            // assign start, end, and mid room values
            if (i == rStart) {
                rooms[i].type = 0;
            } else if (i == rEnd) {
                rooms[i].type = 1;
            } else {
                rooms[i].type = 2;
            }
            
            // initialize numOutbound
            rooms[i].numOutboundConnections = 0;
        }
    }
    
    while (!isGraphFull(rooms, n)) {
        addRandomConnection(rooms, n);
    }
    // prefix
    char dirPath[255] = "youndani.rooms.";
    pid_t pid = getpid();
    
    // put pid on prefix
    sprintf(dirPath + strlen(dirPath), "%d", pid);
    
    int result = mkdir(dirPath,0700);
    
    // write output strings
    char path[255];
    char roomNameLine[255] = "ROOM NAME: ";
    char connectionLine[255] = "CONNECTION "; //CONNECTION 1:_
    char roomTypeLine[255] = "ROOM TYPE: ";
    
    memset(path,0,strlen(path));
    sprintf(path + strlen(path), "%s", dirPath);
    int i;
    for (i=0; i < n; i++) {
        
        // Path
        memset(path,0,strlen(path));
        sprintf(path + strlen(path), "%s", dirPath);
        if(rooms[i].type == 0) {
            sprintf(path + strlen(path), "/-%s", rooms[i].name); // > is start room -Roomname
        } else if (rooms[i].type == 1) {
            sprintf(path + strlen(path), "/_%s", rooms[i].name); // _ is end room _Roomname
        }
        else {
            sprintf(path + strlen(path), "/%s", rooms[i].name);
        }
        
        // Create and Open file for read write
        // O_RDWR | O_CREAT, 0600 ( S_IRUSR | S_IWUSR )
        int file_descr = open(path, O_RDWR | O_CREAT, 0600);
        
        if (file_descr < 0) {
            fprintf(stderr, "Couldn't open %s\n", path);
            perror("Error in main()");
            exit(1);
        }
        
        // Room name String
        char roomName[255];
        memset(roomName,0,strlen(roomName));
        sprintf(roomName + strlen(roomName), "%s",roomNameLine);
        sprintf(roomName + strlen(roomName), "%s\n", rooms[i].name);
        
        // Room type string
        char roomType[255];
        memset(roomType,0,strlen(roomType));
        sprintf(roomType + strlen(roomType), "%s", roomTypeLine);
        if (rooms[i].type == 0) {
            sprintf(roomType + strlen(roomType), "%s\n", "START_ROOM");
        } else if (rooms[i].type == 1) {
            sprintf(roomType + strlen(roomType), "%s\n", "END_ROOM");
        } else {
            sprintf(roomType + strlen(roomType), "%s\n", "MID_ROOM");
        }
        
        ssize_t nwritten;
        
        // write room name
        nwritten = write(file_descr, roomName, strlen(roomName) * sizeof(char));
        
        // write connection
        char connectionName[255];
        int j;
        for (j = 0; j < rooms[i].numOutboundConnections; j++) {
            memset(connectionName, 0,strlen(connectionName));
            sprintf(connectionName + strlen(connectionName), "%s",connectionLine);
            sprintf(connectionName + strlen(connectionName), "%d: ", (j+1));
            sprintf(connectionName + strlen(connectionName), "%s\n", rooms[i].outboundConnections[j]->name);
            
            nwritten = write(file_descr, connectionName, strlen(connectionName) * sizeof(char));
        }
        
        // write type
        nwritten = write(file_descr, roomType, strlen(roomType) * sizeof(char));
        close(file_descr);
    }
    
    return 0;
}

