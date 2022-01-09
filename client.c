#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "errors.h"
#include "clientHandler.h"
#include "client.h"
#include "server.h"

#define MSGBUFFSIZE 256
char myName[10];


void authClie(int sockfd) {
    char buffer[MSGBUFFSIZE];

    printf("Log into server: \n");
    printf("Please enter username: ");

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE)); //zapisem buffer na server
    for (int i = 0; i < 10; ++i) {
        myName[i] = buffer[i];
    }
    trimNL(myName,sizeof (myName));

    printf("Please enter password: ");
    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE)); //zapisem buffer na server


    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    if(strcmp(buffer,"Login or password incorrect!") == 0) {
        welcomeCli(sockfd);
    } else {
        loggedMenuCli(sockfd);
    }

}

void addFriendClie(int sockfd) {
    char buffer[MSGBUFFSIZE];

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    //printf("Please enter number of user you wish to add: ");
    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE)); //zapisem buffer na server

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    if(strcmp(buffer,"Friend request sent!") == 0) {
        loggedMenuCli(sockfd);
    }
}

void getMessagesClie(int sockfd) { //read messages on server
    char buffer[MSGBUFFSIZE];
    int n;

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    n = recv(sockfd, buffer, MSGBUFFSIZE, MSG_WAITALL);
    if(n < 0){
        perror("Receive name Error:");
    }
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    send(sockfd,buffer,MSGBUFFSIZE,MSG_EOR);

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    n = recv(sockfd, buffer,MSGBUFFSIZE, MSG_WAITALL);
    if(n < 0){
        perror("Receive name Error:");
    }
    printf("%s\n", buffer); //vypisem spravu od serveru

    msgMenuCli( sockfd);
}

void registerClie(int sockfd) {
    char buffer[MSGBUFFSIZE];
    int n;

    printf("Create account: \n");
    printf("Please enter username: ");

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    n = send(sockfd,buffer,MSGBUFFSIZE,MSG_EOR);
    if(n < 0){
        perror("Send option Error:");
    }
    for (int i = 0; i < 10; ++i) {
        myName[i] = buffer[i];
    }
    trimNL(myName,sizeof (myName));

    printf("Please enter password: ");
    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    n = send(sockfd,buffer,MSGBUFFSIZE,MSG_EOR);
    if(n < 0){
        perror("Send option Error:");
    }


    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    n = recv(sockfd,buffer,MSGBUFFSIZE,MSG_WAITALL);
    if(n < 0){
        perror("Receive option Error:");
    }

    printf("%s\n", buffer); //vypisem spravu od serveru

    if(strcmp(buffer,"User sucesfully registered") == 0) {
        loggedMenuCli(sockfd);
    }
}

void sendFileInfoCLie(int sockfd, char *filename, char *toUser) {
    char buffer[MSGBUFFSIZE];

    bzero(buffer,MSGBUFFSIZE);
    strcat(buffer,filename);
    strcat(buffer," ");
    strcat(buffer,myName);
    strcat(buffer," ");
    strcat(buffer,toUser);
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE));
}

void sendFileClie(char* filename,int sockfd, char* toUser) {
    FILE *filePointer;
    char data[1024] = {0};
    char buffer[2048];
    trimNL(filename,sizeof (filename));
    if( access( filename, F_OK ) == 0 ) {
        sendFileInfoCLie(sockfd,filename,toUser);
        filePointer = fopen(filename, "r") ;
        bzero(buffer,sizeof (buffer));
        while( fgets ( data, 50, filePointer ) != NULL )
        {
            strcat(buffer,data);
        }
        send(sockfd,buffer,2048,0);
        bzero(data, 1024);
        fclose(filePointer);
    } else {
        printf("File not found!\n");
    }
}

void rcvFileCli(int sockfd) {
    char buffer[MSGBUFFSIZE];
    int n;

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    n = recv(sockfd, buffer, MSGBUFFSIZE, MSG_WAITALL);
    if(n < 0){
        perror("Receive name Error:");
    }
    printf("%s\n", buffer); //vypisem spravu od serveru

    if (strcmp(buffer, "There are no available files for you\n") != 0) {

        printf("Please enter number of file: ");
        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
        send(sockfd,buffer,MSGBUFFSIZE,MSG_EOR);

        FILE *filepointer;
        char filename[256];
        char fileBuffer[2048];
        char directory[100] = "downloadedFiles/";

        struct stat st = {0};
        if (stat(directory, &st) == -1) {
            mkdir(directory, 0700);
        }
        strcat(directory, myName);
        strcat(directory, "/");
        if (stat(directory, &st) == -1) {
            mkdir(directory, 0700);
        }

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        n = recv(sockfd, buffer, MSGBUFFSIZE, MSG_WAITALL);
        if(n < 0){
            perror("Receive name Error:");
        }
        strcpy(filename, buffer);

        strcat(directory, filename);

        bzero(fileBuffer, sizeof(fileBuffer));
        filepointer = fopen(directory, "w");
        n = recv(sockfd, fileBuffer, 2048, MSG_WAITALL);
        if (n < 0) {
            perror("Receive file Error:");
        }

        fprintf(filepointer,"%s",fileBuffer);
        bzero(fileBuffer, 2048);
        fclose(filepointer);
    }
}

char* getMyName() {
    return myName;
}

int client(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent* server; //uchovava informacie o serveri

    if (argc < 3)
    {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        return 1;
    }

    server = gethostbyname(argv[1]); //naplni informacie o serveri, dovoluje posielat nazvy ako localhost frios.fri.uniza a pod...
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        return 2;
    }

    bzero((char*)&serv_addr, sizeof(serv_addr)); //vynulovanie
    serv_addr.sin_family = AF_INET; //sietove sockety
    bcopy(      //na zaklade gethostbyname naplnime udaje
            (char*)server->h_addr, //vyberie ciselnu ip adresu
            (char*)&serv_addr.sin_addr.s_addr, //priradi ju
            server->h_length //dlzka adresy v bajtoch
    );
    serv_addr.sin_port = htons(atoi(argv[2])); //little/big endian

    chScCRErr(sockfd = socket(AF_INET, SOCK_STREAM, 0)); //vytvorim tcp internetovy socket

    chScCNErr(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))); // cez vytvoreny socket sa pripojim (blokujuce volanie, caka kym sa pripojim na server)


    //--------------------------------jadro aplikacie--------------------------------------------------------------------
    welcomeCli(sockfd);
    close(sockfd); //uzavriem socket
    exit(0);
    //--------------------------------jadro aplikacie--------------------------------------------------------------------
}

void manageRequestsClie(int sockfd) {
    char buffer[MSGBUFFSIZE];

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    //printf("Please enter number of request: ");
    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE)); //zapisem buffer na server

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE)); //zapisem buffer na server

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    loggedMenuCli(sockfd);
}

void removeFriendClie(int sockfd) {
    char buffer[MSGBUFFSIZE];

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    printf("Please enter number of friend: ");
    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE)); //zapisem buffer na server

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru


    if((strcmp(buffer,"Friend removed!\n") == 0) || (strcmp(buffer,"You can add friends in following menu  \n") == 0)) {
        loggedMenuCli(sockfd);
    }
}

void backTologMenu(int sockfd) {
    loggedMenuCli(sockfd);
}

void sendMessageClie(int sockfd) {
    char buffer[MSGBUFFSIZE];

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    //printf("Please enter number of user you wish to message: ");
    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE)); //zapisem buffer na server

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    //printf("Please enter text: ");
    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE)); //zapisem buffer na server

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    loggedMenuCli(sockfd);
}

void addMemberClie(int sockfd) {
    char buffer[MSGBUFFSIZE];

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE)); //zapisem buffer na server

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE)); //zapisem buffer na server

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    loggedMenuCli(sockfd);
}

void removeMemberClie(int sockfd) {
    char buffer[MSGBUFFSIZE];

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE)); //zapisem buffer na server

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    loggedMenuCli(sockfd);
}

void createGroupClie(int sockfd) {
    char buffer[MSGBUFFSIZE];

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE)); //zapisem buffer na server

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    loggedMenuCli(sockfd);
}

void sendGroupMessageClie(int sockfd) {
    char buffer[MSGBUFFSIZE];

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE)); //zapisem buffer na server

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE)); //zapisem buffer na server

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    loggedMenuCli(sockfd);
}

void getGroupMessagesClie(int sockfd) {
    char buffer[MSGBUFFSIZE];

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    fgets(buffer, MSGBUFFSIZE, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, MSGBUFFSIZE)); //zapisem buffer na server

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(sockfd, buffer, MSGBUFFSIZE)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    loggedMenuCli(sockfd);
}
