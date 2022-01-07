//
// Created by pc on 3. 1. 2022.
//

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "serverHandler.h"
#include "errors.h"
#include "server.h"

char username[10];

void welcomeServ(int newsockfd) {
    char buffer[10];
    int exitFlag = 0;
    int option;
    char *msg;

    bzero(username, sizeof(username));

    while (exitFlag == 0) {
        bzero(buffer, 10); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 10));

        option = atoi(buffer);
        switch (option) {
            case 1:
                msg = "Option 1\n";
                chScWErr(write(newsockfd, msg, strlen(msg) + 1));
                exitFlag = 1;
                registerUser(newsockfd);
                break;
            case 2:
                msg = "Option 2\n";
                chScWErr(write(newsockfd, msg, strlen(msg)+1));
                exitFlag = 1;
                authServ(newsockfd);
                break;
            case 3:
                msg = "See you soon :)\n";
                chScWErr(write(newsockfd, msg, strlen(msg)+1));
                exit(0);

            default:
                printf("welcomeMenu cycle\n");
                msg = "Choose correct option!\n";
                chScWErr(write(newsockfd, msg, strlen(msg)+1));
        }
    }
}

void loggedMenuServ(int newsockfd) {
    char buffer[10];
    int exitFlag = 0;
    int option;
    char *msg;

    //if (username[1] == '\000') {
        bzero(username, sizeof(username));
        chScRErr(read(newsockfd, username, 10));
        trimNL(username, sizeof(username));
    //}


    while (exitFlag == 0) {
        bzero(buffer, 10); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 10));

        option = atoi(buffer);
        switch (option) {
            case 0:
                sendFileServ(newsockfd,username);
                exitFlag = 1;
                break;
            case 1:
                deleteUser(username);
                welcomeServ(newsockfd);
                exitFlag = 1;
                break;
            case 2:
                exitFlag = 1;
                welcomeServ(newsockfd);
                break;
            case 3:
                exitFlag = 1;
                addFriend(newsockfd, username);
                break;
            case 4:
                exitFlag = 1;
                removeFriend(newsockfd, username);
                break;
            case 5:
                exitFlag = 1;
                msgMenuServ(newsockfd, username);
                break;
            case 6:
                exitFlag = 1;
                fileMenuServ(newsockfd);
                break;
            case 7:
                exitFlag = 1;
                groupMenuServ(newsockfd, username);
                break;
            case 8:
                exitFlag = 1;
                manageRequests(newsockfd, username);
                break;
            case 9:
                msg = "See you soon :)\n";
                chScWErr(write(newsockfd, msg, strlen(msg)+1));
                exit(0);

            default:
                printf("loggedMenu cycle\n");
                exitFlag = 0;
                msg = "Choose correct option!\n";
                chScWErr(write(newsockfd, msg, strlen(msg) + 1));
                loggedMenuServ(newsockfd);
        }
    }
}

void msgMenuServ(int newsockfd, char *username) {
    char buffer[10];
    int exitFlag = 0;
    int option;
    char* msg;

    while (exitFlag == 0) {
        bzero(buffer,10); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 10));

        option = atoi(buffer);
        switch (option) {
            case 1:
                exitFlag = 1;
                sendMessage(newsockfd, username);
                break;
            case 2:
                exitFlag = 1;
                readMessages(newsockfd, username);
                break;
            case 3:
                exitFlag = 1;
                loggedMenuServ(newsockfd);
                break;

            default:
                printf("msgMenu cycle\n");
                msg = "Choose correct option!\n";
                chScWErr(write(newsockfd, msg, strlen(msg)+1));
        }
    }
}

void fileMenuServ(int newsockfd) {
    char buffer[10];
    int exitFlag = 0;
    int option;
    char *msg;

    while (exitFlag == 0) {
        bzero(buffer, 10); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 10));

        option = atoi(buffer);
        switch (option) {
            case 1:
                exitFlag = 1;
                rcvFileServ(newsockfd);
                loggedMenuServ(newsockfd);
                break;
            case 2:
                exitFlag = 1;
                sendFileServ(newsockfd,username);
                loggedMenuServ(newsockfd);
                break;
            case 3:
                exitFlag = 1;
                loggedMenuServ(newsockfd);
                break;

            default:
                printf("fileMenu cycle\n");
                msg = "Choose correct option!\n";
                chScWErr(write(newsockfd, msg, strlen(msg)+1));
        }
    }
}

void groupMenuServ(int newsockfd, char *username) {
    char buffer[10];
    int exitFlag = 0;
    int option;
    char* msg;

    while (exitFlag == 0) {
        bzero(buffer,10); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 10));

        option = atoi(buffer);
        switch (option) {
            case 1:
                exitFlag = 1;
                createGroup(newsockfd, username);
                break;
            case 2:
                exitFlag = 1;
                addMember(newsockfd, username);
                break;
            case 3:
                exitFlag = 1;
                removeMember(newsockfd, username);
                break;
            case 4:
                exitFlag = 1;
                sendGroupMessage(newsockfd, username);
                break;
            case 5:
                exitFlag = 1;
                getGroupMessages(newsockfd, username);
                break;

            case 6:
                exitFlag = 1;
                loggedMenuServ(newsockfd);
                break;

            default:
                msg = "Choose correct option!\n";
                chScWErr(write(newsockfd, msg, strlen(msg)+1));
        }
    }
}
