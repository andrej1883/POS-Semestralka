#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "errors.h"
#include "clientHandler.h"
#include "client.h"
#include "server.h"

#define MSGBUFFSIZE 256

void welcomeCli(int sockfd) {
    char buffer[MSGBUFFSIZE];
    int exitFlag = 0;
    int option;
    int n;

    while(exitFlag == 0) {
        printf("Welcome to chat app \n");
        printf("Select your option: \n");
        printf("1. Create account\n");
        printf("2. Log in\n");
        printf("3. exit\n");
        printf("Your option: ");


        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
        n = send(sockfd,buffer,MSGBUFFSIZE,MSG_EOR);
        if(n < 0){
            perror("Send option Error:");
        }

        option  = atoi(buffer);
        switch (option) {
            case 1:
                registerClie(sockfd);
                break;
            case 2:
                authClie(sockfd);
                break;
            case 3:
                bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
                n = recv(sockfd,buffer,MSGBUFFSIZE,MSG_WAITALL);
                if(n < 0){
                    perror("Receive option Error:");
                }
                printf("%s\n", buffer); //vypisem spravu od serveru
                exit(0);
            default:
                bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
                n = recv(sockfd,buffer,MSGBUFFSIZE,MSG_WAITALL);
                if(n < 0){
                    perror("Receive option Error:");
                }
                printf("%s\n", buffer); //vypisem spravu od serveru
                exitFlag = 0;
        }
    }

}

void loggedMenuCli(int sockfd) {
    char buffer[MSGBUFFSIZE];
    int exitFlag = 0;
    int option;
    int n;


    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    strcpy(buffer,getMyName());
    n = send(sockfd,buffer,MSGBUFFSIZE,MSG_EOR);
    if(n < 0){
        perror("Send option Error:");
    }



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
        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
        n = send(sockfd,buffer,MSGBUFFSIZE,MSG_EOR);
        if(n < 0){
            perror("Send option Error:");
        }

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
                bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
                chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
                printf("%s\n", buffer); //vypisem spravu od serveru
                exit(0);
            default:
                exitFlag = 0;
        }
    }
}

void msgMenuCli(int sockfd) {
    char buffer[MSGBUFFSIZE];
    int exitFlag = 0;
    int option;

    while(exitFlag == 0) {
        printf("Select your option: \n");
        printf("1. Send DM\n");
        printf("2. Read messages\n");
        printf("3. exit\n");
        printf("Your option: ");

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
        chScWErr(write(sockfd, buffer, MSGBUFFSIZE));

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
                bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
                chScRErr(read(sockfd, buffer, MSGBUFFSIZE));
                printf("%s\n", buffer); //vypisem spravu od serveru
                exitFlag = 0;
                //printf("here we go again\n");
        }
    }
}

void fileMenuCli(int sockfd) {
    char buffer[MSGBUFFSIZE];
    char buffer2[MSGBUFFSIZE];
    int exitFlag = 0;
    int option;
    int n;

    while(exitFlag == 0) {
        printf("Select your option: \n");
        printf("1. Send file\n");
        printf("2. Download files\n");
        printf("3. exit\n");
        printf("Your option: ");

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
        n = send(sockfd,buffer,MSGBUFFSIZE,MSG_EOR);
        if(n < 0){
            perror("Send option Error:");
        }

        option  = atoi(buffer);
        switch (option) {
            case 1:
                printf("Enter file name: ");
                bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
                fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
                trimNL(buffer,MSGBUFFSIZE);
                printf("Send to: ");
                bzero(buffer2, MSGBUFFSIZE); //vynulujem buffer
                fgets(buffer2, MSGBUFFSIZE, stdin); //naplnim buffer
                trimNL(buffer2,MSGBUFFSIZE);
                exitFlag = 1;
                sendFileClie(buffer,sockfd,buffer2);
                loggedMenuCli(sockfd);
                break;
            case 2:
                exitFlag = 1;
                rcvFileCli(sockfd);
                loggedMenuCli(sockfd);
                break;
            case 3:
                exitFlag = 1;
                loggedMenuCli(sockfd);
                break;
            default:
                bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
                //chScRErr(read(sockfd, buffer, 255));
                n = recv(sockfd,buffer,MSGBUFFSIZE,MSG_WAITALL);
                if(n < 0){
                    perror("Receive option Error:");
                }
                printf("%s\n", buffer); //vypisem spravu od serveru
                exitFlag = 0;
        }
    }
}

void groupMenuCli(int sockfd) {
    char buffer[MSGBUFFSIZE];
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

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
        chScWErr(write(sockfd, buffer, MSGBUFFSIZE));

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
                bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
                chScRErr(read(sockfd, buffer, MSGBUFFSIZE));
                printf("%s\n", buffer); //vypisem spravu od serveru
                exitFlag = 0;
        }
    }
}
