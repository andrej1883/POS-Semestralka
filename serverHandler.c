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

void welcomeServ(int newsockfd) {
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
                msg = "Option 1\n";
                chScWErr(write(newsockfd, msg, strlen(msg)+1));
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
                msg = "Choose correct option!\n";
                chScWErr(write(newsockfd, msg, strlen(msg)+1));
        }
    }
}

void loggedMenuServ(int newsockfd) {
    char buffer[10];
    int exitFlag = 0;
    int option;
    char* msg;
    char username[10];

    bzero(username,10); //vynulujem buffer
    chScRErr(read(newsockfd, username, 10));
    trimNL(username,sizeof (username));

    while (exitFlag == 0) {
        bzero(buffer,10); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 10));

        option = atoi(buffer);
        switch (option) {
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
                fileMenuServ(newsockfd, username);
                break;
            case 7:
                exitFlag = 0;
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
                exitFlag = 0;
                msg = "Choose correct option!\n";
                chScWErr(write(newsockfd, msg, strlen(msg)+1));
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
                getMessages(newsockfd, username);
                break;
            case 3:
                exitFlag = 1;
                loggedMenuServ(newsockfd);
                break;

            default:
                msg = "Choose correct option!\n";
                chScWErr(write(newsockfd, msg, strlen(msg)+1));
        }
    }
}

/*void getFileDialogServ(int newsockfd) {
    char buffer[256];
    user *managingUser = (user*) malloc(sizeof (user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, username) == 0){
            managingUser = users[i];
        }
    }

    if (managingUser->numFrd !=0) {
        bzero(buffer, 256);
        strcpy(buffer, "Choose friend: \n");

        for (int i = 0; i < managingUser->numFrd; ++i) {
            int val = i + 1;
            //char num = val +'0';          Nefunkcna konverzia int na char, kvoli tomuto nie su poradove cisla
            //strcat(buffer, num);
            strcat(buffer, managingUser->friendlist[i]->fUsername);
            strcat(buffer, "\n");
        }

        chScWErr(write(newsockfd, buffer, strlen(buffer) + 1));

        bzero(buffer, 256); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 256));

        int chosenFrd;
        sscanf(buffer, "%d", &chosenFrd);
        }
}*/

void fileMenuServ(int newsockfd, char *username) {
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
                rcvFileServ(newsockfd);
                break;
            case 2:
                exitFlag = 1;
                getMessages(newsockfd, username);
                break;
            case 3:
                exitFlag = 1;
                loggedMenuServ(newsockfd);
                break;

            default:
                msg = "Choose correct option!\n";
                chScWErr(write(newsockfd, msg, strlen(msg)+1));
        }
    }
}