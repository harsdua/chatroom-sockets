//server-side utils file for chatroom
//INFOF202
// By Harsh 509461 and Omar 478023

#include <stdlib.h>
#include <sys/socket.h>
#include <cstring>

#include "utils.h"




bool readMessage(int socket,Message& msg){
    int r;
    if (read(socket, &msg.size, sizeof(ssize_t)) == 0) return false;
    if (read(socket, &msg.timestamp, sizeof(time_t)) == 0) return false;

    msg.message = new char[msg.size+1];//static_cast<char *>(malloc(sizeof(char) * msg.size + 1));
    msg.message[msg.size] = '\0';

    int readBytes = 0;
    while (readBytes < ((int)msg.size)) {
        if ((r = read(socket, &msg.message[readBytes], msg.size - readBytes)) == -1) {
            delete[] msg.message;
            msg.message = nullptr;
            return false;
        }
        readBytes += r;
    }

    return true;
}

void sendMessage(int socket, Message& msg){
    send(socket , &msg.size , sizeof(ssize_t) , 0 );
    send(socket , &msg.timestamp , sizeof(time_t) , 0 );
    send(socket , msg.message , strlen(msg.message) , 0 );
}
