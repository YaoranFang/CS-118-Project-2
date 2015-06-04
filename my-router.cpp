//#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <string.h>
//#include <sys/wait.h>

#include <fcntl.h>

#include <unistd.h> //Unix Version
//#include <io.h>    //Windows Version

#include <sys/types.h>

//#include <sys/socket.h>  //Unix Version
//#include <WinSock2.h>  //Windows version
//#pragma comment(lib, "ws2_32.lib")

#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctime>
#include <time.h>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string>
#include <cstdlib>

#include <vector>


#define BUFLEN 2048
#define PATHLENGTH 8
#define NUM_ROUTERS 6


struct RoutingTableEntry {
	int cost;
	int nextPort; //Should know where should I forward the packet to
};

RoutingTableEntry table[NUM_ROUTERS];

int aliveRouters[NUM_ROUTERS] = {0};

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

struct alivestruct {
	int m_port;
	int m_timer;
	int direct_cost;
};

//This is set in the beginning in int main. This is the own ABCDEFGH ID.
char MY_ID;
char DEST_ID;
std::vector<struct alivestruct> NEIGHBORS;



packet string_to_packet(char* buf, int recvlen);
/*******************************************************
			Functions
*******************************************************/


packet string_to_packet(char* buf);


//initialize routing tables
void initializeTable(){
	int index = MY_ID - 'A';
	for (int i = 0; i < NUM_ROUTERS; i++){
		if (index == i){
			table[i].cost = 0;
			table[i].nextPort = index + 10000;
		}

		else{
			table[i].cost = INT_MAX;
			table[i].nextPort = -1;
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
				table[dst_router].cost = cost;
				table[dst_router].nextPort = dst_port;

				struct alivestruct haha;
				haha.m_port = dst_router + 10000;
				haha.m_timer = 0;
				haha.direct_cost = cost;

				NEIGHBORS.push_back(haha);
			}
		}
		file.close();
	}
}


//print the content of the ith table.
//specified in "Instruction 5." of the project specification
void printTable (){
	

	time_t timer;
	struct tm* timeinfo;

	time(&timer);
	timeinfo = localtime(&timer);
	
	printf("%s", asctime(timeinfo));
	printf("Destination\tCost\tOutgoing UDP Port\tDestination UDP Port\n");

	for (int i = 0; i < NUM_ROUTERS; i++){
		int cost = table[i].cost;
		if (cost != INT_MAX && i != int(MY_ID - 'A')){
			char dest_id = 'A' + i;
			int my_port = MY_ID - 'A' + 10000;
			int next_port = table[i].nextPort;
			char next_id = next_port - 10000 + 'A';
			printf("%c\t\t%d\t%d (Node %c)\t\t%d (Node %c)\n", dest_id, cost, my_port, MY_ID, next_port, next_id);
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
	/*FILE* fp;
	fp = freopen(filename, "a", stdout);
	printTable();
	fclose(fp);
	freopen("/dev/tty", "a", stdout);*/
	/*int fd;
	fd = open(filename, O_CREAT | O_APPEND | O_WRONLY);
	dup2(fd, 1);
	printTable();
	//dup2(1, fd);
	close(fd);*/
	std::ofstream file;
	file.open(filename, std::fstream::app);

	time_t timer;
	struct tm* timeinfo;
		time(&timer);
			timeinfo = localtime(&timer);

	file << asctime(timeinfo);

	file << "Destination\tCost\tOutgoing UDP Port\tDestination UDP Port\n";
	for (int i = 0; i < NUM_ROUTERS; i++){
		int cost = table[i].cost;
		if (cost != INT_MAX && i != int(MY_ID - 'A')){
			char dest_id = 'A' + i;
			int my_port = MY_ID - 'A' + 10000;
			int next_port = table[i].nextPort;
			char next_id = next_port - 10000 + 'A';

		
			file << dest_id << "\t\t" << cost << "\t" << my_port << "(Node " << MY_ID << ")\t\t" << next_port << " (Node " << next_id << ")\t\t"  << "\n";
		}
	}

	file <<"\n\n";
	file.close();
}


//gets current DV for this router
//make sure to pass in an array of size NUM_ROUTERS
void getDV (int* DV){
	for (int i = 0; i < NUM_ROUTERS; i++){
		DV[i] = table[i].cost;
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
		printf("socket failed\n");

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
	const char* server_cc = server.c_str();
	if (inet_aton(server_cc, &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	//TODO
	/*begin sending message*/
	//Throw in data for packet
		if(DEST_ID == 'Z'){
			toSend.type = '1';
			toSend.destId = 'Z';
			toSend.destPort = 10000;
			toSend.path_travelled[0] = '\0';
			//getDV(toSend.tableEntry);
			for (int i = 0; i < NUM_ROUTERS; i++){
				toSend.tableEntry[i] = table[i].cost;
			}
		}else{
			toSend.type = '0';
			toSend.destId = DEST_ID;
			toSend.destPort = DEST_ID - 'A' + 10000;
			toSend.path_travelled[0] = MY_ID;
			getDV(toSend.tableEntry);
		}
	


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
//	printf("to send: %s", buf);

	close(fd);

}


void broadcast_all (){
	for (std::vector<alivestruct>::iterator it = NEIGHBORS.begin();
		it != NEIGHBORS.end(); it++){
		
			broadcast(int(MY_ID - 'A' + 10000), it->m_port);
	}
}


//Still needs myPort to open socket
//i is the ith table
void receiveDVAndUpdateTable (int myPort){

	struct sockaddr_in myaddr, remaddr;
	unsigned int addrlen = sizeof(remaddr);
//	char *buf2 = ;
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


	//printf("trying to receive");
	//keep listening until something actually comes
	for (;;) {
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
			return;
		}
		recvlen = recvfrom(fd, buf, BUFLEN, 0, (struct sockaddr*) &remaddr, (socklen_t*)&addrlen);
		if (recvlen > 0) break;   
		if (recvlen < 1){ close(fd); return;}
	}

	for (int i = 0; i < NEIGHBORS.size(); i++){
		if (NEIGHBORS[i].m_port == ntohs(remaddr.sin_port))
		{
			NEIGHBORS[i].m_timer = 0;
			break;
		}
	}

	//Now convert the string back into a usable format

	toReceive = string_to_packet(buf, recvlen); 
	//get package source router
	int package_source_index = ntohs(remaddr.sin_port) - 10000;
	//Notice that toReceive.path_travelled is already updated

	//if sending port is not already a neighbor, but it's in the initial file
	//add it to our neighbor list
	/*bool contains = false;
	for (int i = 0; i < NEIGHBORS.size(); i++){
		if (NEIGHBORS[i].m_port == package_source_index)
			contains = true;
	}
	if (!contains){
		std::string line;
		std::ifstream file("initialization-file.txt");
		if (file.is_open()){
			while(getline(file, line)){
			//tokenize string with ',' delimiter
				char* line_ptr = &line[0];
				char* source_router = strtok(line_ptr, ",");
				if (source_router != NULL && source_router[0] == MY_ID){
					char* dest_router = strtok(NULL, ",");
					char* source_port = strtok(NULL, ",");
					char* link_cost = strtok(NULL, ",");
					if (source_port != NULL
						&& atoi(source_port) == package_source_index + 10000
						&& link_cost != NULL) {

						int cost = atoi(link_cost);

						struct alivestruct haha;
						haha.m_port = package_source_index;
						haha.m_timer = 0;
						haha.direct_cost = cost;
						NEIGHBORS.push_back(haha);

						file.close();
						break;
					}
					
				}
			}
		}
		file.close();
	}
*/
	//if received DV, update table

	if (toReceive.type == '1'){

		//update costs
		for(int i = 0; i < NUM_ROUTERS; i++){

			int current_cost = table[i].cost;
			int cost_from_pkg = toReceive.tableEntry[i];
			int cost_to_pkg = table[package_source_index].cost;
			int pkg_cost;

			char tempbuf[15] = "";

			std::string DV_MSG = "Destination\tA\tB\tC\tD\tE\tF\n";
			DV_MSG += "Cost\t\t";
			int j = 0;
			int count = 0;
			for(int k = 8; k < strlen(buf); k++){
				if (count == 6) break;
  				//whenever there is a space, wrap up tempbuf,
				//and store it into a tableEntry.
				if (buf[k] == ' ') {
					tempbuf[j] = '\0';
					if( atoi(tempbuf) == INT_MAX)
						DV_MSG += '-';
					else
						DV_MSG += tempbuf;
					tempbuf[0] = '\0';
					j = 0;
					count++; 
					DV_MSG += '\t';
					continue; 
				} else {
					tempbuf[j] = buf[k];
					j++;
				}
			}
			DV_MSG += "\n";
			const char* dv_m = DV_MSG.c_str();

			if (cost_from_pkg == INT_MAX || cost_to_pkg == INT_MAX)
				pkg_cost = INT_MAX;
			else
				pkg_cost = cost_from_pkg + cost_to_pkg;

			if ((pkg_cost - current_cost < 0)
			    || (current_cost == pkg_cost
				&& table[i].nextPort > ntohs(remaddr.sin_port))){
			
				table[i].cost = pkg_cost;
				table[i].nextPort = ntohs(remaddr.sin_port);


				printf("%s", dv_m);

				printTable();
				saveTable();
			}
		}

	//if received data packet, ....
	} else {
		int dest_index = toReceive.destId - 'A';
		//data arrive at destination
		if(toReceive.destId == MY_ID){
			printf("Package arrive. Traversed path: %s\n", toReceive.path_travelled);
		}else{
		//forward to data to next node
			if (table[dest_index].nextPort == -1)
				printf("NOT CONNECTED!");
	//now define remaddr, the address to whom we want to send messages
			std::string server = "127.0.0.1";	//localhost
			memset((char *) &remaddr, 0, sizeof(remaddr));
			remaddr.sin_family = AF_INET;
			remaddr.sin_port = htons(table[dest_index].nextPort);
			const char* server_cc = server.c_str();
			if (inet_aton(server_cc, &remaddr.sin_addr)==0) {
				fprintf(stderr, "inet_aton() failed\n");
				exit(1);
			}
			if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, sizeof(remaddr))==-1) {
				perror("sendto");
				exit(1);
			}
		}
		
	}
	
	close(fd);

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
	for (; k < recvlen; k++){
		if (buf[k] == '\0') break;
		toGet.path_travelled[j] = buf[k];
		j++;
	}
	
	//k is pointing at end, add current path.
	toGet.path_travelled[j] = MY_ID;
	toGet.path_travelled[j+1] = '\0';
	buf[k] = MY_ID;
	buf[k+1] = '\0';
	return toGet;
	}


/*******************************************************
						Main
*******************************************************/
int main(int argc, char const *argv[])
{	
  //accept 1 or 2 arguments
  if (argc != 2 && argc != 3 ) {
    printf("Please enter a character form A to G.\n");;
    exit(0);
  }
  
  //first arg is this router ID
  MY_ID = argv[1][0];
  if (MY_ID < 'A' || 'F' < MY_ID){
    printf("Please enter a character from A to F\n");
    exit(0);
  }

  //second arg is destination router
  if (argc == 3){
	DEST_ID = argv[2][0];

	if (MY_ID < 'A' || 'F' < MY_ID){
		printf("Please enter a destination character from A to F\n");
		exit(0);
	}

	for(;;){
		broadcast (10006, int(MY_ID - 'A' + 10000));
		usleep(20000);
	}
	
	

  } else {

	  clock_t t1, t2;
	  float diff = 0;
	  t1 = clock();


  	//give DEST_ID an impossible value if only one arg.
  	DEST_ID = 'Z';
  	readInitialFile("initialization_file.txt");
  	printTable();
	saveTable();
	time_t time1;
	time_t time2;
	time(&time1);
  	while(true){
  		broadcast_all (); 
  		receiveDVAndUpdateTable (int(MY_ID - 'A' + 10000));

		time(&time2);
		if (difftime(time1, time2)){  //1 second
			time(&time1); 
			for (int i = 0; i < NEIGHBORS.size(); i++){
				NEIGHBORS[i].m_timer++;
				if (NEIGHBORS[i].m_timer >= 10){  //NEIGHBORS[i] is DEAD
					table[NEIGHBORS[i].m_port - 10000].cost = INT_MAX;
					for (int j = 0; j < 6; j++){
						if (table[j].nextPort == NEIGHBORS[i].m_port){
							table[j].nextPort = -1;
							table[j].cost = INT_MAX;
						}
					}
					NEIGHBORS.erase(NEIGHBORS.begin() + i);
					printTable();
					saveTable();
				}
			}
		}

  		usleep(20000);
  	}
  }
  return 0;
}
