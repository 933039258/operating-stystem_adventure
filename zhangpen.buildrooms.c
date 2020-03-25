/* CS 344 - Fall 2019
 * Author: Peng Zhang
 * Program 2 - Adventure
 * Due Date: 11/3/2019
 * This file is a C program that create rooms for us
 *
 * Some function name, variable name and part of code that cited from 2.2 program outlining in program 2 which is provided on Canvas 
 * Citation: https://oregonstate.instructure.com/courses/1738955/pages/2-dot-2-program-outlining-in-program-2*/

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>

//define what rooms we have. Need 10, and random select 7 from these.
char* roomsName[10] = {
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
char* roomsType[3] = {
   "START_ROOM",
   "MID_ROOM",
   "END_ROOM"
};

//build a struct of rooms to define each room.
struct Rooms{
   char* name;
   char* type;
   int connects;//the number of rooms it connected.
   struct Rooms* connect_pos[6];//each room can maximum connect 6 rooms
};
struct Rooms sele_rooms[7];//define the global array for the rooms file that selected from 10 rooms, I added * casue a segmentation fault.

//return 1 if all rooms have 3 to 6 outbound connections, 0 otherwise
//function name cited from 2.2 Program Outlining in Program 2
int IsGraphFull(){
   int i;
   for(i=0;i<7;i++){//check if the connections in seleted rooms is >=3 and <=6
      if(sele_rooms[i].connects < 3){return 0;}
   }
   return 1;
}
//connects rooms x and y together, does not check if this connection valid.
//function name cited from 2.2 Program Outlining in Program 2
void ConnectRoom(struct Rooms* x, struct Rooms* y){
   x->connect_pos[x->connects] = y;//connect y to x and add 1 to number of connection
   x->connects++;
   y->connect_pos[y->connects] = x;//connection is mutual, allow to go back.
   y->connects++;
}
//return a random room, does not valid if connection can be added
//function name cited 2.2 Program Outlining in Program 2
struct Rooms* GetRandomRoom(){
   int num = rand()%7;
   return &sele_rooms[num];
}

//adds a random, valid outbound connection from a room to another room
//function/variable name and part of code cited from 2.2 Program Outlining in Program 2
void AddRandomConnection(){
   struct Rooms* A;
   struct Rooms* B;
   while(1){
      A = GetRandomRoom();
      if(CanAddConnectionFrom(A) == 1){ break; }
   }
   do{B = GetRandomRoom();}
   while(CanAddConnectionFrom(B) == 0 || IsSameRoom(A, B) == 1 || ConnectionAlreadyExists(A, B) == 1);
   ConnectRoom(A,B);
}


// return 1 if a connection can be added from room x, 0 otherwise
// function name cited from 2.2 Program Outlining in Program 2
int CanAddConnectionFrom(struct Rooms* x){
   if(x->connects < 6){//if the number of connection less than 6, it can connect.
      return 1;
   }
   return 0;
}

//return 1 if a connection from room x to room y already exists, 0 otherwise
//function name cited from 2.2 Program Outlining in Program 2
int ConnectionAlreadyExists(struct Rooms* x, struct Rooms* y){
   int i;
   for (i=0; i<6;i++){ //check 6 times becasue maxinum 6 rooms can connect
      if(x->connect_pos[i] == y){ //check every room connect with x if equal to y
	 return 1;
      }
   }
   return 0;
}


//return 1 if rooms x and y are the same room, 0 otherwise
//function name cited from 2.2 Program Outlining in Program 2
int IsSameRoom(struct Rooms* x, struct Rooms* y){
   if(x == y){
      return 1;
   }
   return 0;

}

int main(){
   //random using seed
   srand(time(0));
   int i;
   //initial the rooms.
   for(i = 0; i < 7; i++) {
      sele_rooms[i].connects = 0;       
      int lop = 1;
      int n;
      int k;
      while(lop){
	 n = rand() % 10; 
	 lop = 0;	 
	 for (k = 0; k < 7; k++){//check if the rooms is repeat
	    if(sele_rooms[k].name == roomsName[n]){
	       lop =1;
	    }
	 }
      }
      sele_rooms[i].name = roomsName[n];     
      if(i!=0 && i!=6){ //define Mid room
	 sele_rooms[i].type = roomsType[1];
      }
   }
   //defin start and end room
   sele_rooms[0].type = roomsType[0];
   sele_rooms[6].type = roomsType[2];
   char folder[256];//creat a folder name
   int pid = getpid();//get the process id for directionary
   sprintf(folder, "zhangpen.rooms.%d", pid); //complete folder name
   int dic = mkdir(folder, 0755); //file permission for creating
   while (IsGraphFull() == 0){AddRandomConnection();}//make connection(cited from 2.2 Program Outlining in Program 2)
   
   //write file and output
   chdir(folder);
   int z;
   for (z=0;z<7;z++){
      FILE* file = fopen(sele_rooms[z].name, "a");
      fprintf(file, "ROOM NAME: %s\n", sele_rooms[z].name);

      int j;
      for(j=0; j<sele_rooms[z].connects;j++){
	 fprintf(file, "CONNECTION %d: %s\n", j+1, (sele_rooms[z].connect_pos[j])->name);
      }
      fprintf(file, "ROOM TYPE: %s\n", sele_rooms[z].type);
      fclose(file);
   }

   return 0;
}
