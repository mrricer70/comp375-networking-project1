/**
 * client.c
 *
 * @author Darius Yeung
 *
 * USD COMP 375: Computer Networks
 * Project 1
 *
 * Project description: This project reverse-engineers the sensor network protocol and then
 * write a client that does indeed adhere to this protocol. The sensor network
 * responds to the clients' request with the current temperature, relative
 * humidity, and wind speed on USD's campus.
 */

#define _XOPEN_SOURCE 600

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define BUFF_SIZE 1024

long prompt();
int connectToHost(char *hostname, char *port);
void mainLoop(int server_fd);

int main() {
	int server_fd = connectToHost("hopper.sandiego.edu", "7030");
		
	printf("WELCOME TO THE COMP375 SENSOR NETWORK\n\n\n");

	mainLoop(server_fd);
	close(server_fd);
	return 0;
}


/**
 * Retrieve the desired weather information (AIR TEMPERATURE, RELATIVE HUMIDITY,
 * OR WIND SPEED) with the current time by sending and receiving messages and
 * by connecting the weather server with the needed information. 
 *
 * @param server_fd Socket file descriptor for communicating with the server
 * @param buff Array buffer to input the sent messages or the received messages
 * @param request Array of characters of the 3 choices (IR TEMPERATURE, RELATIVE HUMIDITY,
 * OR WIND SPEED)
 * 
 */
void getWeatherInfo(int server_fd, char buff[BUFF_SIZE], char *request) {

	char weather_port[4];
	char measurement[BUFF_SIZE];
	char symbol[BUFF_SIZE];
	
	time_t current_time;
	time(&current_time);

	memset(buff, 0, BUFF_SIZE);
	strcpy(buff, "AUTH password123\n");
	send(server_fd, buff, strlen(buff), 0);					
	memset(buff, 0, BUFF_SIZE);

	recv(server_fd, buff, BUFF_SIZE, 0);
	sscanf(buff, "CONNECT weatherstation.sandiego.edu %s sensorpass321\n", (char *)weather_port);
	int weather_server = connectToHost("weatherstation.sandiego.edu", weather_port);
	strcpy(buff, "AUTH sensorpass321\n");
	send(weather_server, buff, strlen(buff), 0);
	memset(buff, 0, BUFF_SIZE);
	recv(weather_server, buff, BUFF_SIZE , 0);

	strcpy(buff, request);
	strcat(buff, "\n");

	send(weather_server, buff, strlen(buff), 0);
	memset(buff, 0, BUFF_SIZE);
	recv(weather_server, buff, BUFF_SIZE, 0);
	sscanf(buff, "%*s %s%s", (char *)measurement, (char *)symbol);
	printf("\nThe last %s reading was %s %s, taken at %s\n\n", request, measurement, symbol, ctime(&current_time));

	memset(buff, 0, BUFF_SIZE);
	strcpy(buff, "CLOSE\n");
	send(weather_server, buff, strlen(buff), 0);
	memset(buff, 0, BUFF_SIZE);
	recv(weather_server, buff, BUFF_SIZE, 0);
	memset(buff, 0, BUFF_SIZE);
	close(weather_server);

}
 

/**
 * Loop to keep asking user what they want to do and calling the appropriate
 * function to handle the selection.
 *
 * @param server_fd Socket file descriptor for communicating with the server
 */
void mainLoop(int server_fd) {
	while (1) {
		long selection = prompt();
		server_fd = connectToHost("hopper.sandiego.edu", "7030");
		char buff[BUFF_SIZE];

		switch (selection) {
			case 1:
				// DONE: Handle case one by calling a function you write

				getWeatherInfo(server_fd, buff, "AIR TEMPERATURE");
				break;

			case 2:
				
				getWeatherInfo(server_fd, buff, "RELATIVE HUMIDITY");
				break;

			case 3:

				getWeatherInfo(server_fd, buff, "WIND SPEED");
				break;

			case 4:

				printf("GOODBYE!\n");
				exit(0);

			default:
			
				fprintf(stderr, "ERROR: Invalid selection\n");
				break;
		}
	}

}

/** 
 * Print command prompt to user and obtain user input.
 *
 * @return The user's desired selection, or -1 if invalid selection.
 */
long prompt() {
	// DONE: add printfs to print out the options
	printf("Which sensor would you like to read:\n\n");
	printf("\t(1) Air temperature\n");
	printf("\t(2) Relative humidity\n");
	printf("\t(3) Wind speed\n");
	printf("\t(4) Quit program\n\n");
	printf("Selection: ");
	

	// Read in a value from standard input
	char input[10];
	memset(input, 0, 10); // set all characters in input to '\0' (i.e. nul)
	char *read_str = fgets(input, 10, stdin);

	// Check if EOF or an error, exiting the program in both cases.
	if (read_str == NULL) {
		if (feof(stdin)) {
			exit(0);
		}
		else if (ferror(stdin)) {
			perror("fgets");
			exit(1);
		}
	}

	// get rid of newline, if there is one
	char *new_line = strchr(input, '\n');
	if (new_line != NULL) new_line[0] = '\0';

	// convert string to a long int
	char *end;
	long selection = strtol(input, &end, 10);

	if (end == input || *end != '\0') {
		selection = -1;
	}

	return selection;
}

/**
 * Socket implementation of connecting to a host at a specific port.
 *
 * @param hostname The name of the host to connect to (e.g. "foo.sandiego.edu")
 * @param port The port number to connect to
 * @return File descriptor of new socket to use.
 */
int connectToHost(char *hostname, char *port) {
	// Step 1: fill in the address info in preparation for setting 
	//   up the socket
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;  // will point to the results

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_INET;       // Use IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	// get ready to connect
	if ((status = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	// Step 2: Make a call to socket
	int fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (fd == -1) {
		perror("socket");
		exit(1);
	}

	// Step 3: connect!
	if (connect(fd, servinfo->ai_addr, servinfo->ai_addrlen) != 0) {
		perror("connect");
		exit(1);
	}

	freeaddrinfo(servinfo); // free's the memory allocated by getaddrinfo

	return fd;
}
