#include <iostream>
#include <string.h>
//#include <sys/wait.h>

#include <fcntl.h>

#include <unistd.h> //Unix Version
//#include <io.h>    //Windows Version

#include <sys/types.h>

#include <sys/socket.h>  //Unix Version
//#include <WinSock2.h>  //Windows version

//#include <netinet/in.h>
#include <ctime>
#include <time.h>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string>
#include <cstdlib>
#include <arpa/inet.h>


#define BUFLEN 2048
#define PATHLENGTH 8
#define NUM_ROUTERS 6


struct RoutingTableEntry {
	int port;
	int cost;
	int nextPort; //Should know where should I forward the packet to
};

struct RoutingTable {
	int port;
	RoutingTableEntry table[NUM_ROUTERS];
} routing_table;

/* index 0-A
 * index 1-B
 * index 2-C
 * index 3-D
 * index 4-E
 * index 5-F
 */

struct packet {
	char type; 			//control 1 or data 0
	char destId; 			//needs to know where it wants to go
	int destPort;
	int tableEntry[6];	 	//the actual payload
	char path_travelled[PATHLENGTH];//Path travelled for this packet,
					//useful to determine when to stop, etc.
} toSend, toReceive;

//This is set in the beginning in int main. This is the own ABCDEFGH ID.
char MY_ID;
packet string_to_packet(char* buf, int recvlen);
/*******************************************************
			Functions
*******************************************************/


packet string_to_packet(char* buf);


//initialize routing tables
void initializeTable(){
	int index = MY_ID - 'A';
	routing_table.port = 10000 + index;
	for (int i = 0; i < NUM_ROUTERS; i++){
		if (index == i){
			routing_table.table[i].cost = 0;
			routing_table.table[i].nextPort = routing_table.port;
		}

		else{
			routing_table.table[i].cost = INT_MAX;
			routing_table.table[i].nextPort = -1;
		}
	}
}


//read the initial file and update the routing table accordingly
//specified in "Approach 8."
void readInitialFile(const char* filename){
	initializeTable();
	std::string line;
	std::ifstream file(filename);
	if (file.is_open()){
		//read file line by line
		while(getline(file, line)){
			//tokenize string with ',' delimiter
			char* line_ptr = &line[0];
			char* source_router = strtok(line_ptr, ",");
			if (source_router == NULL){
				file.close();
				return;
			}
			if (source_router[0] == MY_ID){
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

				int dst_router = dest_router[0] - 'A';
				int dst_port = atoi(source_port);
				int cost = atoi(link_cost);

				//update routing_tables
				routing_table.table[dst_router].cost = cost;
				routing_table.table[dst_router].port = dst_port;
				routing_table.table[dst_router].nextPort = dst_port;
			}
		}
		file.close();
	}
}


//print the content of the ith table.
//specified in "Instruction 5." of the project specification
void printTable (){
	printf("Destination\tCost\tOutgoing UDP Port\tDestination UDP Port\n");
	for (int i = 0; i < NUM_ROUTERS; i++){
		int cost = routing_table.table[i].cost;
		if (cost != INT_MAX && i != MY_ID - 'A'){
			char dest_id = 'A' + i;
			int my_port = routing_table.port;
			int dest_port = routing_table.table[i].port;
			printf("%c\t\t%d\t%d (Node %c)\t\t%d (Node %c)\n",
			dest_id, cost, my_port, MY_ID, dest_port, dest_id);
		}
	}
	printf("\n\n");
}


//save the Routing Table Entries into a file
//specified in "Approach 7." of the project specification
void saveTable (){
	//build filename
	std::string fname = "routing-output";
	fname += MY_ID;
	fname += ".txt";
	const char* filename = fname.c_str();

	//open file and append updated table
	freopen(filename, "a", stdout);
	printTable();
	fclose(stdout);
}


//gets current DV for this router
//make sure to pass in an array of size NUM_ROUTERS
void getDV (int* DV){
	for (int i = 0; i < NUM_ROUTERS; i++){
		DV[i] = routing_table.table[i].cost;
	}
}


//broadcast DV
//myPort is the source port, remPort is the destination port
void broadcast (int myPort, int remPort){

	struct sockaddr_in myaddr, remaddr;
	int fd, i;
	int slen=sizeof(remaddr);
	char buf[BUFLEN] = "";			//message buffer
	char tempbuf[10] = "";
	int recvlen;				//# bytes in ack message
	std::string server = "127.0.0.1";	//localhost

	//create a socket
	if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("socket created\n");

	//bind it to all local addresses
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(myPort);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return;
	}

	//now define remaddr, the address to whom we want to send messages
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
        sprintf(tempbuf, "%d", toSend.destPort);
	strcat(buf, tempbuf);

	//all the table entries
	for (int i = 0; i < 6; i++){
          sprintf(tempbuf, "%d", toSend.tableEntry[i]);
	  strcat(buf, tempbuf);
	  strcat(buf, " ");
	}

	//path_travelled is already a char*.
	strcat(buf, toSend.path_travelled);
	
	//Actually send (send the thing called "buf")
	//find the places possible to send, and use sendTo to send
	if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, slen)==-1) {
		perror("sendto");
		exit(1);
	}
}


//Still needs myPort to open socket
//i is the ith table
void receiveDVAndUpdateTable (int myPort){

	struct sockaddr_in myaddr, remaddr;
	unsigned int addrlen = sizeof(remaddr);
	char buf[BUFLEN] = "";
	
	int fd, recvlen;

	//create a socket
	if ((fd=socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		printf("socket created\n");

	//bind it to all local addresses
	memset((char *) &myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(myPort);

	if (bind(fd, (struct sockaddr *) &myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return;
	}

	//keep listening until something actually comes
	for (;;) {
		recvlen = recvfrom(fd, buf, BUFLEN, 0, (struct sockaddr*) &remaddr, &addrlen);
		if (recvlen > 0) break;   
	}

	//Now convert the string back into a usable format
	toReceive = string_to_packet(buf, recvlen); 
	//Notice that toReceive.path_travelled is already updated

	//if received DV, update table
	if (toReceive.type){
		//update destination port number
		//int dest_index = toReceive.destId - 'A'; //this should be set when the package is a data
		int package_source_index = ntohs(remaddr) - 10000;
		//routing_table.table[dest_index].port = toReceive.destPort; //this should be set when the package is a data

		//update costs
		for(int i = 0; i < NUM_ROUTERS; i++){

			if((routing_table.table[i].cost>toReceive.tableEntry[i]+routing_table.table[package_source_index].cost)
				|| (routing_table.table[i].cost == toReceive.tableEntry[i]+routing_table.table[package_source_index].cost
				&& routing_table.table[i].nextPort > ntohs(remaddr))){

				routing_table.table[i].cost = toReceive.tableEntry[i]+routing_table.table[package_source_index].cost;
				routing_table.table[i].port = i + 10000;
				routing_table.table[i].nextPort = ntohs(remaddr);

				saveTable ();
				printTable ()
			}


		}

	//if received data packet, ....
	} else {
		int dest_index = toReceive.destId - 'A';
		if(toReceive.destId == MY_ID){//data arrive at destination
			printf("Package arrive. Traversed path: %s\n", path_travelled);
		}else{//forward to data to next node
			remaddr.sin_port = htons(routing_table.table[dest_index].nextPort);
			if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, slen)==-1) {
				perror("sendto");
				exit(1);
			}
		}
		
	}
}


//This function changes the string from buffer into a usable packet by 
packet string_to_packet(char* buf, int recvlen)
{
	packet toGet;
	char tempbuf[15] = "";
	int fd,  count=0, k, j;

	toGet.type = buf[0];
	toGet.destId = buf[1];
	for (k = 2; k < 7; k++)
		tempbuf[k-2] = buf[k];
	tempbuf[5] = '\0';			//just making sure it ends
	
	toGet.destPort = atoi(tempbuf);
	tempbuf[0] = '\0';			//reset
	j = 0;
	for(k = 7; k < recvlen; k++){
		if (count == 6) break;
  		//whenever there is a space, wrap up tempbuf,
		//and store it into a tableEntry.
		if (buf[k] == ' ') {
			tempbuf[j] = '\0';
			toGet.tableEntry[count] = atoi(tempbuf);
			tempbuf[0] = '\0';
			j = 0;
			count++; 
			continue; 
		} else {
			tempbuf[j] = buf[k];
			j++;
		}
	}
	//k is not reset and is still pointing at beginning of strings.
	j = 0;
	for (k; k < recvlen; k++){
		if (buf[k] == '\0') break;
		toGet.path_travelled[j] = buf[k];
		j++;
	}
	
	//k is pointing at end, add current path.
	toGet.path_travelled[j] = MY_ID;
	toGet.path_travelled[j+1] = '\0';
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
  
  MY_ID = argv[1][0];
  if (MY_ID < 'A' || 'G' < MY_ID){
    printf("Please enter a character from A to G\n");
    exit(0);
  }

  readInitialFile("initialization_file.txt");
  printTable();

  int DV[NUM_ROUTERS];
  getDV(DV);
  printf("DV: %d, %d, %d, %d, %d, %d\n\n", DV[0], DV[1], DV[2], DV[3], DV[4], DV[5]);

  return 0;
}
