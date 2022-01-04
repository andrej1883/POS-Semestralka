#include <printf.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "errors.h"
#include "clientHandler.h"
#include "client.h"

void welcomeCli(int sockfd) {
    char buffer[256];
    int exitFlag = 0;
    int option;

    while(exitFlag == 0) {
        printf("Welcome to chat app \n");
        printf("Select your option: \n");
        printf("1. Create account\n");
        printf("2. Log in\n");
        printf("3. exit\n");
        printf("Your option: ");

        bzero(buffer, 256); //vynulujem buffer
        fgets(buffer, 255, stdin); //naplnim buffer
        chScWErr(write(sockfd, buffer, strlen(buffer)));

        option  = atoi(buffer);
        switch (option) {
            case 1:
                bzero(buffer, 256); //vynulujem buffer
                chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
                printf("%s\n", buffer); //vypisem spravu od serveru
                exitFlag = 1;
                registerClie(sockfd);
                break;
            case 2:
                bzero(buffer, 256); //vynulujem buffer
                chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
                printf("%s\n", buffer); //vypisem spravu od serveru
                exitFlag = 1;
                authClie(sockfd);
                break;
            case 3:
                bzero(buffer, 256); //vynulujem buffer
                chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
                printf("%s\n", buffer); //vypisem spravu od serveru
                exit(0);
            default:
                bzero(buffer, 256); //vynulujem buffer
                chScRErr(read(sockfd, buffer, 255));
                printf("%s\n", buffer); //vypisem spravu od serveru
                exitFlag = 0;
                //printf("here we go again\n");
        }
    }

}

void loggedMenuCli(int sockfd, char name[10]) {
    char buffer[256];
    int exitFlag = 0;
    int option;

    bzero(buffer, 256); //vynulujem buffer
    strcpy(buffer,name);
    chScWErr(write(sockfd, buffer, strlen(buffer)));


    while(exitFlag == 0) {
        printf("What you want to do?\n");
        printf("Select your option: \n");
        printf("1. Delete account\n");
        printf("2. Log out\n");
        printf("3. Add friend\n");
        printf("4. Remove friend\n");
        printf("5. Messages\n");
        printf("6. Send file\n");
        printf("7. Start group chat\n");
        printf("8. Friend requests\n");
        printf("9. exit\n");

        printf("Your option: ");
        bzero(buffer, 256); //vynulujem buffer
        fgets(buffer, 255, stdin); //naplnim buffer
        chScWErr(write(sockfd, buffer, strlen(buffer)));
        option  = atoi(buffer);

        switch (option) {
            case 1:
                exitFlag = 1;
                welcomeCli(sockfd);
                break;
            case 2:
                exitFlag = 1;
                welcomeCli(sockfd);
                break;
            case 3:
                exitFlag = 1;
                addFriendClie(sockfd);
                break;
            case 4:
                exitFlag = 1;
                removeFriendClie(sockfd);
                break;
            case 5:
                exitFlag = 1;
                msgMenuCli(sockfd);
                break;
            case 6:
                printf("Not implemented yet\n");
                break;
            case 7:
                printf("Not implemented yet\n");
                break;
            case 8:
                exitFlag = 1;
                manageRequestsClie(sockfd);
                break;
            case 9:
                bzero(buffer, 256); //vynulujem buffer
                chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
                printf("%s\n", buffer); //vypisem spravu od serveru
                exit(0);
            default:
                exitFlag = 0;
                //printf("here we go again\n");
        }
    }
}

void msgMenuCli(int sockfd) {
    char buffer[256];
    int exitFlag = 0;
    int option;

    while(exitFlag == 0) {
        printf("Select your option: \n");
        printf("1. Send DM\n");
        printf("2. Read messages\n");
        printf("3. exit\n");
        printf("Your option: ");

        bzero(buffer, 256); //vynulujem buffer
        fgets(buffer, 255, stdin); //naplnim buffer
        chScWErr(write(sockfd, buffer, strlen(buffer)));

        option  = atoi(buffer);
        switch (option) {
            case 1:
                exitFlag = 1;
                sendMessageClie(sockfd);
                break;
            case 2:
                exitFlag = 1;
                getMessagesClie(sockfd);
                break;
            case 3:
                exitFlag = 1;
                backTologMenu(sockfd);

                break;
            default:
                bzero(buffer, 256); //vynulujem buffer
                chScRErr(read(sockfd, buffer, 255));
                printf("%s\n", buffer); //vypisem spravu od serveru
                exitFlag = 0;
                //printf("here we go again\n");
        }
    }
}
