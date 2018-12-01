#ifndef utils 
#define utils 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

int readFromSocket(int sockfd, void *buffer, int bytes2Read) {
    int bytesReaded = 0;
    int result;
    while (bytesReaded < bytes2Read) {
        result = read(sockfd, (char *) buffer + bytesReaded, bytes2Read - bytesReaded);
        if (result < 0) {
            perror("read failed");
            return -1;
        } else if (result == 0) {
            return 0;
        }
        bytesReaded += result;
    }
    return bytesReaded;
}

int writeToSocket(int sockfd, void *buffer, int bytes2Write) {
    int bytesWrited = 0;
    int result;
    while (bytesWrited < bytes2Write) {
        result = write(sockfd, (char *) buffer + bytesWrited, bytes2Write - bytesWrited);
        if (result < 0) {
            perror("write failed");
            return -1;
        } else if (result == 0) {
            return 0;
        }
        bytesWrited += result;
    }
    return bytesWrited;
}

int readFromSocketToFile(int sockfd, void *buffer, int bytes2Read) {
    int bytesReaded = 0;
    int result;
    while (bytesReaded < bytes2Read) {
        result = read(sockfd, (char *) buffer + bytesReaded, bytes2Read - bytesReaded);
        if (result < 0) {
            perror("read failed");
            return -1;
        } else if (result == 0) {
            return 0;
        }
        bytesReaded += result;
    }
    return bytesReaded;
}

void toUpperCase(char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        str[i] = toupper(str[i]);
    }
}

void error(const char *err) {
   perror(err) ;
   exit(1);
}

#endif
