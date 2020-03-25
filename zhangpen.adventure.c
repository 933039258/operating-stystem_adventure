/* CS 344 - Fall 2019
 * Author: Peng Zhang
 * Program 2 - Adventure
 * Due Date: 11/3/2019
 * This file is a C program that let the user play game*/
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
char *roomsName[10] = {
    "Living",
    "Bedroom",
    "Bathroom",
    "Meeting",
    "Guest",
    "Fitness",
    "Waiting",
    "Utility",
    "Dining",
    "Study"};
//defind the room type, we have 1 start, 1 end and others are mid.
char *roomsType[3] = {
    "START_ROOM",
    "MID_ROOM",
    "END_ROOM"};

//build a struct of rooms to define each room.
struct Rooms
{
    char *name;
    char *type;
    int connects;                 //the number of rooms it connected.
    struct Rooms *connect_pos[6]; //each room can maximum connect 6 rooms
};
struct Rooms sele_rooms[7]; //define the global array for the rooms file that selected from 10 rooms, I added * casue a segmentation fault.
//get the directory name
//the code is provided on Canvas, 2.4 Manipulating Directories
//cited from https://oregonstate.instructure.com/courses/1738955/pages/2-dot-4-manipulating-directories
char *GetdrName()
{
    int newestDirTime = -1;                       // Modified timestamp of newest subdir examined
    char targetDirPrefix[32] = "zhangpen.rooms."; // Prefix we're looking for
    char *newestDirName = (char *)malloc(256);
    //char newestDirName[256]; // Holds the name of the newest dir that contains prefix
    memset(newestDirName, '\0', sizeof(newestDirName));

    DIR *dirToCheck;           // Holds the directory we're starting in
    struct dirent *fileInDir;  // Holds the current subdir of the starting dir
    struct stat dirAttributes; // Holds information we've gained about subdir
    dirToCheck = opendir("."); // Open up the directory this program was run in

    if (dirToCheck > 0) // Make sure the current directory could be opened
    {
        while ((fileInDir = readdir(dirToCheck)) != NULL) //   Check each entry in dir
        {
            if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
            {
                stat(fileInDir->d_name, &dirAttributes);         // Get attributes of the entry
                if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
                {
                    newestDirTime = (int)dirAttributes.st_mtime;
                    memset(newestDirName, '\0', sizeof(newestDirName));
                    strcpy(newestDirName, fileInDir->d_name);
                }
            }
        }
    }
    closedir(dirToCheck); // Close the directory we opened
    //free(newestDirName);
    return newestDirName;
}


int step;                                           // record the steps
int f_getTime = 0;                                  //0 display, 1 not display -1 exit/
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER; //declare mutex

//declare a step to do a recording
typedef struct myStep
{
    struct Rooms *cur;   //Define current room
    struct myStep *next; //define next room
} myStep;

struct myStep mystep; //declare a step type of struct that store user's step an path

void readRooms(char *dirNmae);   //read rooms from the file
struct Rooms *Find(char name[]); //Find the room match its name
char *FindType(char type[]);     //determine the room type
int isOk();                      //make sure all the initialize and game is ready
void startGame();                //Play game function
void *showTime();                //show the current time

int main()
{
    //initial the rooms.
    int i, j;
    char Temp_Room[256]; //use for storing the direc name
    struct myStep *cur; //current step
    mystep.cur = NULL;
    mystep.next = NULL;
    for (i = 0; i < 7; i++) //initialize the room type and connect numbers
    {
        sele_rooms[i].connects = 0;
        if (i != 0 && i != 6)
        { //define Mid room
            sele_rooms[i].type = roomsType[1];
        }
    }
    //define start and end room
    sele_rooms[0].type = roomsType[0];
    sele_rooms[6].type = roomsType[2];

    memset(Temp_Room, '\0', sizeof(Temp_Room)); //initial Temp_Room
    strcpy(Temp_Room, GetdrName());             //copy the dir name from that function

    
    readRooms(Temp_Room); //read room from the directory
    if (isOk() == 1)      //check if all the variable or files are legal
    {
        startGame(); //Start the game
        //Game over and show steps and path
        printf("\n\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n\nYOU TOOK %d STEPS.   ", step);//ouput a board if reach the end room
        cur = &mystep;
        printf("YOUR PATH TO VICTORY WAS：\n");
        while (cur) //check if the current room is empty
        {
            printf("%s\n", cur->cur->name); //print the current room name
            cur = cur->next;//check for next room
        }
        
        cur = mystep.next; //read from next room
        
    }
    else
    {
        printf("Error, maybe something wrong with files/variables.\n");
    }
    return 0;
}

//This function is to read rooms from a directory
void readRooms(char *dirNmae)
{
    char fileName[64]; //get the file name
    int i, j, k;
    FILE *fp; //declare a file
    char con[16]; //variable that read connection
    char ty[16];  //variable that read type
    char g[8];    //read the symbol like space and :
    char *p, *q; 
    char fileBuf[128];
    int fileLen;
    struct Rooms *temp;
    //check if the room here
    for (i = 0, j = 0; i < 10; i++)
    {
        sprintf(fileName, "%s/%s", dirNmae, roomsName[i]); 
        
        fp = fopen(fileName, "r"); //read mode for our file
        if (fp != NULL)
        {                                       
            sele_rooms[j++].name = roomsName[i]; //store into the sele_rooms
            fclose(fp);
        }
    }
    //read files
    sprintf(con, "%s", "CONNECTION");
    sprintf(ty, "%s", "ROOM TYPE");
    sprintf(g, "%s", ":");
    for (i = 0; i < 7; i++)
    { 
        sprintf(fileName, "%s/%s", dirNmae, sele_rooms[i].name);
        sele_rooms[i].connects = 0; 
        
        fp = fopen(fileName, "r");
        if (fp)
        {
            //read line by line
            while (fgets(fileBuf, 128, fp) != NULL)
            {
                fileLen = strlen(fileBuf); //get the length of line
                //check if it have connection
                if (strstr(fileBuf, con)) 
                {
                    p = strtok(fileBuf, g); //check the symbol :
                    p = strtok(NULL, g);
                   
                    while (p < (fileBuf + fileLen) && *p == ' ') //ignore spaces
                    {
                        p++;
                    }
                    p = strtok(p, "\n"); 

                    if (sele_rooms[i].connects < 6)
                    {
                        temp = Find(p); 
                        if (temp != NULL)
                        { 
                            sele_rooms[i].connect_pos[sele_rooms[i].connects++] = temp;
                        }
                    }
                }
                else if (strstr(fileBuf, ty)) //check the room type
                {
                    p = strtok(fileBuf, g); //check the symbol :
                    p = strtok(NULL, g);
                    
                    while (p < (fileBuf + fileLen) && *p == ' ') //ignore spaces
                    {
                        p++;
                    }
                    p = strtok(p, "\n");              
                    sele_rooms[i].type = FindType(p); //store the type into the sele.rooms
                }
            }
            fclose(fp);
        }
    }
}

//find the room by its name
struct Rooms *Find(char name[])
{
    int i;
    
    for (i = 0; i < 7; i++)
    {
        if (sele_rooms[i].name == NULL)
        {
            break;
        }
        //check the name
        if (strcmp(sele_rooms[i].name, name) == 0)
        {
            return &sele_rooms[i];
        }
    }
    return NULL;
}

//find the type
char *FindType(char type[])
{
    int i = 0;
    if (type == NULL)
    {
        return NULL;
    }
    for (i = 0; i < 3; i++)
    {
        //campare if the type is right
        if (strcmp(type, roomsType[i]) == 0)
        {
            return roomsType[i];
        }
    }
    return NULL;
}

//This function checks if all variables are legal, and returns 1 legally, otherwise returns 0.
int isOk()
{
    int i, j, f = 1;
    //check the room，
    for (i = 0; i < 7; i++)
    {
        //check the room name is legal
        if (sele_rooms[i].name == NULL || sele_rooms[i].type == NULL)
        {
            f = 0;
        }
        //check the room connection is legal
        if (sele_rooms[i].connects == 0 || sele_rooms[i].connects > 6)
        {
            f = 0;
        }
        else
        {
            //check the pointer of connects is legal
            for (j = 0; j < sele_rooms[i].connects; j++)
            {
                //check the pointer of connect position is legal
                if (sele_rooms[i].connect_pos[j] == NULL)
                {
                    f = 0;
                    break;
                }
            }
        }
        
        if (f == 0)
        {
            break;
        }
    }
    return f;
}

//This function will run the whole game.
void startGame()
{
    int i, j;
    struct Rooms *ptr;
    struct myStep *cur, *temp;
    char input[20]; 
    pthread_t thread1;
    step = 0;
    //find the start room
    for (i = 0; i < 7; i++)
    {
        if (strcmp(sele_rooms[i].type, roomsType[0]) == 0)
        {
            ptr = &sele_rooms[i];
            break;
        }
    }
    if (ptr == NULL)
    {
        return;
    }
    mystep.cur = ptr; //record the current room
    mystep.next = NULL;
    cur = &mystep;
    //creat thread for showing time
    if (pthread_create(&thread1, NULL, &showTime, NULL) != 0)
    {
        printf("Error, cannot create pthread\n");
    }
    while (strcmp(ptr->type, roomsType[2]) != 0) //check the room is reach end room
    {
        
        printf("\n\nCURRENT LOCATION: %s\n", ptr->name);
        printf("POSSIBLE CONNECTIONS: ");
        for (i = 0; i < ptr->connects; i++)
        {
            printf("%s ", ptr->connect_pos[i]->name);
        }
        printf("\n");
        printf("WHERE TO? >");
        while (scanf("%20s", input) == -1)
        {
            printf("error!\n");
            printf("WHERE TO? >");
        }
        //check if the user input the time
        if (strcmp(input, "time") == 0)
        {
            //get rhe current time
           
            pthread_mutex_lock(&mutex1);
            f_getTime = 1;
            pthread_mutex_unlock(&mutex1);
            usleep(2000); 
        }
        else
        {
            //check the room name if it is legal
            for (i = 0; i < ptr->connects; i++)
            {
                if (strcmp(ptr->connect_pos[i]->name, input) == 0)
                {
                    break;
                }
            }
            if (i == ptr->connects)
            {
                printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
            }
            else
            {
                step++;                    
                ptr = ptr->connect_pos[i]; //go to the next room
                //record the paths
                temp = NULL;
                while (temp == NULL)
                {
                    temp = (myStep *)malloc(sizeof(myStep));
                }
               
                temp->cur = ptr;
                temp->next = NULL;
                cur->next = temp;
                cur = temp;
            }
        }
    }
    //exit
    pthread_mutex_lock(&mutex1);
    f_getTime = -1;
    pthread_mutex_unlock(&mutex1);
    pthread_join(thread1, NULL); 
}
//function that show the cuurent time
void *showTime()
{
    FILE *t_file;
    time_t ti;
    struct tm *curtime;
    char strTime[20];
    int test;
    
    while (1)
    {
        //use pthread to allow second thread and mutex
        pthread_mutex_lock(&mutex1);
        test = f_getTime;
        pthread_mutex_unlock(&mutex1);
        if (test == 1)
        { 
            ti = time(NULL);
            curtime = localtime(&ti);
            strftime(strTime, 20, "%F %T", curtime);
            printf("%s\n", strTime);
            t_file = fopen("currentTime.txt","w");
            fprintf(t_file, "%s",strTime);
            fclose(t_file);
            pthread_mutex_lock(&mutex1);
            f_getTime = 0;
            pthread_mutex_unlock(&mutex1);
            
        }
       
        if (test == -1)
        {
            break;
        }
        usleep(1000); 
    }
}