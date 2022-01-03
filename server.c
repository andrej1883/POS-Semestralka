#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "errors.h"
#include "serverHandler.h"

typedef struct{
    //struct user* fromUser;
    char fromUser[10];
    char text[10000];
    int  newMsg;
} message;

typedef struct{
    char fromUser[10];
    char toUser[10];
} fRequest;

typedef struct {
    char fUsername[10];
} friend;

typedef struct {
    char username[10];
    char passwd[10];
    int numMsg;
    message *messages[10];
    int online;
    int numFrd;
    friend *friendlist[50];
    int numReq;
    fRequest *requests[10];
} user;

//typedef struct {
//    user* friends[100];
//} friendlist;

user *users[10];
int numberUsers = 0;

void trimNL(char* arr, int length) {
    for (int i = 0; i < length; ++i) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void sendRequest(char * fromUser, char* toUser) {
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, toUser) == 0) {
            strcpy(users[i]->requests[users[i]->numReq]->toUser, toUser);
            strcpy(users[i]->requests[users[i]->numReq]->fromUser, fromUser);
            users[i]->numReq++;
        }
    }

}

void addFriend(char * usersName, char * friendsName) {
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, usersName) == 0) {
            strcpy(users[i]->friendlist[users[i]->numFrd]->fUsername, friendsName) ;
            users[i]->numFrd++;
        }
    }

}

void establishFriendship(char * friendOne, char * friendTwo) {
    addFriend(friendOne, friendTwo);
    addFriend(friendTwo, friendTwo);
}

void authServ(int newsockfd) {

    char buffer[10];
    char name[10];
    char psswd[10];
    int userFound = 0;

    bzero(buffer,10); //vynulujem buffer
    chScRErr(read(newsockfd, buffer, 10));
    trimNL(buffer,sizeof (buffer));
    strcpy(name,buffer);

    bzero(buffer,10); //vynulujem buffer
    chScRErr(read(newsockfd, buffer, 10));
    trimNL(buffer,sizeof (buffer));
    strcpy(psswd,buffer);

    for (int i = 0; i < numberUsers; ++i) {
        if((strcmp(users[i]->username,name) == 0) && (strcmp(users[i]->passwd,psswd) == 0)) {
            users[i]->online = 1;
            userFound = 1;
            break;
        }
    }
    if(userFound == 0) {
        const char* msg = "Login or password incorrect!";
        chScWErr(write(newsockfd, msg, strlen(msg)+1));

    }

    if (userFound == 1) {
        const char* msg = "User sucesfully logged in";
        chScWErr(write(newsockfd, msg, strlen(msg)+1));
        loggedMenuServ(newsockfd);
    }
}

void updateAccountsLoad() {
    FILE *filePointer ;
    char line[50];

    if( access( "users.txt", F_OK ) == 0 ) {
        filePointer = fopen("users.txt", "r") ;
        while( fgets ( line, 50, filePointer ) != NULL )
        {
            char name[10], psswd[10];
            sscanf(line, "%s %s", name, psswd);
            trimNL(name,sizeof(name));
            trimNL(psswd,sizeof(psswd));
            if(numberUsers < 10) {
                user *new = (user *) malloc(sizeof (user));
                strcpy(new->username, name);
                strcpy(new->passwd,psswd);
                users[numberUsers] = new;
            }
            numberUsers++;
        }
        fclose(filePointer);
    } else {
        printf("Users file not found!\n");
    }
}

void updateAccountsSave() {
    FILE *filePointer;
    filePointer = fopen("users.txt", "w");

    for (int i = 0; i < numberUsers; ++i) {
        fputs(users[i]->username,filePointer);
        fputs(" ", filePointer);
        fputs(users[i]->passwd,filePointer);
        fputs("\n", filePointer);
    }

    fclose(filePointer);
}

void addMessage( char* toUserName, char* text, char* fromUserName)
{
    user * toUser = (user*) malloc(sizeof (user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, toUserName) == 0){
            toUser = users[i];
        }
    }

    user * fromUser = (user*) malloc(sizeof (user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, fromUserName) == 0){
            fromUser = users[i];
        }
    }

    message *newMessage = (message *) malloc(sizeof (message));
    newMessage->newMsg = 1;
    strcpy(newMessage->text,text);
    strcpy(newMessage->fromUser, fromUser->username);
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, toUser->username) == 0) {
        //if (users[i]->username == toUser->username) {
            users[i]->messages[users[i]->numMsg] = newMessage;
            users[i]->numMsg++;
            return;
        }
    }

}

void getMessages(int newsockfd, char* msgOfUser) {
    char buffer[256];
    user *newUser = (user*) malloc(sizeof (user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, msgOfUser) == 0){
            newUser = users[i];
        }
    }
    printf("Here are messages for you: \n");
    bzero(buffer,256);
    strcpy(buffer, "Here are messages for you: \n");
    //chScWErr(write(newsockfd, buffer, strlen(buffer)+1));
    for (int i = 0; i < newUser->numMsg; ++i) {
        strcat(buffer, newUser->messages[i]->text);
    }
    printf("%s\n", buffer);
    chScWErr(write(newsockfd, buffer, strlen(buffer)+1));


}

void getMessagesFrom(int newsockfd, char* msgOfUser, char* msgFromUser) {
    char buffer[256];
    user *newUser = (user*) malloc(sizeof (user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, msgOfUser) == 0){
            newUser = users[i];
        }
    }
    user *senderUser = (user*) malloc(sizeof (user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, msgFromUser) == 0){
            senderUser = users[i];
        }
    }
    int numOfMsgFromUser = 0;
    message *usersMessages[newUser->numMsg];
    for (int i = 0; i < newUser->numMsg; ++i) {
        if (strcmp(newUser->messages[i]->fromUser, senderUser->username) == 0) {
            usersMessages[numOfMsgFromUser]= newUser->messages[i];
            numOfMsgFromUser++;
        }
    }

    bzero(buffer,256);
    strcpy(buffer, "Here are messages for you from " );
    strcat(buffer, senderUser->username);
    strcat(buffer, ": \n");
    printf("Here are messages for you from %s: \n", buffer);

    for (int i = 0; i < numOfMsgFromUser; ++i) {

        strcat(buffer, usersMessages[i]->text);
    }
    printf("%s\n", buffer);
    chScWErr(write(newsockfd, buffer, strlen(buffer)+1));
}

void registerUser(int newsockfd) {
    user *new = (user *) malloc(sizeof (user));
    char buffer[10];

    bzero(buffer,10); //vynulujem buffer
    chScRErr(read(newsockfd, buffer, 10));
    trimNL(buffer,sizeof (buffer));
    strcpy(new->username, buffer);

    bzero(buffer,10); //vynulujem buffer
    chScRErr(read(newsockfd, buffer, 10));
    trimNL(buffer,sizeof (buffer));
    strcpy(new->passwd,buffer);

    for (int i = 0; i < 10; ++i) {
        if(!users[i]) {
            users[i] = new;
            numberUsers++;
            updateAccountsSave();
            break;
        }
    }
    printf("New user: %s\n",new->username);
    printf("Current users: \n");
    for (int i = 0; i < numberUsers; ++i) {
        printf("%s\n",users[i]->username);
    }
    const char* msg = "User sucesfully registered";
    chScWErr(write(newsockfd, msg, strlen(msg)+1));
}

void deleteUser(char* name) {
    for (int i = 0; i < numberUsers; ++i) {
        if(strcmp(users[i]->username,name) == 0) {
            users[i] = NULL;
            numberUsers--;
            updateAccountsSave();
        }
    }
}


int server(int argc, char *argv[])
{
    int sockfd, newsockfd;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
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

    chScCRErr(sockfd = socket(AF_INET, SOCK_STREAM, 0)); // vytvori socket
    chScBDErr(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))); // na socket namapujem strukturu (tento socket bude pracovat so spojeniami z celeho internetu na tomto porte)


    listen(sockfd, 5); //pasivny socket (nie na komunikaciu, ale na pripojenie pouzivatela) n:kolko klientov sa moze pripojit v jeden moment
    cli_len = sizeof(cli_addr);

    chScACErr(newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len)); //blokujuce systemove volanie, ked sa niekto pripoji, vrati novy socket na komunikaciu s pripojenym klientom


    //--------------------------------jadro aplikacie--------------------------------------------------------------------
    signal(SIGPIPE, SIG_IGN);
    updateAccountsLoad();
    //deleteUser("Pepa");
    welcomeServ(newsockfd);
    //authServ(newsockfd);
    //registerUser(newsockfd);
    //loggedMenuServ(newsockfd);


    for (;;) {
        bzero(buffer,256); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 255)); //precitam data zo socketu a ulozim do buffra, je to blokujuce volanie, cakam dokedy klient nezada spravu

        addMessage("Jarko", buffer, "Jarko");
        //addFriend("Jarko","Jarko");
        printf("Here is the message: %s\n", buffer);
        //getMessages(users[0]);
        if(strcmp(buffer,"exit") == 0) {
            break;
        }
        const char* msg = "I got your message";
        //getMessages(newsockfd, "Jarko");
        //getMessagesFrom(newsockfd, "Jarko", "Jarko");
        chScWErr(write(newsockfd, msg, strlen(msg)+1));
    }

    //--------------------------------jadro aplikacie--------------------------------------------------------------------

    close(newsockfd); // uzatvaram az ked chcem ukoncit komunikaciu
    close(sockfd);

    return 0;
}