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
        bzero(buffer, 256); //vynulujem buffer
        chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
        printf("%s\n", buffer); //vypisem spravu od serveru


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
                exitFlag = 0;
                printf("here we go again\n");
        }
    }
}

void loggedMenuCli(int sockfd, char* name) {
    char buffer[256];
    int exitFlag = 0;
    int option;

    while(exitFlag == 0) {
        bzero(buffer, 256); //vynulujem buffer
        chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
        printf("%s\n", buffer); //vypisem spravu od serveru

        printf("Select your option: \n");
        printf("1. Delete account\n");
        printf("2. Log out\n");
        printf("3. Add friend\n");
        printf("4. Remove friend\n");
        printf("5. Send DM\n");
        printf("6. Send file\n");
        printf("7. Start group chat\n");
        printf("8. exit\n");

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
                bzero(buffer, 256);
                chScWErr(write(sockfd,name, sizeof(name)));
                exitFlag = 1;
                break;
            case 2:
                bzero(buffer, 256); //vynulujem buffer
                chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
                printf("%s\n", buffer); //vypisem spravu od serveru
                exitFlag = 1;
                welcomeCli(sockfd);
                break;
            case 3:
                bzero(buffer, 256); //vynulujem buffer
                chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
                printf("%s\n", buffer); //vypisem spravu od serveru
                break;
            case 4:
                bzero(buffer, 256); //vynulujem buffer
                chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
                printf("%s\n", buffer); //vypisem spravu od serveru
                break;
            case 5:
                bzero(buffer, 256); //vynulujem buffer
                chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
                printf("%s\n", buffer); //vypisem spravu od serveru
                break;
            case 6:
                bzero(buffer, 256); //vynulujem buffer
                chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
                printf("%s\n", buffer); //vypisem spravu od serveru
                break;
            case 7:
                bzero(buffer, 256); //vynulujem buffer
                chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
                printf("%s\n", buffer); //vypisem spravu od serveru
                break;
            case 8:
                bzero(buffer, 256); //vynulujem buffer
                chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
                printf("%s\n", buffer); //vypisem spravu od serveru
                exit(0);
            default:
                exitFlag = 0;
                printf("here we go again\n");
        }
    }
}