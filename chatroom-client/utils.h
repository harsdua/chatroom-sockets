//client-side utils header for chatroom
//INFOF202
// By Harsh 509461 and Omar 478023
#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <unistd.h>

struct Message{
    size_t size;
    time_t timestamp;
    char* message;
};

bool readMessage(int socket,Message& msg);

void sendMessage(int socket, Message& msg);

#endif 
