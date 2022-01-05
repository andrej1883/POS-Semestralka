#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "errors.h"
#include "clientHandler.h"
#include "client.h"
#include "server.h"

char myName[10];

void authClie(int sockfd) {
    char buffer[256];

    printf("Log into server: \n");
    printf("Please enter username: ");

    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server
    for (int i = 0; i < 10; ++i) {
        myName[i] = buffer[i];
    }

    printf("Please enter password: ");
    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server


    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    if(strcmp(buffer,"Login or password incorrect!") == 0) {
        welcomeCli(sockfd);
    } else {
        loggedMenuCli(sockfd,myName);
    }

}

void addFriendClie(int sockfd) {
    char buffer[256];

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    //printf("Please enter number of user you wish to add: ");
    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    if(strcmp(buffer,"Friend request sent!") == 0) {
        loggedMenuCli(sockfd,myName);
    }
}

void getMessagesClie(int sockfd) {
    char buffer[256];

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    msgMenuCli( sockfd);
}

void getMessagesFromClie(int sockfd) {
    char buffer[256];

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru
}

void registerClie(int sockfd) {
    char buffer[256];

    printf("Create account: \n");
    printf("Please enter username: ");

    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server
    for (int i = 0; i < 10; ++i) {
        myName[i] = buffer[i];
    }

    printf("Please enter password: ");
    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server


    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    if(strcmp(buffer,"User sucesfully registered") == 0) {
        loggedMenuCli(sockfd,myName);
    }
}

void sendFileInfoCLie(int sockfd, char *filename, char *toUser) {
    char buffer[256];

    bzero(buffer,sizeof (buffer));
    strcat(buffer,filename);
    strcat(buffer," ");
    strcat(buffer,myName);
    strcat(buffer," ");
    strcat(buffer,toUser);
    chScWErr(write(sockfd, buffer, sizeof (buffer)));
    bzero(buffer,sizeof (buffer));
}

void sendFileClie(char* filename,int sockfd, char* toUser) {
    FILE *filePointer;
    char data[1024] = {0};
    trimNL(filename,sizeof (filename));
    if( access( filename, F_OK ) == 0 ) {
        sendFileInfoCLie(sockfd,filename,toUser);
        filePointer = fopen(filename, "r") ;
        while( fgets ( data, 1024, filePointer ) != NULL )
        {
            chSFErr(send(sockfd,data,sizeof (data),0));
        }
        bzero(data, 1024);
        fclose(filePointer);
    } else {
        printf("File not found!\n");
    }
}

void rcvFileCli(int newsockfd) {
    //TODO 2: Get files from server
    /*int n;
    FILE *filepointer;
    char *filename = "files/rcv.txt";
    char buffer[1024];
    char username[10];

    bzero(username,10); //vynulujem buffer
    chScRErr(read(newsockfd, username, 10));
    trimNL(username,sizeof (username));

    filepointer = fopen(filename, "w");
    while (1) {
        n = recv(newsockfd, buffer, 1024, 0);
        if (n <= 0){
            break;
        }
        fprintf(filepointer, "%s", buffer);
        bzero(buffer, 1024);
    }*/
}

int client(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent* server; //uchovava informacie o serveri

    char buffer[256];

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
    //signal(SIGPIPE, SIG_IGN);
    //authClie(sockfd);
    //registerClie(sockfd);
    //loggedMenuCli(sockfd,"Lojzik");
    //sendFileClie("file.txt", sockfd,"Pepa");
    //sendFileInfoCLie(sockfd,"file.txt","Pepas");

    welcomeCli(sockfd);
    exit(0);
    for(;;) {
        printf("Please enter a message: ");
        bzero(buffer, 256); //vynulujem buffer
        fgets(buffer, 255, stdin); //naplnim buffer
        if(strcmp(buffer,"exit") == 0) {
            break;
        }
        chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

        getMessagesClie(sockfd);
        //getMessagesFromClie(sockfd);
        bzero(buffer, 256); //vynulujem buffer
        chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera

        printf("%s\n", buffer); //vypisem spravu od serveru
    }
    //--------------------------------jadro aplikacie--------------------------------------------------------------------

    close(sockfd); //uzavriem socket

    return 0;
}

void manageRequestsClie(int sockfd) {
    char buffer[256];

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    //printf("Please enter number of request: ");
    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

    loggedMenuCli(sockfd,myName);
}

void removeFriendClie(int sockfd) {
    char buffer[256];

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    printf("Please enter number of friend: ");
    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru


    if((strcmp(buffer,"Friend removed!\n") == 0) || (strcmp(buffer,"You have no friends :( \n") == 0)) {
        loggedMenuCli(sockfd,myName);
    }
}

void backTologMenu(int sockfd) {
    loggedMenuCli(sockfd,myName);
}

void sendMessageClie(int sockfd) {
    char buffer[256];

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    printf("Please enter number of user you wish to message: ");
    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    printf("Please enter text: ");
    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    loggedMenuCli(sockfd,myName);
}

void addMemberClie(int sockfd) {
    char buffer[256];

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    loggedMenuCli(sockfd,myName);
}

void removeMemberClie(int sockfd) {
    char buffer[256];

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server
    loggedMenuCli(sockfd,myName);
}

void createGroupClie(int sockfd) {
    char buffer[256];

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    loggedMenuCli(sockfd,myName);
}

void sendGroupMessageClie(int sockfd) {
    char buffer[256];

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    loggedMenuCli(sockfd,myName);
}

void getGroupMessagesClie(int sockfd) {
    char buffer[256];

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru
    loggedMenuCli(sockfd,myName);
}
