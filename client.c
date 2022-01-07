#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
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
    trimNL(myName,sizeof (myName));

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
        loggedMenuCli(sockfd);
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
        loggedMenuCli(sockfd);
    }
}

void getMessagesClie(int sockfd) { //read messages on server
    char buffer[256];
    int n;

    bzero(buffer, 256); //vynulujem buffer
    //chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera

    n = recv(sockfd, buffer, 255, MSG_WAITALL);
    if(n < 0){
        perror("Receive name Error:");
    }
    printf("%s\n", buffer); //vypisem spravu od serveru

    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    //chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server
    send(sockfd,buffer,255,MSG_EOR);

    bzero(buffer, 256); //vynulujem buffer
    //chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera

    n = recv(sockfd, buffer,255, MSG_WAITALL);
    if(n < 0){
        perror("Receive name Error:");
    }
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
    char buffer[255];
    int n;

    printf("Create account: \n");
    printf("Please enter username: ");

    bzero(buffer, 255); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    //chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server
    n = send(sockfd,buffer,10,MSG_EOR);
    if(n < 0){
        perror("Send option Error:");
    }
    for (int i = 0; i < 10; ++i) {
        myName[i] = buffer[i];
    }
    trimNL(myName,sizeof (myName));

    printf("Please enter password: ");
    bzero(buffer, 255); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    //chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server
    n = send(sockfd,buffer,10,MSG_EOR);
    if(n < 0){
        perror("Send option Error:");
    }


    bzero(buffer, 255); //vynulujem buffer
    //chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    n = recv(sockfd,buffer,255,MSG_WAITALL);
    if(n < 0){
        perror("Receive option Error:");
    }

    printf("%s\n", buffer); //vypisem spravu od serveru

    if(strcmp(buffer,"User sucesfully registered") == 0) {
        loggedMenuCli(sockfd);
    }
}

void sendFileInfoCLie(int sockfd, char *filename, char *toUser) {
    //funguje
    char buffer[256];

    bzero(buffer,sizeof (buffer));
    strcat(buffer,filename);
    strcat(buffer," ");
    strcat(buffer,myName);
    strcat(buffer," ");
    strcat(buffer,toUser);
    //chScWErr(write(sockfd, buffer, sizeof (buffer)));
    send(sockfd,buffer,256,MSG_EOR);
    bzero(buffer,sizeof (buffer));
}

void sendFileClie(char* filename,int sockfd, char* toUser) {
    //funguej
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
        chSFErr(send(sockfd,buffer,2048,0));
        bzero(data, 1024);
        fclose(filePointer);
    } else {
        printf("File not found!\n");
    }
}

void rcvFileCli(int sockfd) {
    //send your name  to server
    char buffer[256];
    int n;

    //strcpy(myName, "Pepa");
    /*//strcpy(myName, "Pepa");
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, myName);
    //chScWErr(write(sockfd, buffer, sizeof(buffer)));
    send(sockfd,buffer,10,MSG_EOR);*/


    bzero(buffer, sizeof (buffer)); //vynulujem buffer
    //chScRErr(read(sockfd, buffer, sizeof (buffer))); //precitam spravu zo servera
    n = recv(sockfd, buffer, sizeof(buffer), MSG_WAITALL);
    if(n < 0){
        perror("Receive name Error:");
    }
    printf("%s\n", buffer); //vypisem spravu od serveru

    if (strcmp(buffer, "There are no available files for you\n") != 0) {

        printf("Please enter number of file: ");
        bzero(buffer, sizeof (buffer)); //vynulujem buffer
        fgets(buffer, sizeof (buffer), stdin); //naplnim buffer
        //chScWErr(write(sockfd, buffer, sizeof (buffer)));
        send(sockfd,buffer,256,MSG_EOR);

        int n;
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

        bzero(buffer, sizeof (buffer)); //vynulujem buffer
        //chScRErr(read(sockfd, buffer,sizeof (buffer))); //precitam spravu zo servera
        n = recv(sockfd, buffer, 20, MSG_WAITALL);
        if(n < 0){
            perror("Receive name Error:");
        }
        strcpy(filename, buffer);

        strcat(directory, filename);

        bzero(fileBuffer, sizeof(fileBuffer));
        filepointer = fopen(directory, "w");
        /*while (1) {
            n = recv(sockfd, fileBuffer, 1024, 0);
            if (n <= 0) {
                break;
            }
            fprintf(filepointer, "%s", fileBuffer);
            bzero(fileBuffer, 1024);
        }*/
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
    // Set the socket I/O mode: In this case FIONBIO
    // enables or disables the blocking mode for the
    // socket based on the numerical value of iMode.
    // If iMode = 0, blocking is enabled;
    // If iMode != 0, non-blocking mode is enabled.
    ioctl(sockfd, FIONBIO, 0);

    //signal(SIGPIPE, SIG_IGN);

    //authClie(sockfd);
    //registerClie(sockfd);
    //loggedMenuCli(sockfd,"Lojzik");
    //sendFileClie("file.txt", sockfd,"Pepa");
    //sendFileInfoCLie(sockfd,"file.txt","Pepas");




    welcomeCli(sockfd);
    //rcvFileCli(sockfd);
    exit(0);

    for (;;) {
        printf("Please enter a message: ");
        bzero(buffer, 256); //vynulujem buffer
        fgets(buffer, 255, stdin); //naplnim buffer
        if (strcmp(buffer, "exit") == 0) {
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

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    loggedMenuCli(sockfd);
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


    if((strcmp(buffer,"Friend removed!\n") == 0) || (strcmp(buffer,"You can add friends in following menu  \n") == 0)) {
        loggedMenuCli(sockfd);
    }
}

void backTologMenu(int sockfd) {
    loggedMenuCli(sockfd);
}

void sendMessageClie(int sockfd) {
    char buffer[256];

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    //printf("Please enter number of user you wish to message: ");
    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    //printf("Please enter text: ");
    bzero(buffer, 256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer
    chScWErr(write(sockfd, buffer, strlen(buffer))); //zapisem buffer na server

    bzero(buffer, 256); //vynulujem buffer
    chScRErr(read(sockfd, buffer, 255)); //precitam spravu zo servera
    printf("%s\n", buffer); //vypisem spravu od serveru

    loggedMenuCli(sockfd);
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

    loggedMenuCli(sockfd);
}

void removeMemberClie(int sockfd) {
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

    loggedMenuCli(sockfd);
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

    loggedMenuCli(sockfd);
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

    loggedMenuCli(sockfd);
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

    loggedMenuCli(sockfd);
}
