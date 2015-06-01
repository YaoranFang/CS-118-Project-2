#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctime>
#include <time.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string>

using namespace std;


#define BUFLEN 2048
#define NUM_ROUTERS 6


struct RoutingTableEntry {
	char destId;
	int destPort;
	int cost;
	int nextPort;
} entry;


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
	char buf[BUFLEN];	/* message buffer */
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
}


void receiveDVAndUpdateTable (int i){//i is the ith table

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
  	
  }

   if(argv[1]=="B"){
  	
  }

   if(argv[1]=="C"){
  	
  }

   if(argv[1]=="D"){

  }

   if(argv[1]=="E"){

  }

  if(argv[1]=="F"){

  	
  }

  if(argv[1]=="F"){

  	
  }


	return 0;
}
