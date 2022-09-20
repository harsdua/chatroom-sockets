//server-side main file for chatroom
//INFOF202
// By Harsh 509461 and Omar 478023

// Boilerplate borrowed from
// https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/

#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <signal.h>

#define TRUE 1
#define FALSE 0
#define MAX_USERS 1000

#include <iostream>
#include "utils.h"

int master_socket;
int client_socket[MAX_USERS];
bool volatile keepRunning = true;


void signal_handler(int sig){
    //use of a signal handler to close connections
    //close client connections
    for(int i=0;i<MAX_USERS;i++){
        if (client_socket[i] == 0) continue;
        close(client_socket[i]);
    }
    //close host connection
    close(master_socket);
    keepRunning = false;
}

int main(int argc , char *argv[]){
    if (argc < 2){
        std::cout<<"Please provide the port"<<std::endl;
        exit(EXIT_FAILURE);
    }
    signal(SIGINT, signal_handler);

    int port = atoi(argv[1]);

    int opt = TRUE;
    int  addrlen , new_socket , activity, i , sd;
    std::string usernames[MAX_USERS];

    int max_sd;
    struct sockaddr_in address;

    //set of socket descriptors
    fd_set readfds;


    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < MAX_USERS; i++)
    {
        client_socket[i] = 0;
    }

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );

    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", port);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while(keepRunning)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for ( i = 0 ; i < MAX_USERS ; i++)
        {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if (!keepRunning) return 0;
            if ((new_socket = accept(master_socket,
                                     (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //add new socket to array of sockets
            for (i = 0; i < MAX_USERS; i++){
                //if position is empty
                if( client_socket[i] == 0 ){
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n" , i);
                    break;
                }
            }
        }

        //else it's some IO operation on some other socket
        for (i = 0; i < MAX_USERS; i++){
            sd = client_socket[i];
            if(sd == 0) continue;

            //if data recieved
            if (FD_ISSET( sd , &readfds)){
                Message msg;


                //launch read message since data is probably a message recieved
                if (readMessage(sd,msg)){

                    //if username is not initialize
                    if (usernames[i] == "") {
                        //first message recieved is the message with only the username
                        //initialize client's username as the message recieved
                        usernames[i] = msg.message;
                        continue;
                    }

                    //display time on server side 

                    char buff[5];
                    strftime(buff, 20, "%H:%M", localtime(&msg.timestamp));
                    std::cout<<"Time: "<<buff<<std::endl;

                    //link username with message into newMsg
                    std::string newMsg = usernames[i] + " ("+buff+"): "+msg.message;

                    delete[] msg.message;
                    
                    //reassign newMsg into message
                    msg.message = new char[newMsg.size()+1];
                    strcpy(msg.message,newMsg.c_str());
                    msg.size = newMsg.length();
                    
                    //send message to other users    
                    for(int j=0;j<MAX_USERS;j++){
                        if (client_socket[j] == 0 || i == j) continue;
                        sendMessage(client_socket[j],msg);
                    }


                    delete[] msg.message;
                }

                //Somebody disconnected , get his details and print
                else{
                    
                    getpeername(sd , (struct sockaddr*)&address , \
						(socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" ,
                           inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                    usernames[i] = "";
                }
            }
        }
    }

    return 0;
}
