//
// Created by pc on 3. 1. 2022.
//

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <pthread.h>
#include "serverHandler.h"
#include "errors.h"
#include "server.h"

#define MSGBUFFSIZE 256


static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void welcomeServ(int newsockfd) {
    char buffer[MSGBUFFSIZE];
    int exitFlag = 0;
    int option;
    char *msg;
    int n;


    while (exitFlag == 0) {

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        n = recv(newsockfd,buffer,MSGBUFFSIZE,MSG_WAITALL);
        if(n < 0){
            perror("Receive option Error:");
        }


        option = atoi(buffer);
        switch (option) {
            case 1:
                registerUser(newsockfd);
                break;
            case 2:
                authServ(newsockfd);
                break;
            case 3:
                msg = "See you soon :)\n";
                n = send(newsockfd,msg,MSGBUFFSIZE,MSG_EOR);
                if(n < 0){
                    perror("Send option Error:");
                }
                exit(0);

            default:
                printf("welcomeMenu cycle\n");
                msg = "Choose correct option!\n";
                n = send(newsockfd,msg,MSGBUFFSIZE,MSG_EOR);
                if(n < 0){
                    perror("Send option Error:");
                }
        }
    }
}

void loggedMenuServ(int newsockfd) {
    char buffer[MSGBUFFSIZE];
    int exitFlag = 0;
    int option;
    char *msg;
    int n;

    bzero(buffer, MSGBUFFSIZE);
    n = recv(newsockfd,buffer,MSGBUFFSIZE,MSG_WAITALL);
        if(n < 0){
            perror("Receive option Error:");
        }
        trimNL(buffer, MSGBUFFSIZE);

    pthread_mutex_lock(&mutex);
    setUsername(buffer,newsockfd);
    pthread_mutex_unlock(&mutex);


    while (exitFlag == 0) {
        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        n = recv(newsockfd,buffer,MSGBUFFSIZE,MSG_WAITALL);
        if(n < 0){
            perror("Receive option Error:");
        }

        option = atoi(buffer);
        switch (option) {
            case 0:
                sendFileServ(newsockfd,getUsername(newsockfd));
                exitFlag = 1;
                break;
            case 1:
                deleteUser(getUsername(newsockfd));
                welcomeServ(newsockfd);
                exitFlag = 1;
                break;
            case 2:
                exitFlag = 1;
                welcomeServ(newsockfd);
                break;
            case 3:
                exitFlag = 1;
                addFriend(newsockfd, getUsername(newsockfd));
                break;
            case 4:
                exitFlag = 1;
                removeFriend(newsockfd, getUsername(newsockfd));
                break;
            case 5:
                exitFlag = 1;
                msgMenuServ(newsockfd);
                break;
            case 6:
                exitFlag = 1;
                fileMenuServ(newsockfd);
                break;
            case 7:
                exitFlag = 1;
                groupMenuServ(newsockfd);
                break;
            case 8:
                exitFlag = 1;
                manageRequests(newsockfd, getUsername(newsockfd));
                break;
            case 9:
                msg = "See you soon :)\n";
                n = send(newsockfd,msg,MSGBUFFSIZE,MSG_EOR);
                if(n < 0){
                    perror("Send option Error:");
                }
                exit(0);

            default:
                msg = "Choose correct option!\n";
                n = send(newsockfd,msg,MSGBUFFSIZE,MSG_EOR);
                if(n < 0){
                    perror("Send option Error:");
                }
        }
    }
}

void msgMenuServ(int newsockfd) {
    char buffer[MSGBUFFSIZE];
    int exitFlag = 0;
    int option;
    char* msg;

    while (exitFlag == 0) {
        bzero(buffer,MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        option = atoi(buffer);
        switch (option) {
            case 1:
                exitFlag = 1;
                sendMessage(newsockfd, getUsername(newsockfd));
                break;
            case 2:
                exitFlag = 1;
                readMessages(newsockfd, getUsername(newsockfd));
                break;
            case 3:
                exitFlag = 1;
                loggedMenuServ(newsockfd);
                break;

            default:
                printf("msgMenu cycle\n");
                msg = "Choose correct option!\n";
                chScWErr(write(newsockfd, msg, MSGBUFFSIZE));
        }
    }
}

void fileMenuServ(int newsockfd) {
    char buffer[MSGBUFFSIZE];
    int exitFlag = 0;
    int option;
    char *msg;
    int n;

    while (exitFlag == 0) {
        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        n = recv(newsockfd,buffer,MSGBUFFSIZE,MSG_WAITALL);
        if(n < 0){
            perror("Receive option Error:");
        }

        option = atoi(buffer);
        switch (option) {
            case 1:
                exitFlag = 1;
                rcvFileServ(newsockfd);
                loggedMenuServ(newsockfd);
                break;
            case 2:
                exitFlag = 1;
                sendFileServ(newsockfd,getUsername(newsockfd));
                loggedMenuServ(newsockfd);
                break;
            case 3:
                exitFlag = 1;
                loggedMenuServ(newsockfd);
                break;

            default:
                msg = "Choose correct option!\n";
                n = send(newsockfd,msg,MSGBUFFSIZE,MSG_EOR);
                if(n < 0){
                    perror("Send option Error:");
                }
        }
    }
}

void groupMenuServ(int newsockfd) {
    char buffer[MSGBUFFSIZE];
    int exitFlag = 0;
    int option;
    char* msg;

    while (exitFlag == 0) {
        bzero(buffer,MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        option = atoi(buffer);
        switch (option) {
            case 1:
                exitFlag = 1;
                createGroup(newsockfd, getUsername(newsockfd));
                break;
            case 2:
                exitFlag = 1;
                addMember(newsockfd, getUsername(newsockfd));
                break;
            case 3:
                exitFlag = 1;
                removeMember(newsockfd, getUsername(newsockfd));
                break;
            case 4:
                exitFlag = 1;
                sendGroupMessage(newsockfd, getUsername(newsockfd));
                break;
            case 5:
                exitFlag = 1;
                getGroupMessages(newsockfd, getUsername(newsockfd));
                break;

            case 6:
                exitFlag = 1;
                loggedMenuServ(newsockfd);
                break;

            default:
                msg = "Choose correct option!\n";
                chScWErr(write(newsockfd, msg, MSGBUFFSIZE));
        }
    }
}
