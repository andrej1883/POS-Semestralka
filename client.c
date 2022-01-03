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

 char* myName[10];

void authClie(int sockfd) {
    char buffer[256];

    printf("Log into server: \n");
    printf("Please enter username: ");

    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server
    for (int i = 0; i < 10; ++i) {
        myName[i] = &buffer[i];
    }

    printf("My name is: %s\n", buffer);

    printf("Please enter password: ");
    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server


    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    loggedMenuCli(sockfd,myName);
}

void getMessagesClie(int sockfd) {
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


    printf("Please enter password: ");
    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server


    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru
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
    signal(SIGPIPE, SIG_IGN);
    //authClie(sockfd);
    //registerClie(sockfd);
    welcomeCli(sockfd);

    for(;;) {
        printf("Please enter a message: ");
        bzero(buffer, 256); //vynulujem buffer
        fgets(buffer, 255, stdin); //naplnim buffer
        if(strcmp(buffer,"exit") == 0) {
            break;
        }
        chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

        getMessagesClie(sockfd);
        bzero(buffer, 256); //vynulujem buffer
        chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera

        printf("%s\n", buffer); //vypisem spravu od serveru
    }
    //--------------------------------jadro aplikacie--------------------------------------------------------------------

    close(sockfd); //uzavriem socket

    return 0;
}