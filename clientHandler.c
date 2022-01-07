#include <printf.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "errors.h"
#include "clientHandler.h"
#include "client.h"
#include "server.h"

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

void loggedMenuCli(int sockfd) {
    char buffer[256];
    int exitFlag = 0;
    int option;

    bzero(buffer, 256); //vynulujem buffer
    strcpy(buffer,getMyName());
    chScWErr(write(sockfd, buffer, strlen(buffer)));


    while(exitFlag == 0) {
        printf("What you want to do?\n");
        printf("Select your option: \n");
        printf("0. Test download\n");
        printf("1. Delete account\n");
        printf("2. Log out\n");
        printf("3. Add friend\n");
        printf("4. Remove friend\n");
        printf("5. Messages\n");
        printf("6. Files\n");
        printf("7. Group chats\n");
        printf("8. Friend requests\n");
        printf("9. exit\n");

        printf("Your option: ");
        bzero(buffer, 256); //vynulujem buffer
        fgets(buffer, 255, stdin); //naplnim buffer
        chScWErr(write(sockfd, buffer, strlen(buffer)));
        option  = atoi(buffer);

        switch (option) {
            case 0:
                rcvFileCli(sockfd);
                exitFlag = 1;
                break;
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
                exitFlag = 1;
                fileMenuCli(sockfd);
                break;
            case 7:
                exitFlag = 1;
                groupMenuCli(sockfd);
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

void fileMenuCli(int sockfd) {
    char buffer[256];
    char buffer2[256];
    int exitFlag = 0;
    int option;

    while(exitFlag == 0) {
        printf("Select your option: \n");
        printf("1. Send file\n");
        printf("2. Download files\n");
        printf("3. exit\n");
        printf("Your option: ");

        bzero(buffer, 256); //vynulujem buffer
        fgets(buffer, 255, stdin); //naplnim buffer
        chScWErr(write(sockfd, buffer, strlen(buffer)));

        option  = atoi(buffer);
        switch (option) {
            case 1:
                printf("Enter file name: ");
                bzero(buffer, 256); //vynulujem buffer
                fgets(buffer, 255, stdin); //naplnim buffer
                trimNL(buffer,sizeof (buffer));
                printf("Send to: ");
                bzero(buffer2, 256); //vynulujem buffer
                fgets(buffer2, 255, stdin); //naplnim buffer
                trimNL(buffer2,sizeof (buffer2));
                exitFlag = 1;
                sendFileClie(buffer,sockfd,buffer2);
                break;
            case 2:
                exitFlag = 1;
                rcvFileCli(sockfd);
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

void groupMenuCli(int sockfd) {
    char buffer[256];
    int exitFlag = 0;
    int option;

    while(exitFlag == 0) {
        printf("Select your option: \n");
        printf("1. Create new chat\n");
        printf("2. Add member to chat\n");
        printf("3. Leave chat\n");
        printf("4. Send message to chat\n");
        printf("5. Read chat messages\n");
        printf("6. exit\n");
        printf("Your option: ");

        bzero(buffer, 256); //vynulujem buffer
        fgets(buffer, 255, stdin); //naplnim buffer
        chScWErr(write(sockfd, buffer, strlen(buffer)));

        option  = atoi(buffer);
        switch (option) {
            case 1:
                exitFlag = 1;
                createGroupClie(sockfd);
                break;
            case 2:
                exitFlag = 1;
                addMemberClie(sockfd);
                break;
            case 3:
                exitFlag = 1;
                removeMemberClie(sockfd);
                break;
            case 4:
                exitFlag = 1;
                sendGroupMessageClie(sockfd);
                break;
            case 5:
                exitFlag = 1;
                getGroupMessagesClie(sockfd);
                break;
            case 6:
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
