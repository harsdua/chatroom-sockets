//Client-side main file for chatroom
//INFOF202
// By Harsh 509461 and Omar 478023



//some code reused from https://www.geeksforgeeks.org/socket-programming-cc/
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

#include <time.h>

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>
#include <string>
#include <iostream>


void *userThreadFun(void *vargp){
    //function that will be run on another thread that will be responsible for recieving messages
    int sock = *(int*) vargp;

    Message msg;

    while(readMessage(sock,msg)){
        std::cout<<std::endl<<msg.message<<std::endl;
        delete[] msg.message;
    }
    _exit(EXIT_FAILURE);
    return nullptr;
}


int main(int argc, char const *argv[]){
    //get 3 args, nicnkname server ip and port
    if (argc < 4){
        std::cout<<"Please provide 3 arguments"<<std::endl;
        exit(EXIT_FAILURE);
    }

    //init respective variables
    char const* username = argv[1];
    char const* server = argv[2];
    int port = atoi(argv[3]);

    //create socket
    int sock = 0;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }


    serv_addr.sin_family = AF_INET; //ipv4 families
    serv_addr.sin_port = htons(port);//convert to network byte order

    // Convert IP addresses from text to binary form
    if(inet_pton(AF_INET, server, &serv_addr.sin_addr)<=0)
        //error for incompatible address provided
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    //socket connection
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    //init message with username
    Message msg;
    time(&msg.timestamp);
    msg.message = new char[strlen(username)];
    strcpy(msg.message,username);
    msg.size = strlen(msg.message);

    //send server with the username
    sendMessage(sock,msg);

    delete[] msg.message;
    msg.message = nullptr;


    //thread responsible for recieved messages
    pthread_t thread_id;
    pthread_create(&thread_id,NULL,userThreadFun,&sock);

    std::string line;


    //get inputs and send it to server
    while(std::getline(std::cin,line)){
        time(&msg.timestamp);
        msg.message = (char *)realloc(msg.message, line.size());
        strcpy(msg.message,line.c_str());
        msg.size = strlen(msg.message);
        sendMessage(sock,msg);

    }


    return 0;
}
