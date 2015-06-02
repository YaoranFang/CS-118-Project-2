#include <iostream>
#include <string.h>
//#include <sys/wait.h>

#include <fcntl.h>

//#include <unistd.h> //Unix Version
#include <io.h>    //Windows Version

#include <sys/types.h>

//#include <sys/socket.h>  //Unix Version
#include <WinSock2.h>  //Windows version

//#include <netinet/in.h>
#include <ctime>
#include <time.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string>

using namespace std;


#define BUFLEN 2048
#define PATHLENGTH 8
#define NUM_ROUTERS 6


struct RoutingTableEntry {
	char destId;
	int destPort;
	int cost;
	int nextPort;
} entry;

struct packet {
	char type; //control or data
	char destId;  //needs to know where it wants to go
	int destPort;
	int tableEntry[6];  //the actual payload
	char path_travelled[PATHLENGTH]; //Path travelled for this packet, useful to determine when to stop, etc.

	
} toSend, toReceive;
char MY_ID; //This is set in the beginning in int main. This is the own ABCDEFGH ID.

/*******************************************************
						Functions
*******************************************************/

//initialize Routing table
int routing_tables[NUM_ROUTERS][NUM_ROUTERS] = {INT_MAX};



//read the initial file and update the routing table accordingly
//specified in "Approach 8."
void readInitialFile(){

}


//print the content of the ith table.
//specified in "Instruction 5." of the project specification
void printTable (int i){// i is the ith table
	
}


//save the Routing Table Entries into a file
//specified in "Approach 7." of the project specification
void saveTable (int i){// i is the ith table

}


//broadcast DV
void broadcast (int myPort, int remPort){//myPort is the source port, remPort is the destination port

	struct sockaddr_in myaddr, remaddr;
	int fd, i;
	int slen=sizeof(remaddr);
	char buf[BUFLEN] = "";	/* message buffer */
	char tempbuf[10]="";
	int recvlen;		/* # bytes in acknowledgement message */
	string server = "127.0.0.1";	/* localhost */

	/* create a socket */

	if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("socket created\n");

	/* bind it to all local addresses */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(myPort);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return;
	}

	/* now define remaddr, the address to whom we want to send messages */
	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(remPort);
	/*if (inet_aton(server, &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}*/

	//TODO
	/*begin sending message*/
	//Throw in data for packet


	//Change struct packet format into string (bytes)
	//The String will take the format: "%c%c%d5%d %d %d %d %d %d %c%c%c%c%c%c%c%c", type, destId, destPort, tableEntry[0]...[5], path_travelled[0]...[7]
	strcpy(buf, &toSend.type);
	strcat(buf, &toSend.destId);
	strcat(buf, itoa(toSend.destPort, tempbuf, 10));
	for (int i = 0; i<6; i++)    //all the table entries
	{ 
	strcat(buf, itoa(toSend.tableEntry[i], tempbuf, 10));
	strcat(buf, " ");
	}
	strcat(buf, toSend.path_travelled); //path_travelled is already a char*.
	
	//Actually send (send the thing called "buf")
	//find the places possible to send, and use sendTo to send

}

//Still needs myPort to open socket
void receiveDVAndUpdateTable (int myPort, int i){//i is the ith table
	struct sockaddr_in myaddr, remaddr;
	int addrlen = sizeof(remaddr);
	char buf[BUFLEN]="";
	
	int fd, recvlen;

	if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("socket created\n");

	
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(myPort);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return;
	}

	for(;;)  //keep receiving until something actually comes
	{
	recvlen = recvfrom(fd, buf, BUFLEN, 0, (struct sockaddr*)&remaddr, &addrlen);
	if(recvlen>0) break;   
	}
	//Now convert the string back into a usable format
	toReceive = string_to_packet(buf); //Notice that toReceive.path_travelled is already updated

	//do stuff with it
}


//This function changes the string from buffer into a usable packet by 
packet string_to_packet(char* buf)
{
	packet toGet;
	char tempbuf[15]="";
	int fd, recvlen, count, k, j;

	toGet.type = buf[0];
	toGet.destId = buf[1];
	for(k = 2; k<7; k++)
		tempbuf[k-2]=buf[k];
	tempbuf[5]='\0'; //just making sure it ends

	toGet.destPort = atoi(tempbuf);
	tempbuf[0]='\0';//reset
	j=0;
	for(k = 6; k<recvlen; k++)
	{
		if(count==6) break;
		if(buf[k]==' ') {  //whenever there is a space, wrap up tempbuf, and store it into a tableEntry.
			tempbuf[j]='\0';
			toGet.tableEntry[count]=atoi(tempbuf);
			tempbuf[0]='\0';
			j=0;
			count++; 
			continue; 
		}
		else
		{
			tempbuf[j]=buf[k];
			j++;
		}
	}
	//k is not reset and is still pointing at beginning of strings.
	j=0;
	for (k; k<recvlen; k++)
	{
		if(buf[k]=='\0') break;
		toGet.path_travelled[j]=buf[k];
	}
	//k is pointing at end, add current path.
	toGet.path_travelled[k] = MY_ID;
	toGet.path_travelled[k+1] = '\0';

	return toGet;
}


/*******************************************************
						Main
*******************************************************/
int main(int argc, char const *argv[])
{	

  if (argc != 2 ) {
    printf("Please enter a character form A to G.\n");;
    exit(0);
  }
  
  	

  if(argv[1]=="A"){
  	MY_ID = 'A';

  }

   if(argv[1]=="B"){
	   MY_ID = 'B';
  	
  }

   if(argv[1]=="C"){
	   MY_ID = 'C';
  	
  }

   if(argv[1]=="D"){
	   MY_ID = 'D';

  }

   if(argv[1]=="E"){
	   MY_ID = 'E';

  }

  if(argv[1]=="F"){
	  MY_ID = 'F';

  	
  }

  if(argv[1]=="G"){
	  MY_ID = 'G';

  	
  }


	return 0;
}
