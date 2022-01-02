#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int client(int argc, char *argv[])
{
    int sockfd, n;
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

    sockfd = socket(AF_INET, SOCK_STREAM, 0); //vytvorim tcp internetovy socket
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return 3;
    }

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) // cez vytvoreny socket sa pripojim (blokujuce volanie, caka kym sa pripojim na server)
    {
        perror("Error connecting to socket");
        return 4;
    }


    //--------------------------------jadro aplikacie--------------------------------------------------------------------
    printf("Please enter a message: ");
    bzero(buffer,256); //vynulujem buffer
    fgets(buffer, 255, stdin); //naplnim buffer

    n = write(sockfd, buffer, strlen(buffer)); //zapisem buffer na server
    if (n < 0)
    {
        perror("Error writing to socket");
        return 5;
    }

    bzero(buffer,256); //vynulujem buffer
    n = read(sockfd, buffer, 255); //precitam spravu zo servera
    if (n < 0)
    {
        perror("Error reading from socket");
        return 6;
    }

    printf("%s\n",buffer); //vypisem spravu od serveru


    //--------------------------------jadro aplikacie--------------------------------------------------------------------




    close(sockfd); //uzavriem socket

    return 0;
}