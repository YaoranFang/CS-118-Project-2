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
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string>
#include <cstdlib>


#define BUFLEN 2048
#define NUM_ROUTERS 6


struct RoutingTableEntry {
	int port;
	int cost;
};

struct RoutingTable {
	int port;
	RoutingTableEntry table[NUM_ROUTERS];
} routing_tables[NUM_ROUTERS];

/* index 0-A
 * index 1-B
 * index 2-C
 * index 3-D
 * index 4-E
 * index 5-F
 */

/*******************************************************
						Functions
*******************************************************/


//initialize routing tables
void initializeTables(){
	for (int i = 0; i < NUM_ROUTERS; i++){
		routing_tables[i].port = 10000 + i;
		for (int j = 0; j < NUM_ROUTERS; j++){
			if (i == j)
				routing_tables[i].table[j].cost = 0;
			else
				routing_tables[i].table[j].cost = INT_MAX;
		}
	}
}


//read the initial file and update the routing table accordingly
//specified in "Approach 8."
void readInitialFile(const char* filename){
	initializeTables();
	std::string line;
	std::ifstream file(filename);
	if (file.is_open()){
		while(getline(file, line)){
			char* line_ptr = &line[0];
			char* source_router = strtok(line_ptr, ",");
			if (source_router == NULL){
				file.close();
				return;
			}
			char* dest_router = strtok(NULL, ",");
			if (dest_router == NULL){
				file.close();
				return;
			}
			char* source_port = strtok(NULL, ",");
			if (source_port == NULL){
				file.close();
				return;
			}
			char* link_cost = strtok(NULL, ",");
			if (link_cost == NULL){
				file.close();
				return;
			}

			int src_router = source_router[0] - 'A';
			int dst_router = dest_router[0] - 'A';
			int dst_port = atoi(source_port);
			int cost = atoi(link_cost);

			routing_tables[src_router].table[dst_router].cost = cost;
			routing_tables[src_router].table[dst_router].port = dst_port;
		}
		file.close();
	}
}


//print the content of the ith table.
//specified in "Instruction 5." of the project specification
void printTable (int i){// i is the ith table
	printf("Destination\tCost\tOutgoing UDP Port\tDestination UDP Port\n");
	char my_id = 'A' + i;
	for (int j = 0; j < NUM_ROUTERS; j++){
		int cost = routing_tables[i].table[j].cost;
		if (cost != INT_MAX && i != j){
			char dest_id = 'A' + j;
			int my_port = routing_tables[i].port;
			int dest_port = routing_tables[i].table[j].port;
			printf("%c\t\t%d\t%d (Node %c)\t\t%d (Node %c)\n",
			dest_id, cost, my_port, my_id, dest_port, dest_id);
		}
	}
	printf("\n\n");
}


//save the Routing Table Entries into a file
//specified in "Approach 7." of the project specification
void saveTable (int i){// i is the ith table
	std::string fname = "routing-output";
	fname += ('A' + i);
	fname += ".txt";

	const char* filename = fname.c_str();
	freopen(filename, "a", stdout);
	printTable(i);
	fclose(stdout);
}


//broadcast DV
void broadcast (int myPort, int remPort){//myPort is the source port, remPort is the destination port

	struct sockaddr_in myaddr, remaddr;
	int fd, i;
	int slen=sizeof(remaddr);
	char buf[BUFLEN];	/* message buffer */
	int recvlen;		/* # bytes in acknowledgement message */
	std::string server = "127.0.0.1";	/* localhost */

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
	readInitialFile("sample");
	//initializeTables();
	printTable(0);
	printTable(1);
	printTable(2);
	printTable(3);
	printTable(4);
	printTable(5);
/*
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

*/
	return 0;
}
