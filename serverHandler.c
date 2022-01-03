//
// Created by pc on 3. 1. 2022.
//

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
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
                exitFlag = 0;
                break;
            case 4:
                exitFlag = 0;
                break;
            case 5:
                exitFlag = 0;
                break;
            case 6:
                exitFlag = 0;
                break;
            case 7:
                exitFlag = 0;
                break;
            case 8:
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
