#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int server(int argc, char *argv[])
{
    int sockfd, newsockfd;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    char buffer[256];

    if (argc < 2)
    {
        fprintf(stderr,"usage %s port\n", argv[0]);
        return 1;
    }

    bzero((char*)&serv_addr, sizeof(serv_addr)); //vynuluje strukuturu servaddr
    serv_addr.sin_family = AF_INET; // cely internet
    serv_addr.sin_addr.s_addr = INADDR_ANY; // povolene ip adresy - teraz vsetky
    serv_addr.sin_port = htons(atoi(argv[1])); //nastavi port litle to big endian

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // vytvori socket
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return 1;
    }

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) // na socket namapujem strukturu (tento socket bude pracovat so spojeniami z celeho internetu na tomto porte)
    {
        perror("Error binding socket address");
        return 2;
    }

    listen(sockfd, 5); //pasivny socket (nie na komunikaciu, ale na pripojenie pouzivatela) n:kolko klientov sa moze pripojit v jeden moment
    cli_len = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len); //blokujuce systemove volanie, ked sa niekto pripoji, vrati novy socket na komunikaciu s pripojenym klientom
    if (newsockfd < 0)
    {
        perror("ERROR on accept");
        return 3;
    }


    //--------------------------------jadro aplikacie--------------------------------------------------------------------



    bzero(buffer,256); //vynulujem buffer
    n = read(newsockfd, buffer, 255); //precitam data zo socketu a ulozim do buffra, je to blokujuce volanie, cakam dokedy klient nezada spravu
    if (n < 0)
    {
        perror("Error reading from socket");
        return 4;
    }
    printf("Here is the message: %s\n", buffer);

    const char* msg = "I got your message";
    n = write(newsockfd, msg, strlen(msg)+1);
    if (n < 0)
    {
        perror("Error writing to socket");
        return 5;
    }

    //--------------------------------jadro aplikacie--------------------------------------------------------------------

    close(newsockfd); // uzatvaram az ked chcem ukoncit komunikaciu
    close(sockfd);

    return 0;
}