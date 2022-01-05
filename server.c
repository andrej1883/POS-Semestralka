#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "errors.h"
#include "serverHandler.h"
#include "server.h"

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
    char chatName[10];
    int numMemb;
    int numMsg;
    friend *members[50];
    message *messages[100];
}groupChat;

typedef struct {
    char username[10];
    char passwd[10];
    int numMsg;
    message *messages[10];
    int online;
    int numFrd;
    friend *friendlist[50];
    int numReq;
    int numGroups;
    fRequest *requests[10];
    groupChat *groupChats[10];
} user;

typedef struct {
    char fromUser[10];
    char toUser[10];
    int fileIDI;
    char fileName[256];
} fileInfo;

fileInfo *fileList[9999];
user *users[10];
groupChat *groupChats[10];
int numberChats = 0;
int numberUsers = 0;
int fileIdS = 0;
int filesCount = 0;

void trimNL(char* arr, int length) {
    for (int i = 0; i < length; ++i) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void addFriend(int newsockfd, char* username) {
    /* vypise vsetkych pouzivatelov
     * user zvoli ktoreho si chce pridat
     * tomu sa posle request
     * */


    user * client = (user*) malloc(sizeof (user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, username) == 0) {
            client = users[i];
        }
    }
    char buffer[256];
    bzero(buffer,256);
    strcpy(buffer, "Here is list of all users: \n");
    for (int i = 0; i < numberUsers; ++i) {
        int val = i;
        char sid[3];
        sprintf(sid,"%i",val);
        strcat(buffer, sid);
        strcat(buffer, ". ");
        strcat(buffer, users[i]->username);
        strcat(buffer, "\n");
    }
    chScWErr(write(newsockfd, buffer, strlen(buffer)+1));

    bzero(buffer,256); //vynulujem buffer
    chScRErr(read(newsockfd, buffer, 256));

    int test;
    sscanf(buffer, "%d", &test);
    if ((test >= 0) ) {
        sendRequest(username, users[test]->username);

    }
    //trimNL(buffer,sizeof (buffer));
    //strcpy(choise, buffer);
    const char* msg = "Friend request sent!";
    chScWErr(write(newsockfd, msg, strlen(msg)+1));
    loggedMenuServ(newsockfd);

}

void sendRequest(char * fromUser, char* toUser) {
    fRequest *newRequest = (fRequest *) malloc(sizeof (fRequest));
    strcpy(newRequest->fromUser, fromUser);
    strcpy(newRequest->toUser, toUser);
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, toUser) == 0) {
            users[i]->requests[users[i]->numReq] = newRequest;
            users[i]->numReq++;
        }
    }

}

void updateFriendlist(char * usersName, char * friendsName) {
    friend *newFriend = (friend *) malloc(sizeof (friend));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, usersName) == 0) {
            strcpy(newFriend->fUsername, friendsName);
            users[i]->friendlist[users[i]->numFrd] = newFriend;
            users[i]->numFrd++;
        }
    }
}

void establishFriendship(char * friendOne, char * friendTwo) {
    updateFriendlist(friendOne, friendTwo);
    updateFriendlist(friendTwo, friendOne);
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
        welcomeServ(newsockfd);
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
                numberUsers++;
            }
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

    msgMenuServ(newsockfd, msgOfUser);
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
    strcpy(buffer, "Here are messages for you from chosen user" );
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
    loggedMenuServ(newsockfd);
}

void deleteUser(char* name) {
    for (int i = 0; i < numberUsers; ++i) {
        if(strcmp(users[i]->username,name) == 0) {
            printf("User %s deleted\n",users[i]->username);
            users[i] = NULL;
            for (int j = i; j < numberUsers - 1; ++j) {
                users[j] = users[j+1];
            }
            numberUsers--;
            updateAccountsSave();
        }
    }

}

void sendFileServ(char* filename,int newsockfd) {
    //TODO 1 : Send from server to specific user
    /*FILE *filePointer;
    char data[1024] = {0};
    char buffer[256];


    if( access( filename, F_OK ) == 0 ) {
        filePointer = fopen(filename, "r") ;
        while( fgets ( data, 1024, filePointer ) != NULL )
        {
            chSFErr(send(newsockfd,data,sizeof (data),0));
        }
        bzero(data, 1024);
        fclose(filePointer);
    } else {
        printf("File not found!\n");
    }*/
}

void getFileInfoServ(int newsockfd) {
    char buffer[256];
    char filename[256];
    char from[10];
    char to[10];

    fileInfo *new = (fileInfo *) malloc(sizeof (fileInfo));
    new->fileIDI = fileIdS;

    bzero(buffer,256);
    chScRErr(read(newsockfd, buffer, 256));
    sscanf(buffer, "%s %s %s", new->fileName, new->fromUser,new->toUser);

    for (int i = 0; i < filesCount; ++i) {
        if(!fileList[i]){
            fileList[i] = new;
        }
    }
}

void rcvFileServ(int newsockfd) {
    int n;
    FILE *filepointer;
    char buffer[1024];
    char directory[100] = "files/";
    char type[5] = ".fl";
    char sId[10];

    struct stat st = {0};
    if (stat(directory, &st) == -1) {
        mkdir(directory, 0700);
    }

    sprintf(sId,"%i",fileIdS);
    strcat(directory,sId);
    strcat(directory,type);

    filesCount++;
    getFileInfoServ(newsockfd);

    filepointer = fopen(directory, "w");
    while (1) {
        n = recv(newsockfd, buffer, 1024, 0);
        if (n <= 0){
            break;
        }
        fprintf(filepointer, "%s", buffer);
        bzero(buffer, 1024);
    }
    fileIdS++;
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
    //signal(SIGPIPE, SIG_IGN);
    //deleteUser("Pepa");
    //authServ(newsockfd);
    //registerUser(newsockfd);
    //loggedMenuServ(newsockfd);
    //rcvFileServ(newsockfd);
    //getFileInfoServ(newsockfd);

    updateAccountsLoad();
    welcomeServ(newsockfd);
    exit(0);

    for (;;) {
        bzero(buffer,256); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 255)); //precitam data zo socketu a ulozim do buffra, je to blokujuce volanie, cakam dokedy klient nezada spravu

        //addMessage("Jarko", buffer, "Jarko");
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

void manageRequests(int newsockfd, char *username) {
    /*
     * dostane meno usera. Requesty pre tohto usera ideme spravovat
     * Vypiseme vsetky nevybavene requesty
     * user vyberie request, dostane moznost potvrdit alebo zamietnut a rozhodne sa
     * ak potvrdi vytvori sa priatelstvo ak zamietne zmiznme request
     * pole requestov sa musi preusporiadat aby dalej dobre fungovalo
     *
     *
     * */

    char buffer[256];
    user *managingUser = (user*) malloc(sizeof (user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, username) == 0){
            managingUser = users[i];
        }
    }

    if (managingUser->numReq == 0){
        bzero(buffer,256);
        strcpy(buffer, "No friend requests \n Press 0 to continue");
        chScWErr(write(newsockfd, buffer, strlen(buffer)+1));

        bzero(buffer,256); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 256));

        bzero(buffer,256);
        strcpy(buffer, "You should add friends in following menu  \n Press 0 to continue");
        chScWErr(write(newsockfd, buffer, strlen(buffer)+1));

        bzero(buffer,256); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 256));
    } else {
        bzero(buffer,256);
        strcpy(buffer, "Here is list of people who want to be your friends: \n");
        for (int i = 0; i < managingUser->numReq; ++i) {
            int val = i;
            char sid[3];
            sprintf(sid,"%i",val);
            strcat(buffer, sid);
            strcat(buffer, ". ");
            strcat(buffer, managingUser->requests[i]->fromUser);
            strcat(buffer, "\n");
        }
        strcat(buffer, "Please enter number of request: \n");

        chScWErr(write(newsockfd, buffer, strlen(buffer)+1));

        bzero(buffer,256); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 256));

        int chosenReq;
        sscanf(buffer, "%d", &chosenReq);
        if ((chosenReq >= 0) && (chosenReq < managingUser->numReq)) {
            bzero(buffer,256);
            strcpy(buffer, "Do you wish to accept this request? \n(Y - Yes/N -NO ) \n");
            chScWErr(write(newsockfd, buffer, strlen(buffer)+1));

            bzero(buffer,256); //vynulujem buffer
            chScRErr(read(newsockfd, buffer, 256));
            char choice = buffer[0];
            //strcpy(choise, buffer);
            //if ((strcmp(choise, 'Y') == 0) || (strcmp(choise, 'y') == 0)) {
            if ((choice == 'Y') || (choice== 'y')) {
                establishFriendship(username, managingUser->requests[chosenReq]->fromUser);
                managingUser->numReq--;

                for (int i = 0; i < managingUser->numReq; ++i) {
                    if (i>chosenReq) {
                        fRequest *newRequest = (fRequest *) malloc(sizeof (fRequest));
                        strcpy(newRequest->fromUser, managingUser->requests[i+1]->fromUser);
                        strcpy(newRequest->toUser, managingUser->requests[i+1]->toUser);
                        managingUser->requests[i] = newRequest;
                    }
                }
            } else {
                if ((choice == 'N') || (choice== 'n')) {
                    managingUser->numReq--;

                    for (int i = 0; i < managingUser->numReq; ++i) {
                        if (i>chosenReq) {
                            fRequest *newRequest = (fRequest *) malloc(sizeof (fRequest));
                            strcpy(newRequest->fromUser, managingUser->requests[i+1]->fromUser);
                            strcpy(newRequest->toUser, managingUser->requests[i+1]->toUser);
                            managingUser->requests[i] = newRequest;
                        }
                    }
                }
            }
        } else {
            strcpy(buffer, "Wrong value\n");
            chScWErr(write(newsockfd, buffer, strlen(buffer)+1));
        }
    }

    loggedMenuServ(newsockfd);
}

void removeFriend(int newsockfd, char *username) {
    /*  userovi sa ukazu jeho priatelia
     * on si jedneho vyberie
     * toto priatelstvo sa zrusi
     * */
    char buffer[256];
    user *managingUser = (user*) malloc(sizeof (user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, username) == 0){
            managingUser = users[i];
        }
    }

    if (managingUser->numFrd !=0) {
        bzero(buffer,256);
        strcpy(buffer, "Choose friend (by number) to be removed: \n");

        for (int i = 0; i < managingUser->numFrd; ++i) {
            int val = i;
            char sid[3];
            sprintf(sid,"%i",val);
            strcat(buffer, sid);
            strcat(buffer, ". ");
            strcat(buffer, managingUser->friendlist[i]->fUsername);
            strcat(buffer, "\n");
        }

        chScWErr(write(newsockfd, buffer, strlen(buffer)+1));

        bzero(buffer,256); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 256));

        int chosenFrd;
        sscanf(buffer, "%d", &chosenFrd);

        user *removedFriend = (user*) malloc(sizeof (user));
        for (int i = 0; i < numberUsers; ++i) {
            if (strcmp(users[i]->username, managingUser->friendlist[chosenFrd]->fUsername) == 0) {
                removedFriend = users[i];
            }
        }

        managingUser->numFrd--;
        for (int i = 0; i < managingUser->numFrd; ++i) {
            if (i>chosenFrd) {
                friend *newFriend = (friend *) malloc(sizeof (friend));
                strcpy(newFriend->fUsername, managingUser->friendlist[i+1]->fUsername);
                managingUser->friendlist[i] = newFriend;
            }
        }

        int selected;
        for (int i = 0; i < removedFriend->numFrd; ++i) {
            if (strcmp(removedFriend->friendlist[i]->fUsername, managingUser->username) == 0) {
                selected = i;
            }
        }

        removedFriend->numFrd--;

        for (int i = 0; i < removedFriend->numFrd; ++i) {
            if (i>selected) {
                friend *newFriend = (friend *) malloc(sizeof (friend));
                strcpy(newFriend->fUsername, removedFriend->friendlist[i+1]->fUsername);
                removedFriend->friendlist[i] = newFriend;
            }
        }
        bzero(buffer,256);
        strcpy(buffer, "Friend removed!\n");
        chScWErr(write(newsockfd, buffer, strlen(buffer)+1));

    } else {
        bzero(buffer,256);
        strcpy(buffer, "You have no friends :( \n Press 0 to continue");
        chScWErr(write(newsockfd, buffer, strlen(buffer)+1));

        bzero(buffer,256); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 256));

        bzero(buffer,256);
        strcpy(buffer, "You can add friends in following menu  \n");
        chScWErr(write(newsockfd, buffer, strlen(buffer)+1));
    }

    loggedMenuServ(newsockfd);

}

void sendMessage(int newsockfd, char *username) {
    /* ponukne userovi jeho kontakty na vyber
     * po vybere umozni napisat spravu
     * ulozi spravu prislusnemu kontaktu
     */
    char buffer[256];
    user *managingUser = (user*) malloc(sizeof (user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, username) == 0){
            managingUser = users[i];
        }
    }

    if (managingUser->numFrd !=0) {
        bzero(buffer, 256);
        strcpy(buffer, "Choose friend: \n");

        for (int i = 0; i < managingUser->numFrd; ++i) {
            int val = i;
            char sid[3];
            sprintf(sid,"%i",val);
            strcat(buffer, sid);
            strcat(buffer, ". ");
            strcat(buffer, managingUser->friendlist[i]->fUsername);
            strcat(buffer, "\n");
        }

        chScWErr(write(newsockfd, buffer, strlen(buffer) + 1));

        bzero(buffer, 256); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 256));

        int chosenFrd;
        sscanf(buffer, "%d", &chosenFrd);

        user *textedFriend = (user*) malloc(sizeof (user));
        for (int i = 0; i < numberUsers; ++i) {
            if (strcmp(users[i]->username, managingUser->friendlist[chosenFrd]->fUsername) == 0) {
                textedFriend = users[i];
            }
        }

        bzero(buffer, 256);
        strcpy(buffer, "Write message: \n");
        chScWErr(write(newsockfd, buffer, strlen(buffer) + 1));

        bzero(buffer, 256); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 256));
        addMessage(textedFriend->username, buffer, managingUser->username);

        bzero(buffer,256);
        strcpy(buffer, "Message sent! \n");
        chScWErr(write(newsockfd, buffer, strlen(buffer)+1));

    }else {
        bzero(buffer,256);
        strcpy(buffer, "You have no friends :( \n Press 0 to continue");
        chScWErr(write(newsockfd, buffer, strlen(buffer)+1));

        bzero(buffer,256); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 256));

        bzero(buffer,256);
        strcpy(buffer, "You can add friends in following menu  \n");
        chScWErr(write(newsockfd, buffer, strlen(buffer)+1));

    }
    loggedMenuServ(newsockfd);
}

void readMessages(int newsockfd, char *username) {
    char buffer[256];
    user *managingUser = (user*) malloc(sizeof (user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, username) == 0){
            managingUser = users[i];
        }
    }

    if (managingUser->numFrd !=0) {
        bzero(buffer, 256);
        strcpy(buffer, "Choose friend: \n");

        for (int i = 0; i < managingUser->numFrd; ++i) {
            int val = i;
            char sid[3];
            sprintf(sid,"%i",val);
            strcat(buffer, sid);
            strcat(buffer, ". ");
            strcat(buffer, managingUser->friendlist[i]->fUsername);
            strcat(buffer, "\n");
        }

        chScWErr(write(newsockfd, buffer, strlen(buffer) + 1));

        bzero(buffer, 256); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 256));

        int chosenFrd;
        sscanf(buffer, "%d", &chosenFrd);

        user *textedFriend = (user*) malloc(sizeof (user));
        for (int i = 0; i < numberUsers; ++i) {
            if (strcmp(users[i]->username, managingUser->friendlist[chosenFrd]->fUsername) == 0) {
                textedFriend = users[i];
            }
        }

        getMessagesFrom(newsockfd, managingUser->username, textedFriend->username);


    }else {
        bzero(buffer,256);
        strcpy(buffer, "You have no friends so no messages :( \n Press 0 to continue");
        chScWErr(write(newsockfd, buffer, strlen(buffer)+1));

        bzero(buffer,256); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, 256));

        bzero(buffer,256);
        strcpy(buffer, "You can add friends in following menu  \n");
        chScWErr(write(newsockfd, buffer, strlen(buffer)+1));
    }

    loggedMenuServ(newsockfd);

}

void createGroup(char *nameofGroup, char *founderName) {
    groupChat *newGroup = (groupChat *) malloc(sizeof (groupChat));
    strcpy(newGroup->chatName, nameofGroup);
    friend *newMember = (friend *) malloc(sizeof (friend));
    strcpy(newMember->fUsername,founderName);
    newGroup->members[0] = newMember;
    newGroup->numMemb = 1;
    newGroup->numMsg = 0;
    groupChats[numberChats] = newGroup;
    numberChats++;

    user *founder = (user*) malloc(sizeof (user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, founderName) == 0){
            founder = users[i];
        }
    }
    founder->groupChats[founder->numGroups]=newGroup;
    founder->numGroups++;
}

void addMember(char *nameofGroup, char *membersName) {
    groupChat *group = (groupChat *) malloc(sizeof (groupChat));
    for (int i = 0; i < numberChats; ++i) {
        if (strcmp(groupChats[i]->chatName, nameofGroup) == 0) {
            group = groupChats[i];
        }
    }


    friend *newMember = (friend *) malloc(sizeof (friend));
    strcpy(newMember->fUsername,membersName);
    group->members[group->numMemb] = newMember;
    group->numMemb++;

    user * newMemberU = (user*) malloc(sizeof (user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, membersName) == 0) {
            newMemberU = users[i];
        }
    }

    newMemberU->groupChats[newMemberU->numGroups] = group;
    newMemberU->numGroups++;

}

void removeMember(char *nameofGroup, char *membersName) {

    groupChat *group = (groupChat *) malloc(sizeof (groupChat));
    for (int i = 0; i < numberChats; ++i) {
        if (strcmp(groupChats[i]->chatName, nameofGroup) == 0) {
            group = groupChats[i];
        }
    }

    user *removedMember = (user*) malloc(sizeof (user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, membersName) == 0) {
            removedMember = users[i];
        }
    }

    int chosen;
    for (int i = 0; i < group->numMemb; ++i) {
        if (strcmp(group->members[i]->fUsername, membersName) == 0) {
            chosen == i;
        }
    }
    group->numMemb--;

    for (int i = 0; i < group->numMemb; ++i) {
        if (i >= chosen) {
            strcpy(group->members[i]->fUsername , group->members[i+1]->fUsername );
        }
    }

    int chosengr;
    for (int i = 0; i < removedMember->numGroups; ++i) {
        if (strcmp(removedMember->groupChats[i]->chatName, nameofGroup) == 0) {
            chosengr == i;
        }
    }
    removedMember->numGroups--;

    //groupChat *helperGroup = (groupChat *) malloc(sizeof (groupChat));
    for (int i = 0; i < removedMember->numGroups; ++i) {
        if (i >= chosengr) {
            removedMember->groupChats[i] = removedMember->groupChats[i+1];
        }
    }

}
