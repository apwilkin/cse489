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

struct client_struct;
void server(const char* ip_address, const char* port_string, int port_num);
void serverCommandProcessor(const char* ip_address, int port_num, int listener, std::vector<client_struct> c_list);
void client(const char* ip_address, const char* port_string, int port_num);
void clientCommandProcessor(const char* ip_address, int port_num, int listener, std::vector<client_struct>* c_list, string port_string, int* fdmax, fd_set* master);
void author(string command);
void ip(string command, string ip_ad);
void port(string command, int port);
void list(string command, std::vector<client_struct> c_list);

struct client_struct {
	//sort thing, later do std::sort
	int list_id;
	string hostname;
	string ip_addr;
	int port_number;

	bool operator < (const client_struct& cs) const {
        return port_number < cs.port_number;
    }

} ;

bool logged_in = false;
int client_sockfd;


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

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

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
	

	fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    std::vector<client_struct> client_list;

	int buf_length = 256;
    char buf[buf_length];    // buffer for client data
    int nbytes;

    char remoteIP[INET_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(ip_address, port_string, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);
    FD_SET(0, &master);
    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read        
        for(i = 0; i <= fdmax; i++) {
        	if (i == 0) {
        		if (FD_ISSET(0, &read_fds)) {
        			serverCommandProcessor(ip_address, port_num, listener, client_list);
        		}
        	}
            else if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);
                    
                    //setsockopt(newfd, SOL_TCP, TCP_NODELAY, 
                    //my code
					//const char *msg = "Hello Client!";
					int len, bytes_sent;
					/*strcpy(buf, "Hello Client");
					bytes_sent = send(newfd, buf, sizeof(buf), 0);
					std::cout << "first message sent" << std::endl;
					usleep(500);*/
					//my code;
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }

                        int client_fd = newfd;
                        
                        
                        memset(buf, 0, sizeof(buf) - 1);					
						int host_bytes = recv(client_fd, buf, sizeof(buf), 0); //MSG_WAITALL
                        string client_hostname = buf;
                        std::cout << "hostname: " << client_hostname << std::endl;
                        
                        memset(buf, 0, sizeof(buf) - 1);					
						int port_bytes = recv(client_fd, buf, sizeof(buf), 0); //MSG_WAITALL
                        int client_port = atoi(buf);
                        std::cout << "port: " << buf << std::endl;
                        
                        string client_ip = inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET_ADDRSTRLEN);
                        
                        client_struct new_client;
                        
                        new_client.list_id = client_fd;
						new_client.hostname = client_hostname;
						new_client.ip_addr = client_ip;
						new_client.port_number = client_port;
                        	
						client_list.push_back(new_client);
						
						const char* liststart = "LISTSTART";
						memset(buf, 0, sizeof(buf) - 1);
						strcpy(buf, liststart);
						bytes_sent = send(newfd, buf, sizeof(buf), 0);
						usleep(500);
						
						for (int j = 0; j < client_list.size(); j++) {
							std::cout << "Sending Client" << std::endl;
							client_struct current_client = client_list.at(j);
							
							memset(buf, 0, sizeof(buf) - 1);
							sprintf(buf, "%d", current_client.list_id);
							bytes_sent = send(newfd, buf, sizeof(buf), 0);
							std::cout << "First Part: " << buf << std::endl;
							usleep(500);
							
							memset(buf, 0, sizeof(buf) - 1);
							strcpy(buf, current_client.hostname.c_str());
							bytes_sent = send(newfd, buf, sizeof(buf), 0);
							std::cout << "Second Part: " << buf << std::endl;
							usleep(500);
							
							memset(buf, 0, sizeof(buf) - 1);
							strcpy(buf, current_client.ip_addr.c_str());
							bytes_sent = send(newfd, buf, sizeof(buf), 0);
							std::cout << "Third Part: " << buf << std::endl;
							usleep(500);
							
							memset(buf, 0, sizeof(buf) - 1);
							sprintf(buf, "%d", current_client.port_number);
							bytes_sent = send(newfd, buf, sizeof(buf), 0);
							std::cout << "Fourth Part: " << buf << std::endl;
							usleep(500);
							
						}
						const char* listend = "LISTEND";
						memset(buf, 0, sizeof(buf) - 1);
						strcpy(buf, listend);
						bytes_sent = send(newfd, buf, sizeof(buf), 0);
						usleep(500);
                        //send list somehow. maybe for each client_struct iterate through items. Start with LISTSTART, end with LISTEND
                        //send all buffered messages, as well as [EVENT]: Message Received                        
                        //("msg from:%s\n[msg]:%s\n", client-ip, msg)
                        printf("selectserver: new connection from %s on socket %d\n", remoteIP, newfd);
                    }
                } else {
                    // handle data from a client
                    memset(buf, 0, buf_length);
                    if ((nbytes = recv(i, buf, buf_length, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            //remove this item from list
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client
                        
                        
                        /* This is a way to send to all
                        for(j = 0; j <= fdmax; j++) {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1) {
                                        perror("send");
                                    }
                                }
                            }
                        }*/
                        
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
}

	//command processing
void serverCommandProcessor(const char* ip_address, int port_num, int listener, std::vector<client_struct> c_list) {
	string command_str;
	getline(cin, command_str);
	if (command_str != "") {
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
			list(command_str, c_list);
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
			//close(listener);
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
			exit(0); //may need to do something different here
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
	


	// first, load up address structs with getaddrinfo():

	//login/logout code
	
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    std::vector<client_struct> client_list;

	int buf_length = 256;
    char buf[buf_length];    // buffer for client data
    int nbytes;

    char remoteIP[INET_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;


    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(ip_address, port_string, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);
    FD_SET(0, &master);
    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read        
        for(i = 0; i <= fdmax; i++) {
        	if (i == 0) {
        		if (FD_ISSET(0, &read_fds)) {
        			clientCommandProcessor(ip_address, port_num, listener, &client_list, port_string, &fdmax, &master);
        		}
        	}
            else if (FD_ISSET(i, &read_fds) && logged_in) {
				//do stuff. This info should be from the server?
			}
		}
	}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

}
	
void clientCommandProcessor(const char* ip_address, int port_num, int listener, std::vector<client_struct>* c_list, string port_string, int* fdmax, fd_set* master) {
	string command_str;
	getline(cin, command_str);
	if (command_str != "") {
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
			list(command_str, *c_list);
		}
		
		//client
		else if (command_vector[0] == "LOGIN") {
				struct addrinfo hints, *res;
				//int sockfd;
			if (command_vector.size() != 3) {
				cse4589_print_and_log("[%s:ERROR]\n", command_str.c_str());
			}
			else {
				int server_port_num = atoi(command_vector[2].c_str()); //2345
				const char* server_port = command_vector[2].c_str();
				const char* server_ip = command_vector[1].c_str(); //"128.205.36.8"
				struct sockaddr_in test_addrin;
				if (server_port_num > 65535 || server_port_num < 1024) {
					cse4589_print_and_log("[%s:ERROR]\n", command_str.c_str());
				}
				else if (inet_pton(AF_INET, server_ip, &(test_addrin.sin_addr)) == -1) {
					cse4589_print_and_log("[%s:ERROR]\n", command_str.c_str());
				}
				else {
					memset(&hints, 0, sizeof hints);
					hints.ai_family = AF_UNSPEC;
					hints.ai_socktype = SOCK_STREAM;

					getaddrinfo(server_ip, server_port, &hints, &res);

					// make a socket:

					client_sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

					// connect!

					connect(client_sockfd, res->ai_addr, res->ai_addrlen);
					
					FD_SET(client_sockfd, master); // add to master set
                    if (client_sockfd > *fdmax) {    // keep track of the max
                        *fdmax = client_sockfd;
                    }
                    
					cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
					logged_in = true;
					
					int buf_length = 256;
					
					//send hostname
					char hostname[256];
					gethostname(hostname, sizeof(hostname));
					int host_bytes_sent = send(client_sockfd, hostname, strlen(hostname), 0);
					usleep(500);
					
					//send port_num
					int port_bytes_sent = send(client_sockfd, port_string.c_str(), strlen(port_string.c_str()), 0);
					usleep(500);
					
					
					//recv list
					//recv buffer (haha yeah right!)

					
					char buf[buf_length];
					memset(buf, 0, sizeof(buf) - 1);					
					int num_bytes = recv(client_sockfd, buf, sizeof(buf), 0);
					string start_end = buf;
					//if list start set done to false
					//if list end set done to true
					//loop  through add to client list pointer
					if (start_end == "LISTSTART") {
						std::cout << "Reached start of list: " << buf << std::endl;
						memset(buf, 0, sizeof(buf) - 1);					
						recv(client_sockfd, buf, sizeof(buf), 0);
						start_end = buf;
						int j = 0;
						client_struct next_client;
						while (start_end != "LISTEND") {
							std::cout << "Next received buf: " << buf << std::endl;
							if (j == 0) {
								next_client.list_id = atoi(buf);
							}
							else if (j == 1) {
								next_client.hostname = buf;
							}
							else if (j == 2) {
								next_client.ip_addr = buf;
							}
							else if (j == 3) {
								next_client.port_number = atoi(buf);
								c_list->push_back(next_client);
								client_struct empty_struct;
								next_client = empty_struct;
							}
							
							if (j == 3) {j = 0;}
							else { j++;}
							memset(buf, 0, sizeof(buf) - 1);
							recv(client_sockfd, buf, sizeof(buf), 0);
							start_end = buf;
							std::cout << start_end << std::endl;
						}
					}
				}
			}
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		
		//client
		else if (command_str == "REFRESH") {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			//refresh
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		
		//client
		else if (command_str == "SEND") {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			//send
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		
		//client
		else if (command_str == "BROADCAST") {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			//broadcast
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		
		//client
		else if (command_str == "BLOCK") {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			//block
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		
		//client
		else if (command_str == "UNBLOCK") {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			//unblock
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		
		//client
		else if (command_str == "LOGOUT" && logged_in) {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			logged_in = false;
			//erase from list
			close(client_sockfd);
			FD_CLR(client_sockfd, master);
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
		}
		//client (and server?)
		else if (command_str == "EXIT") {
			cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
			cse4589_print_and_log("[%s:END]\n", command_str.c_str());
			exit(0); //may need to do something different here
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

void list(string command_str, std::vector<client_struct> c_list) {
	cse4589_print_and_log("[%s:SUCCESS]\n", command_str.c_str());
	std::sort(c_list.begin(), c_list.end());
	//print list
	for (int j = 0; j < c_list.size(); j++) {
		cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", j + 1, c_list[j].hostname.c_str(), c_list[j].ip_addr.c_str(), c_list[j].port_number);
	}
	cse4589_print_and_log("[%s:END]\n", command_str.c_str());
}







