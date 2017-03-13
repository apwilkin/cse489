/**
 * @apwilkin_assignment1
 * @author  Andrew Wilkinson <apwilkin@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <iostream>
#include <stdio.h>
#include <string> //addition made by me
#include <string.h>
#include <stdlib.h> //addition made by me
#include <sstream> //addition made by me
#include <algorithm> //addition made by me
#include <iterator> //addition made by me
#include <vector> // addition made by me
#include <sys/types.h> // addition made by me
#include <sys/socket.h> // addition made by me
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "../include/global.h"
#include "../include/logger.h"

using namespace std;

void server(const char* ip_address, const char* port_string, int port_num);
void client(const char* ip_address, const char* port_string, int port_num);
void author(string command);
void ip(string command, string ip_ad);
void port(string command, int port);
void list(string command);


/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
 
 
int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/* Clear LOGFILE*/
    fclose(fopen(LOGFILE, "w"));

	/*Start Here*/
	
	int port_num;
	const char* ip_address;
	struct sockaddr_in sock_addr;
	
	//Check to see if arguments are valid and store
	if (argc != 3) {
		std::cout << "wrong number of arguments" << std::endl;
		return 0;
	}
	
	//check to see if port number is valid
	port_num = atoi(argv[2]);
	if (port_num > 65535 || port_num < 1024) {
		std::cout << "port error" << std::endl;
		return 0;
	}
	else {
		//get ip address
		char hostname[256];
		gethostname(hostname, sizeof(hostname));
		struct hostent* host = gethostbyname(hostname);
		struct in_addr ip = *(struct in_addr*)host->h_addr_list[0];
		ip_address = inet_ntoa(ip);
		
		//set up client/server
		const char* port_string = argv[2];
		string type = argv[1];
		if (type == "c") {
			std::cout << "client" << std::endl;
			client(ip_address, port_string, port_num);
		}
		else if (type == "s") {
			std::cout << "server" << std::endl;
			server(ip_address, port_string, port_num);		
		}
		else {
			std::cout << "client server error" << std::endl;
			return 0;
		}
	}
	return 0;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void server(const char* ip_address, const char* port_string, int port_num) {
	/*
	socket:
	bind:
	listen:
	accept:
	read/write:
	close:
	*/
	//BEEJ
	
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	struct addrinfo hints, *res;
	int sockfd, new_fd;

	// first, load up address structs with getaddrinfo():

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	//hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	getaddrinfo(ip_address, port_string, &hints, &res); // instead of NULL do it manually

	// make a socket:

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	// bind it to the port we passed in to getaddrinfo():

	bind(sockfd, res->ai_addr, res->ai_addrlen);
	//BEEJ
	listen(sockfd, 5);
	
	addr_size = sizeof their_addr;
	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
	
	const char *msg = "Hello Client!";
	int len, bytes_sent;
	bytes_sent = send(new_fd, msg, strlen(msg), 0);
	
	close(new_fd);
	close(sockfd);
	
	
	//command processing
	string command_str;
	while (command_str != "EXIT") {
		getline(cin, command_str);
		/*split string into vector
		http://stackoverflow.com/questions/236129/split-a-string-in-c */
		istringstream iss(command_str);
		vector<string> command_vector;
		copy(istream_iterator<string>(iss), istream_iterator<string>(),	back_inserter(command_vector));
		
		// server or client
		if (command_str == "AUTHOR") {
			author(command_str);
		}
		
		//server or client
		else if (command_str == "IP") {
			ip(command_str, ip_address);
		}
		
		// server or client
		else if (command_str == "PORT") {
			port(command_str, port_num);
		}
		
		// server or client
		else if (command_str == "LIST") {
			list(command_str);
		}
		
		// server
		else if (command_str == "STATISTICS") {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
	
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		
		// server
		else if (command_vector[0] == "BLOCKED") {
			if (command_vector.size() != 3) {
				//if there is error is the entire input command printed, or just the first word
				cse4589_print_and_log("[%s:ERROR]\n", command_str.c_str());
			}
			else {
				cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
				string client_ip = command_vector[1];
				//handle invalid or incorrect/nonexistent IP address
				//block
			}
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		//client (and server?)
		else if (command_str == "EXIT") {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
			break; //may need to do something different here
		}
		
		else {
			cse4589_print_and_log("[%s:ERROR]\n", command_str.c_str());
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
	}
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void client(const char* ip_address, const char* port_string, int port_num) {
	/*
	socket:
	connect:
	write/read:
	close:
	*/
	
	struct addrinfo hints, *res;
	int sockfd;

	// first, load up address structs with getaddrinfo():

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	const char* test_ip = "128.205.36.8";
	getaddrinfo(test_ip, "2345", &hints, &res);

	// make a socket:

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	// connect!

	connect(sockfd, res->ai_addr, res->ai_addrlen);
	
	char buf[100];
	recv(sockfd, buf, 99, 0);
	std::cout << "Message from server: " << buf << std::endl;
	
	close(sockfd);
	
	string command_str;
	while (command_str != "EXIT") {
		getline(cin, command_str);
		/*split string into vector
		http://stackoverflow.com/questions/236129/split-a-string-in-c */
		istringstream iss(command_str);
		vector<string> command_vector;
		copy(istream_iterator<string>(iss), istream_iterator<string>(),	back_inserter(command_vector));
		
		// server or client
		if (command_str == "AUTHOR") {
			author(command_str);
		}
		
		//server or client
		else if (command_str == "IP") {
			ip(command_str, ip_address);
		}
		
		// server or client
		else if (command_str == "PORT") {
			port(command_str, port_num);
		}
		
		// server or client
		else if (command_str == "LIST") {
			list(command_str);
		}
		
		//client
		else if (command_vector[0] == "LOGIN" && client) {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			//login
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		
		//client
		else if (command_str == "REFRESH" && client) {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			//refresh
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		
		//client
		else if (command_str == "SEND" && client) {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			//send
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		
		//client
		else if (command_str == "BROADCAST" && client) {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			//broadcast
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		
		//client
		else if (command_str == "BLOCK" && client) {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			//block
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		
		//client
		else if (command_str == "UNBLOCK" && client) {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			//unblock
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		
		//client
		else if (command_str == "LOGOUT" && client) {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			//logout
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		//client (and server?)
		else if (command_str == "EXIT") {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
			break; //may need to do something different here
		}
		
		else {
			cse4589_print_and_log("[%s:ERROR]\n", command_str.c_str());
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
	}
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void author(string command_str) {
	cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
	string your_ubit_name = "apwilkin";
	cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", your_ubit_name.c_str());
	cse4589_print_and_log("[%s:END]\n", command_str.c_str());
}

void ip(string command_str, string ip_address) {
	cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
	cse4589_print_and_log("IP:%s\n", ip_address.c_str());
	cse4589_print_and_log("[%s:END]\n", command_str.c_str());
	
}

void port(string command_str, int port) {
	cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
	cse4589_print_and_log("PORT:%d\n", port);
	cse4589_print_and_log("[%s:END]\n", command_str.c_str());
}

void list(string command_str) {
	cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
	//print list
	//cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_id, hostname, ip_addr, port_num)
	cse4589_print_and_log("[%s:END]\n", command_str.c_str());
}







