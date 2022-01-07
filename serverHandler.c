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
        printf("welcome loop\n");
        bzero(buffer, 10); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 10));

        option = atoi(buffer);
        switch (option) {
            case 1:
                printf("welcome opt1\n");
                msg = "Option 1\n";
                chScWErr(write(newsockfd, msg, strlen(msg) + 1));
                exitFlag = 1;
                registerUser(newsockfd);
                break;
            case 2:
                printf("welcome opt2\n");
                msg = "Option 2\n";
                chScWErr(write(newsockfd, msg, strlen(msg)+1));
                exitFlag = 1;
                authServ(newsockfd);
                break;
            case 3:
                printf("welcome opt3\n");
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

    if (username[1] == '\000') {
        bzero(username, sizeof(username));
        chScRErr(read(newsockfd, username, 10));
        trimNL(username, sizeof(username));
    }


    while (exitFlag == 0) {
        printf("logged loop\n");
        bzero(buffer, 10); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 10));
        printf("logged buffer read: %s\n",buffer);

        option = atoi(buffer);
        switch (option) {
            /*case 0:
                printf("logged opt0\n");
                sendFileServ(newsockfd);
                break;*/
            case 1:
                printf("logged opt1\n");
                deleteUser(username);
                welcomeServ(newsockfd);
                exitFlag = 1;
                break;
            case 2:
                printf("logged opt2\n");
                exitFlag = 1;
                welcomeServ(newsockfd);
                break;
            case 3:
                printf("logged opt3\n");
                exitFlag = 1;
                addFriend(newsockfd, username);
                break;
            case 4:
                printf("logged opt4\n");
                exitFlag = 1;
                removeFriend(newsockfd, username);
                break;
            case 5:
                printf("logged opt5\n");
                exitFlag = 1;
                msgMenuServ(newsockfd, username);
                break;
            case 6:
                printf("logged opt6\n");
                exitFlag = 1;
                fileMenuServ(newsockfd);
                break;
            case 7:
                printf("logged opt7\n");
                exitFlag = 1;
                groupMenuServ(newsockfd, username);
                break;
            case 8:
                printf("logged opt8\n");
                exitFlag = 1;
                manageRequests(newsockfd, username);
                break;
            case 9:
                printf("logged opt9\n");
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
                printf("file opt1\n");
                rcvFileServ(newsockfd);
                loggedMenuServ(newsockfd);
                break;
            case 2:
                exitFlag = 1;
                sendFileServ(newsockfd);
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
