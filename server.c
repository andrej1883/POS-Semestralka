#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include "errors.h"
#include "serverHandler.h"
#include "server.h"

#define MSGBUFFSIZE 256
#define NUMBERUSERS 1024
#define NUMBERGROUPCHATS 1024
#define USERNAMEPASSWDLEN 10

typedef struct {
    char fromUser[10];
    char text[10000];
    int newMsg;
} message;

typedef struct {
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
} groupChat;

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
    char fileName[MSGBUFFSIZE];
} fileInfo;

fileInfo *fileList[NUMBERUSERS];
user *users[NUMBERUSERS];
groupChat *groupChats[NUMBERGROUPCHATS];
int numberChats = 0;
int numberUsers = 0;
int fileIdS = 0;
int filesCount = 0;
int clientCount = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int cancel = 0;
int idClient = 0;

typedef struct {

    int id;
    int sockID;
    struct sockaddr_in clientAddr;
    unsigned int len;
    char username[USERNAMEPASSWDLEN];
} client;

client *clients[NUMBERUSERS];
pthread_t thread[NUMBERUSERS];

int cligetID(int socktID) {
    for (int i = 0; i < clientCount; ++i) {
        if (clients[i]->sockID == socktID) {
            return clients[i]->id;
        }
    }
}

void* handleConnection(void *connClientInfo) {
    client *new = (client *) connClientInfo;
    int id = new->id;
    int clientSocket = new->sockID;

    printf("Client ID%d connected.\n", id);

    welcomeServ(clientSocket);
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < clientCount; ++i) {
        if (clients[i]->id == id) {
            printf("Client ID%d deleted from client list\n", id);
            free(clients[i]);
            clients[i] = NULL;
            for (int j = i; j < NUMBERUSERS - 1; ++j) {
                clients[j] = clients[j + 1];
            }
            clientCount--;
        }
    }
    pthread_mutex_unlock(&mutex);
    close(clientSocket);
    printf("Client ID%d disconnected.\n", id);
    pthread_join(thread[id], NULL);
    return NULL;
}

void trimNL(char *arr, int length) {
    for (int i = 0; i < length; ++i) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void setUsername(char *username, int sockfd) {
    for (int i = 0; i < clientCount; ++i) {
        if (clients[i]->id == cligetID(sockfd)) {
            strcpy(clients[i]->username, username);
        }
    }
}

char *getUsername(int sockfd) {
    for (int i = 0; i < clientCount; ++i) {
        if (clients[i]->id == cligetID(sockfd)) {
            return clients[i]->username;
        }
    }
    return NULL;
}

void addFriend(int newsockfd, char *username) {
    /* vypise vsetkych pouzivatelov
     * user zvoli ktoreho si chce pridat
     * tomu sa posle request
     * */
    friend *userList[NUMBERUSERS] = {0};
    int numOfusers = 0;
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, username) != 0) {
            friend *helperFriend = (friend *) malloc(sizeof(friend));
            strcpy(helperFriend->fUsername, users[i]->username);
            userList[numOfusers] = helperFriend;
            numOfusers++;
            helperFriend = NULL;
        }
    }


    user *client ;//= (user *) malloc(sizeof(user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, username) == 0) {
            client = users[i];
        }
    }

    friend *nonFriends[numOfusers + 1];
    int numOfNonFrd = 1;
    int decision;

    for (int i = 0; i < numOfusers; ++i) {
        decision = 1;
        for (int j = 0; j < client->numFrd; ++j) {
            if (strcmp(userList[i]->fUsername, client->friendlist[j]->fUsername) == 0) {
                decision = 0;
            }
        }
        if (decision == 1) {
            friend *helperFriend = (friend *) malloc(sizeof(friend));
            strcpy(helperFriend->fUsername, userList[i]->fUsername);
            nonFriends[numOfNonFrd] = helperFriend;
            numOfNonFrd++;
        }
    }

    for (int i = 0; i < numOfusers; ++i) {
        free(userList[i]);
    }

    char buffer[MSGBUFFSIZE];
    if (numOfNonFrd == 1) {

        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "It would seem that all users are your friends \n Press 0 to continue");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));


        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));


        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "Or maybe you are just only one \n Press 0 to contemplate your lonely existence");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));


    } else {

        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "Here is list of users you aren't friends with: \n");
        for (int i = 1; i < numOfNonFrd; ++i) {
            int val = i;
            char sid[3];
            sprintf(sid, "%i", val);
            strcat(buffer, sid);
            strcat(buffer, ". ");
            strcat(buffer, nonFriends[i]->fUsername);
            strcat(buffer, "\n");
        }
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        int test= -1;
        sscanf(buffer, "%d", &test);
        if ((test > 0) && (test <= numOfNonFrd)) {
            pthread_mutex_lock(&mutex);
            sendRequest(username, nonFriends[test]->fUsername);
            pthread_mutex_unlock(&mutex);
            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Friend request sent! \n" );
        } else {

            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Invalid input \n" );
        }
        //const char *msg = "Friend request sent!";
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
        for (int i = 1; i < numOfNonFrd; ++i) {
            free(nonFriends[i]);
        }
    }
    loggedMenuServ(newsockfd);
}

void sendRequest(char *fromUser, char *toUser) {
    fRequest *newRequest = (fRequest *) malloc(sizeof(fRequest));
    strcpy(newRequest->fromUser, fromUser);
    strcpy(newRequest->toUser, toUser);
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, toUser) == 0) {
            users[i]->requests[users[i]->numReq] = newRequest;
            users[i]->numReq++;
        }
    }
}

void updateFriendlist(char *usersName, char *friendsName) {
    friend *newFriend = (friend *) malloc(sizeof(friend));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, usersName) == 0) {
            strcpy(newFriend->fUsername, friendsName);
            free(users[i]->friendlist[users[i]->numFrd]);
            users[i]->friendlist[users[i]->numFrd] = newFriend;
            users[i]->numFrd++;
        }
    }
}

void establishFriendship(char *friendOne, char *friendTwo) {
    updateFriendlist(friendOne, friendTwo);
    updateFriendlist(friendTwo, friendOne);
}

void authServ(int newsockfd) {
    char buffer[MSGBUFFSIZE];
    char name[10];
    char psswd[10];
    int userFound = 0;

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));
    trimNL(buffer, MSGBUFFSIZE);
    strcpy(name, buffer);

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));
    trimNL(buffer, MSGBUFFSIZE);
    strcpy(psswd, buffer);

    pthread_mutex_lock(&mutex);
    for (int i = 0; i < numberUsers; ++i) {
        if ((strcmp(users[i]->username, name) == 0) && (strcmp(users[i]->passwd, psswd) == 0)) {
            users[i]->online = 1;
            userFound = 1;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    if (userFound == 0) {
        const char *msg = "Login or password incorrect!";
        chScWErr(write(newsockfd, msg, MSGBUFFSIZE));
        welcomeServ(newsockfd);
    }

    if (userFound == 1) {
        const char *msg = "User sucesfully logged in";
        chScWErr(write(newsockfd, msg, MSGBUFFSIZE));
        loggedMenuServ(newsockfd);
    }
}

void updateAccountsLoad() {
    FILE *filePointer;
    char line[50];

    if (access("users.txt", F_OK) == 0) {
        filePointer = fopen("users.txt", "r");
        while (fgets(line, 50, filePointer) != NULL) {
            char name[10], psswd[10];
            sscanf(line, "%s %s", name, psswd);
            trimNL(name, sizeof(name));
            trimNL(psswd, sizeof(psswd));
            if (numberUsers < 10) {
                user *new = (user *) malloc(sizeof(user));
                strcpy(new->username, name);
                strcpy(new->passwd, psswd);
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
        fputs(users[i]->username, filePointer);
        fputs(" ", filePointer);
        fputs(users[i]->passwd, filePointer);
        fputs("\n", filePointer);
    }

    fclose(filePointer);
}

void updateFileLogLoad() {
    FILE *filePointer;
    char line[50];

    if (access("file_log_count.txt", F_OK) == 0) {
        filePointer = fopen("file_log_count.txt", "r");
        while (fgets(line, 50, filePointer) != NULL) {
            int count, id;
            sscanf(line, "%d %d", &count, &id);
            if (filesCount < 9999) {
                filesCount = count;
                fileIdS = id;
            }
        }
        fclose(filePointer);
    } else {
        printf("file_log_count file not found!\n");
    }

    if (access("file_log.txt", F_OK) == 0) {
        filePointer = fopen("file_log.txt", "r");
        while (fgets(line, 50, filePointer) != NULL) {
            int id;
            char from[10], to[10], filename[MSGBUFFSIZE];
            sscanf(line, "%i %s" "%s %s", &id, from, to, filename);
            trimNL(from, sizeof(from));
            trimNL(to, sizeof(to));
            trimNL(filename, sizeof(filename));

            if (filesCount < 9999) {
                fileInfo *new = (fileInfo *) malloc(sizeof(fileInfo));
                strcpy(new->fileName, filename);
                strcpy(new->toUser, to);
                strcpy(new->fromUser, from);

                new->fileIDI = id;
                for (int i = 0; i < filesCount; ++i) {
                    if (!fileList[i]) {
                        fileList[i] = new;
                        break;
                    }
                }
            }
        }
        fclose(filePointer);
    } else {
        printf("file_log file not found!\n");
    }
}

void updateFileLogSave() {
    FILE *filePointer;
    remove("file_log_count.txt");
    filePointer = fopen("file_log_count.txt", "w+");

    char sId[10], SID[10];
    char counts[20];
    sprintf(sId, "%d", filesCount);
    strcpy(counts, sId);
    sprintf(SID, "%d", fileIdS);
    strcat(counts, " ");
    strcat(counts, SID);
    fputs(counts, filePointer);
    fclose(filePointer);

    bzero(sId, 10);

    FILE *filePointer2;
    remove("file_log.txt");
    filePointer2 = fopen("file_log.txt", "w");
    for (int i = 0; i < filesCount; ++i) {
        sprintf(sId, "%i", fileList[i]->fileIDI);
        fputs(sId, filePointer2);
        fputs(" ", filePointer2);
        fputs(fileList[i]->fromUser, filePointer2);
        fputs(" ", filePointer2);
        fputs(fileList[i]->toUser, filePointer2);
        fputs(" ", filePointer2);
        fputs(fileList[i]->fileName, filePointer2);
        fputs("\n", filePointer2);
    }
    fclose(filePointer2);
}

void addMessage(char *toUserName, char *text, char *fromUserName) {
    user *toUser ;//= (user *) malloc(sizeof(user));

    pthread_mutex_lock(&mutex);

    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, toUserName) == 0) {
            toUser = users[i];
        }
    }

    user *fromUser ;//= (user *) malloc(sizeof(user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, fromUserName) == 0) {
            fromUser = users[i];
        }
    }
    pthread_mutex_unlock(&mutex);

    message *newMessage = (message *) malloc(sizeof(message));
    newMessage->newMsg = 1;
    strcpy(newMessage->text, text);
    strcpy(newMessage->fromUser, fromUser->username);

    pthread_mutex_lock(&mutex);
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, toUser->username) == 0) {
            //if (users[i]->username == toUser->username) {
            users[i]->messages[users[i]->numMsg] = newMessage;
            users[i]->numMsg++;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
}

void getMessagesFrom(int newsockfd, char *msgOfUser, char *msgFromUser) {
    char buffer[MSGBUFFSIZE];
    user *newUser ;//= (user *) malloc(sizeof(user));

    pthread_mutex_lock(&mutex);

    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, msgOfUser) == 0) {
            newUser = users[i];
        }
    }
    user *senderUser;// = (user *) malloc(sizeof(user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, msgFromUser) == 0) {
            senderUser = users[i];
        }
    }
    pthread_mutex_unlock(&mutex);

    int numOfMsgFromUser = 0;
    message *usersMessages[newUser->numMsg];
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < newUser->numMsg; ++i) {
        if (strcmp(newUser->messages[i]->fromUser, senderUser->username) == 0) {
            usersMessages[numOfMsgFromUser] = newUser->messages[i];
            numOfMsgFromUser++;
        }
    }
    pthread_mutex_unlock(&mutex);

    bzero(buffer, MSGBUFFSIZE);
    strcpy(buffer, "Here are messages for you from ");
    strcat(buffer, senderUser->username);
    strcat(buffer, ": \n");


    for (int i = 0; i < numOfMsgFromUser; ++i) {

        strcat(buffer, usersMessages[i]->text);
    }
    printf("%s\n", buffer);

    send(newsockfd, buffer, MSGBUFFSIZE, MSG_EOR);

}

void registerUser(int newsockfd) {
    user *new = (user *) malloc(sizeof(user));
    char buffer[MSGBUFFSIZE];
    int n;

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    n = recv(newsockfd, buffer, MSGBUFFSIZE, MSG_WAITALL);
    if (n < 0) {
        perror("Receive option Error:");
    }

    trimNL(buffer, MSGBUFFSIZE);
    strcpy(new->username, buffer);

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    n = recv(newsockfd, buffer, MSGBUFFSIZE, MSG_WAITALL);
    if (n < 0) {
        perror("Receive option Error:");
    }

    trimNL(buffer, MSGBUFFSIZE);
    strcpy(new->passwd, buffer);
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < 10; ++i) {
        if (!users[i]) {
            users[i] = new;
            numberUsers++;
            updateAccountsSave();
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    printf("New user: %s\n", new->username);
    printf("Current users: \n");
    for (int i = 0; i < numberUsers; ++i) {
        printf("%s\n", users[i]->username);
    }
    const char *msg = "User sucesfully registered";
    n = send(newsockfd, msg, MSGBUFFSIZE, MSG_EOR);
    if (n < 0) {
        perror("Send option Error:");
    }

    loggedMenuServ(newsockfd);
}

void deleteUser(char *name) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, name) == 0) {
            printf("User %s deleted\n", users[i]->username);
            free(users[i]);
            users[i] = NULL;
            for (int j = i; j < numberUsers - 1; ++j) {
                users[j] = users[j + 1];
            }
            numberUsers--;
            updateAccountsSave();
        }
    }
    pthread_mutex_unlock(&mutex);
}

void sendFileServ(int newsockfd, char *current) {
    char buffer[MSGBUFFSIZE];
    char loaded[USERNAMEPASSWDLEN];

    //creating temporary fileList
    fileInfo *temporary[9999] = {0};
    pthread_mutex_lock(&mutex);
    int found = 0;
    for (int i = 0; i < filesCount; ++i) {
        strcpy(loaded, fileList[i]->toUser);
        if (strcmp(fileList[i]->toUser, current) == 0) {
            fileInfo *foundInfo = (fileInfo *) malloc(sizeof(fileInfo));
            strcpy(foundInfo->toUser, fileList[i]->toUser);
            strcpy(foundInfo->fromUser, fileList[i]->fromUser);
            strcpy(foundInfo->fileName, fileList[i]->fileName);
            foundInfo->fileIDI = fileList[i]->fileIDI;
            for (int j = 0; j < 9999; ++j) {
                if (!temporary[j]) {
                    temporary[j] = foundInfo;
                    break;
                }
            }
            found++;
        }
    }
    pthread_mutex_unlock(&mutex);

    if (found > 0) {

        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "Here are your files: \n");
        for (int i = 0; i < found; ++i) {
            char sid[4];
            sprintf(sid, "%i", i);
            strcat(buffer, sid);
            strcat(buffer, ". File: ");
            strcat(buffer, temporary[i]->fileName);
            strcat(buffer, " From: ");
            strcat(buffer, temporary[i]->fromUser);
            strcat(buffer, "\n");
        }

        send(newsockfd, buffer, MSGBUFFSIZE, MSG_EOR);

        bzero(buffer, MSGBUFFSIZE);
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        int chosenReq;
        sscanf(buffer, "%d", &chosenReq);

        char file[100] = "files/";
        char type[5] = ".fl";
        char sId[10];

        sprintf(sId, "%i", temporary[chosenReq]->fileIDI);
        strcat(file, sId);
        strcat(file, type);

        FILE *filePointer;
        char data[1024] = {0};
        char fileBuffer[2048];
        trimNL(file, sizeof(file));
        if (access(file, F_OK) == 0) {

            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, temporary[chosenReq]->fileName);

            send(newsockfd, buffer, MSGBUFFSIZE, MSG_EOR);

            filePointer = fopen(file, "r");
            bzero(fileBuffer, sizeof(fileBuffer));
            while (fgets(data, 50, filePointer) != NULL) {
                strcat(fileBuffer, data);
            }
            chSFErr(send(newsockfd, fileBuffer, 2048, 0));
            bzero(data, 1024);
            fclose(filePointer);
        } else {
            printf("File not found!\n");
        }

    } else {
        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "There are no available files for you\n");
        send(newsockfd, buffer, MSGBUFFSIZE, MSG_EOR);
    }

    for (int i = 0; i < found; ++i) {
        free(temporary[i]);
    }
}

void getFileInfoServ(int newsockfd) {
    char buffer[MSGBUFFSIZE];

    fileInfo *new = (fileInfo *) malloc(sizeof(fileInfo));

    bzero(buffer, MSGBUFFSIZE);
    chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

    sscanf(buffer, "%s %s %s", new->fileName, new->fromUser, new->toUser);
    new->fileIDI = fileIdS;
    fileIdS++;
    for (int i = 0; i < filesCount; ++i) {
        if (!fileList[i]) {
            pthread_mutex_lock(&mutex);
            fileList[i] = new;
            pthread_mutex_unlock(&mutex);
            updateFileLogSave();
        }
    }
}

void rcvFileServ(int newsockfd) {
    int n;
    FILE *filepointer;
    char buffer[2048];
    char directory[100] = "files/";
    char type[5] = ".fl";
    char sId[10];

    struct stat st = {0};
    if (stat(directory, &st) == -1) {
        mkdir(directory, 0700);
    }

    sprintf(sId, "%i", fileIdS);
    strcat(directory, sId);
    strcat(directory, type);

    pthread_mutex_lock(&mutex);
    filesCount++;
    pthread_mutex_unlock(&mutex);
    getFileInfoServ(newsockfd);
    filepointer = fopen(directory, "w");

    n = recv(newsockfd, buffer, 2048, MSG_WAITALL);
    if (n < 0) {
        perror("Receive file Error:");
    }

    fprintf(filepointer, "%s", buffer);
    bzero(buffer, 2048);
    fclose(filepointer);
}

void* checkExit() {
    char input[10];
    printf("t3\n");
    fgets(input,10,stdin);
    if(strcmp(input,"EXIT")) {
        cancel = 1;
        printf("t4\n");
    }

    exit(0);
}

int server(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in serv_addr;

    if (argc < 2) {
        fprintf(stderr, "usage %s port\n", argv[0]);
        return 1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr)); //vynuluje strukuturu servaddr
    serv_addr.sin_family = AF_INET; // cely internet
    serv_addr.sin_addr.s_addr = INADDR_ANY; // povolene ip adresy - teraz vsetky
    serv_addr.sin_port = htons(atoi(argv[1])); //nastavi port litle to big endian

    chScCRErr(sockfd = socket(AF_INET, SOCK_STREAM, 0)); // vytvori socket
    chScBDErr(bind(sockfd, (struct sockaddr *) &serv_addr,
                   sizeof(serv_addr))); // na socket namapujem strukturu (tento socket bude pracovat so spojeniami z celeho internetu na tomto porte)

    listen(sockfd,5); //pasivny socket (nie na komunikaciu, ale na pripojenie pouzivatela) n:kolko klientov sa moze pripojit v jeden moment

    //--------------------------------jadro aplikacie--------------------------------------------------------------------
    updateAccountsLoad();
    updateFileLogLoad();

    pthread_t th1;
    printf("t1\n");
    pthread_create(&th1, NULL, checkExit, NULL);
    printf("t2\n");


    while (cancel == 0) {
        int n = 0;
        struct sockaddr_in cli_addr = {};
        socklen_t cli_len = 0;

        n = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);
        if (n > 0) {
            client *new = (client *) malloc(sizeof(client));
            new->sockID = n;
            new->clientAddr = cli_addr;
            new->len = cli_len;
            new->id = idClient;
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < 10; ++i) {
                if (!clients[i]) {
                    clients[i] = new;
                    clientCount++;
                    idClient++;
                    break;
                }
            }
            pthread_mutex_unlock(&mutex);
            pthread_create(&thread[new->id], NULL, handleConnection,(void *) new);
        }
    }
    for (int i = 0; i < filesCount; ++i) {
        free(fileList[i]);
    }

    for (int i = 0; i < numberUsers; ++i) {
        free(users[i]);
    }

    for (int i = 0; i < clientCount; ++i) {
        free(clients[i]);
    }

    close(sockfd);
    exit(0);

    //--------------------------------jadro aplikacie--------------------------------------------------------------------
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

    char buffer[MSGBUFFSIZE];
    user *managingUser; // = (user *) malloc(sizeof(user));
    pthread_mutex_lock(&mutex);

    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, username) == 0) {
            managingUser = users[i];
        }
    }
    pthread_mutex_unlock(&mutex);

    if (managingUser->numReq == 0) {
        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "No friend requests \n Press 0 to continue");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "You should add friends in following menu  \n Press 0 to continue");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));
    } else {
        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "Here is list of people who want to be your friends: \n");
        for (int i = 0; i < managingUser->numReq; ++i) {
            int val = i;
            char sid[3];
            sprintf(sid, "%i", val);
            strcat(buffer, sid);
            strcat(buffer, ". ");
            strcat(buffer, managingUser->requests[i]->fromUser);
            strcat(buffer, "\n");
        }
        strcat(buffer, "Please enter number of request: \n");

        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        int chosenReq;
        sscanf(buffer, "%d", &chosenReq);
        if ((chosenReq >= 0) && (chosenReq < managingUser->numReq)) {

            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Do you wish to accept this request? \n(Y - Yes/N -NO ) \n");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

            bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
            chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));
            char choice = buffer[0];
            if ((choice == 'Y') || (choice == 'y')) {

                pthread_mutex_lock(&mutex);
                establishFriendship(username, managingUser->requests[chosenReq]->fromUser);
                managingUser->numReq--;

                for (int i = 0; i < managingUser->numReq; ++i) {
                    if (i >= chosenReq) {
                        managingUser->requests[i] = NULL;
                        fRequest *newRequest = (fRequest *) malloc(sizeof(fRequest));
                        strcpy(newRequest->fromUser, managingUser->requests[i + 1]->fromUser);
                        strcpy(newRequest->toUser, managingUser->requests[i + 1]->toUser);
                        managingUser->requests[i] = newRequest;
                        if (i == managingUser->numReq) {
                            managingUser->requests[i+1] = NULL;}
                    }
                }
                pthread_mutex_unlock(&mutex);

            } else {
                if ((choice == 'N') || (choice == 'n')) {
                    managingUser->numReq--;

                    pthread_mutex_lock(&mutex);
                    for (int i = 0; i < managingUser->numReq; ++i) {
                        if (i >= chosenReq) {
                            managingUser->requests[i] = NULL;
                            fRequest *newRequest = (fRequest *) malloc(sizeof(fRequest));
                            strcpy(newRequest->fromUser, managingUser->requests[i + 1]->fromUser);
                            strcpy(newRequest->toUser, managingUser->requests[i + 1]->toUser);
                            managingUser->requests[i] = newRequest;
                            if (i == managingUser->numReq) {
                                managingUser->requests[i+1] = NULL;}
                        }
                    }
                    pthread_mutex_unlock(&mutex);

                }
            }
        } else {
            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Wrong value\n");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

            bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
            chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));
        }
    }
    bzero(buffer, MSGBUFFSIZE);
    strcpy(buffer, "All doner\n");
    chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

    loggedMenuServ(newsockfd);
}

void removeFriend(int newsockfd, char *username) {
    /*  userovi sa ukazu jeho priatelia
     * on si jedneho vyberie
     * toto priatelstvo sa zrusi
     * */
    char buffer[MSGBUFFSIZE];
    user *managingUser;// = (user *) malloc(sizeof(user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, username) == 0) {
            managingUser = users[i];
        }
    }

    if (managingUser->numFrd != 0) {
        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "Choose friend (by number) to be removed: \n");

        for (int i = 0; i < managingUser->numFrd; ++i) {
            int val = i;
            char sid[3];
            sprintf(sid, "%i", val);
            strcat(buffer, sid);
            strcat(buffer, ". ");
            strcat(buffer, managingUser->friendlist[i]->fUsername);
            strcat(buffer, "\n");
        }

        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        int chosenFrd = -1;
        sscanf(buffer, "%d", &chosenFrd);
        if ((chosenFrd >=0) && (chosenFrd < managingUser->numFrd)) {

            pthread_mutex_lock(&mutex);
            user *removedFriend;// = (user *) malloc(sizeof(user));
            for (int i = 0; i < numberUsers; ++i) {
                if (strcmp(users[i]->username, managingUser->friendlist[chosenFrd]->fUsername) == 0) {
                    removedFriend = users[i];
                }
            }

            managingUser->numFrd--;
            for (int i = 0; i < managingUser->numFrd; ++i) {
                if (i > chosenFrd) {
                    managingUser->friendlist[i] = NULL;
                    friend *newFriend = (friend *) malloc(sizeof(friend));
                    strcpy(newFriend->fUsername, managingUser->friendlist[i + 1]->fUsername);
                    managingUser->friendlist[i] = newFriend;
                    if (i == managingUser->numFrd) {
                        managingUser->friendlist[i] = NULL;
                    }
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
                if (i > selected) {
                    removedFriend->friendlist[i] = NULL;
                    friend *newFriend = (friend *) malloc(sizeof(friend));
                    strcpy(newFriend->fUsername, removedFriend->friendlist[i + 1]->fUsername);
                    removedFriend->friendlist[i] = newFriend;
                    if (i == removedFriend->numFrd) {
                        removedFriend->friendlist[i+1] = NULL;
                    }
                }
            }
            pthread_mutex_unlock(&mutex);
            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Friend removed!\n");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
        } else {
            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Invalid input!\n");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
        }

    } else {
        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "You have no friends :( \n Press 0 to continue");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "You can add friends in following menu  \n");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
    }

    loggedMenuServ(newsockfd);

}

void sendMessage(int newsockfd, char *username) {
    /* ponukne userovi jeho kontakty na vyber
     * po vybere umozni napisat spravu
     * ulozi spravu prislusnemu kontaktu
     */
    char buffer[MSGBUFFSIZE];
    user *managingUser;// = (user *) malloc(sizeof(user));
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, username) == 0) {
            managingUser = users[i];
        }
    }
    pthread_mutex_unlock(&mutex);

    if (managingUser->numFrd != 0) {
        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "Choose friend: \n");

        for (int i = 0; i < managingUser->numFrd; ++i) {
            int val = i;
            char sid[3];
            sprintf(sid, "%i", val);
            strcat(buffer, sid);
            strcat(buffer, ". ");
            strcat(buffer, managingUser->friendlist[i]->fUsername);
            strcat(buffer, "\n");
        }

        strcat(buffer, "\nPlease enter number of user you wish to message: \n");

        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        int chosenFrd = -1;
        sscanf(buffer, "%d", &chosenFrd);

        if ((chosenFrd >=0) && (chosenFrd < managingUser->numFrd)) {

            user *textedFriend;// = (user *) malloc(sizeof(user));

            pthread_mutex_lock(&mutex);
            for (int i = 0; i < numberUsers; ++i) {
                if (strcmp(users[i]->username, managingUser->friendlist[chosenFrd]->fUsername) == 0) {
                    textedFriend = users[i];
                }
            }

            pthread_mutex_unlock(&mutex);

            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Write message: \n");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

            bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
            chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));
            addMessage(textedFriend->username, buffer, managingUser->username);

            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Message sent! \n");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
        } else {
            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Invalid input \n press enter");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

            bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
            chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Next time make sure to write\njust the number of friend you wish to message. \n");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
        }

    } else {
        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "You have no friends :( \n Press 0 to continue");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "You can add friends in following menu  \n");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "You can add friends in following menu  \n");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

    }
    loggedMenuServ(newsockfd);
}

void readMessages(int newsockfd, char *username) {
    char buffer[MSGBUFFSIZE];
    int n;
    user *managingUser ;//= (user *) malloc(sizeof(user));
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, username) == 0) {
            managingUser = users[i];
        }
    }
    pthread_mutex_unlock(&mutex);

    if (managingUser->numFrd != 0) {
        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "Choose friend: \n");

        for (int i = 0; i < managingUser->numFrd; ++i) {
            int val = i;
            char sid[3];
            sprintf(sid, "%i", val);
            strcat(buffer, sid);
            strcat(buffer, ". ");
            strcat(buffer, managingUser->friendlist[i]->fUsername);
            strcat(buffer, "\n");
        }


        send(newsockfd, buffer, MSGBUFFSIZE, MSG_EOR);

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer

        n = recv(newsockfd, buffer, MSGBUFFSIZE, MSG_WAITALL);
        if (n < 0) {
            perror("Receive name Error:");
        }
        int chosenFrd = -1;
        sscanf(buffer, "%d", &chosenFrd);

        if ((chosenFrd >=0)&& (chosenFrd < managingUser->numFrd)){

            pthread_mutex_lock(&mutex);

            user *textedFriend;// = (user *) malloc(sizeof(user));
            for (int i = 0; i < numberUsers; ++i) {
                if (strcmp(users[i]->username, managingUser->friendlist[chosenFrd]->fUsername) == 0) {
                    textedFriend = users[i];
                }
            }

            pthread_mutex_unlock(&mutex);

            getMessagesFrom(newsockfd, managingUser->username, textedFriend->username);
        } else {


            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Invalid input  \n");

            send(newsockfd, buffer, MSGBUFFSIZE, MSG_EOR);
        }


    } else {
        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "You have no friends so no messages :( \n Press 0 to continue");

        send(newsockfd, buffer, MSGBUFFSIZE, MSG_EOR);

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer

        n = recv(newsockfd, buffer, MSGBUFFSIZE, MSG_WAITALL);
        if (n < 0) {
            perror("Receive name Error:");
        }

        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "You can add friends in main menu  \n");

        send(newsockfd, buffer, MSGBUFFSIZE, MSG_EOR);
    }

    msgMenuServ(newsockfd);

}

void createGroup(int newsockfd, char *founderName) {
    char buffer[MSGBUFFSIZE];
    bzero(buffer, MSGBUFFSIZE);
    strcpy(buffer, "Choose a name for new chat \n");
    chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

    bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
    chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

    pthread_mutex_lock(&mutex);
    groupChat *newGroup = (groupChat *) malloc(sizeof(groupChat));
    strcpy(newGroup->chatName, buffer);
    friend *newMember = (friend *) malloc(sizeof(friend));
    strcpy(newMember->fUsername, founderName);
    newGroup->members[0] = newMember;
    newGroup->numMemb = 1;
    newGroup->numMsg = 0;
    groupChats[numberChats] = newGroup;
    numberChats++;

    user *founder;// = (user *) malloc(sizeof(user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, founderName) == 0) {
            founder = users[i];
        }
    }
    founder->groupChats[founder->numGroups] = newGroup;
    founder->numGroups++;
    pthread_mutex_unlock(&mutex);

    bzero(buffer, MSGBUFFSIZE);
    strcpy(buffer, "Chat created! \n");
    chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

    loggedMenuServ(newsockfd);

}

void addMember(int newsockfd, char *membersName) {

    user *managingUser;// = (user *) malloc(sizeof(user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, membersName) == 0) {
            managingUser = users[i];
        }
    }
    pthread_mutex_lock(&mutex);
    groupChat *group ;//= (groupChat *) malloc(sizeof(groupChat));
    char buffer[MSGBUFFSIZE];
    bzero(buffer, MSGBUFFSIZE);
    if ((managingUser->numGroups) > 0) {

        strcpy(buffer, "Here is list of all group chats: \n");
        for (int i = 0; i < managingUser->numGroups; ++i) {
            int val = i;
            char sid[3];
            sprintf(sid, "%i", val);
            strcat(buffer, sid);
            strcat(buffer, ". ");
            strcat(buffer, managingUser->groupChats[i]->chatName);
            strcat(buffer, "\n");
        }
        pthread_mutex_unlock(&mutex);
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        int test = -1;
        sscanf(buffer, "%d", &test);
        if ((test >= 0) && (test < managingUser->numGroups)) {

            pthread_mutex_lock(&mutex);
            if ((test >= 0)) {
                group = managingUser->groupChats[test];

            }
            pthread_mutex_unlock(&mutex);
            // Start of change

            friend *userList[999] ={0};
            int numOfusers = 1;
            for (int i = 0; i < numberUsers; ++i) {
                if (strcmp(users[i]->username, membersName) != 0) {
                    friend *helperFriend = (friend *) malloc(sizeof(friend));
                    strcpy(helperFriend->fUsername, users[i]->username);
                    userList[numOfusers] = helperFriend;
                    numOfusers++;
                    helperFriend = NULL;
                }
            }

            // End of change
            if (numOfusers > 1) {

                bzero(buffer, MSGBUFFSIZE);
                strcpy(buffer, "Here is list of all users: \n");
                for (int i = 1; i < numOfusers; ++i) {
                    int val = i;
                    char sid[3];
                    sprintf(sid, "%i", val);
                    strcat(buffer, sid);
                    strcat(buffer, ". ");
                    strcat(buffer, userList[i]->fUsername);
                    strcat(buffer, "\n");
                }
                chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

                bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
                chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

                int test2 = -1;
                int chosenUser = -1;
                sscanf(buffer, "%d", &test2);
                if ((test2 > 0) && (test2 <= numOfusers)) {

                    pthread_mutex_lock(&mutex);

                    for (int i = 0; i < numberUsers; ++i) {
                        if (strcmp(users[i]->username, userList[test2]->fUsername) == 0) {
                            chosenUser = i;
                        }
                    }

                    friend *newMember = (friend *) malloc(sizeof(friend));
                    if ((chosenUser >= 0)) {
                        strcpy(newMember->fUsername, users[chosenUser]->username);
                        group->members[group->numMemb] = newMember;
                        group->numMemb++;
                        user *newMemberU;// = (user *) malloc(sizeof(user));
                        newMemberU = users[chosenUser];

                        newMemberU->groupChats[newMemberU->numGroups] = group;
                        newMemberU->numGroups++;
                    }
                    pthread_mutex_unlock(&mutex);

                    for (int i = 0; i < numOfusers; ++i) {
                        free(userList[i]);
                    }

                    bzero(buffer, MSGBUFFSIZE);
                    strcpy(buffer, "User added to chat! \n");
                    chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
                } else {
                    bzero(buffer, MSGBUFFSIZE);
                    strcpy(buffer, "Incorrect number of user! \n");
                    chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
                }
            } else {
                bzero(buffer, MSGBUFFSIZE);
                strcpy(buffer, "it would seem you are the only user registered on this app:) \n Press 0 to continue");
                chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

                bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
                chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

                bzero(buffer, MSGBUFFSIZE);
                strcpy(buffer, "Does it feel nice to be the first? \n Or are you feeling lonely all by yourself? \n Press whatever to continue");
                chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
            }
        } else  {
            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Invalid group number \n Press 0 to continue");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

            bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
            chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, " Press 0 to continue \n");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
        }

    } else {
        pthread_mutex_unlock(&mutex);
        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "You aren't in any chat group :( \n Press 0 to continue");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "You can create one :) \n Press 0 to continue");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "Or try being nicer to your friends \n");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));


    }

    loggedMenuServ(newsockfd);

}

void removeMember(int newsockfd, char *membersName) {

    user *managingUser;// = (user *) malloc(sizeof(user));
    pthread_mutex_lock(&mutex);

    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, membersName) == 0) {
            managingUser = users[i];
        }
    }
    pthread_mutex_unlock(&mutex);

    groupChat *group;// = (groupChat *) malloc(sizeof(groupChat));
    char buffer[MSGBUFFSIZE];
    bzero(buffer, MSGBUFFSIZE);
    if (managingUser->numGroups > 0) {

        pthread_mutex_lock(&mutex);
        strcpy(buffer, "Choose chat you want to leave: \n");
        for (int i = 0; i < managingUser->numGroups; ++i) {
            int val = i;
            char sid[3];
            sprintf(sid, "%i", val);
            strcat(buffer, sid);
            strcat(buffer, ". ");
            strcat(buffer, managingUser->groupChats[i]->chatName);
            strcat(buffer, "\n");
        }
        pthread_mutex_unlock(&mutex);
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        int test= -1;
        sscanf(buffer, "%d", &test);

        if ((test >= 0) && (test < managingUser->numGroups)) {

            pthread_mutex_lock(&mutex);
            if ((test >= 0)) {
                group = managingUser->groupChats[test];

            }

            int chosen = 0;
            for (int i = 0; i < group->numMemb; ++i) {
                if (strcmp(group->members[i]->fUsername, membersName) == 0) {
                    chosen = i;
                }
            }
            group->numMemb--;

            for (int i = 0; i < group->numMemb; ++i) {
                if (i >= chosen) {
                    strcpy(group->members[i]->fUsername, group->members[i + 1]->fUsername);
                    if (i ==  group->numMemb) {
                        group->members[i + 1] = NULL;
                    }
                }
            }

            int chosengr;
            for (int i = 0; i < managingUser->numGroups; ++i) {
                if (strcmp(managingUser->groupChats[i]->chatName, group->chatName) == 0) {
                    chosengr = i;
                }
            }
            managingUser->numGroups--;

            for (int i = 0; i < managingUser->numGroups; ++i) {
                if (i >= chosengr) {
                    managingUser->groupChats[i] = NULL;
                    managingUser->groupChats[i] = managingUser->groupChats[i + 1];
                    if (i ==  managingUser->numGroups) {
                        managingUser->groupChats[i + 1] = NULL;
                    }
                }
            }
            pthread_mutex_unlock(&mutex);

            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "You left the chat\n");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
        } else {
            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Invalid input\n");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
        }

    } else {
        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "You aren't member of any chat\n");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "No need leav any then :)\n");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

    };

    loggedMenuServ(newsockfd);

}

void addGroupMessage(char *toGroupeName, char *text, char *fromUserName) {
    pthread_mutex_lock(&mutex);
    groupChat *toGroup;// = (groupChat *) malloc(sizeof(groupChat));
    for (int i = 0; i < numberChats; ++i) {
        if (strcmp(groupChats[i]->chatName, toGroupeName) == 0) {
            toGroup = groupChats[i];
        }
    }

    user *fromUser ;//= (user *) malloc(sizeof(user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, fromUserName) == 0) {
            fromUser = users[i];
        }
    }

    message *newMessage = (message *) malloc(sizeof(message));
    newMessage->newMsg = 1;
    strcpy(newMessage->text, text);
    strcpy(newMessage->fromUser, fromUser->username);

    toGroup->messages[toGroup->numMsg] = newMessage;
    toGroup->numMsg++;

    pthread_mutex_unlock(&mutex);

}

void sendGroupMessage(int newsockfd, char *userName) {
    char buffer[MSGBUFFSIZE];
    pthread_mutex_lock(&mutex);
    user *managingUser;// = (user *) malloc(sizeof(user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, userName) == 0) {
            managingUser = users[i];
        }
    }
    pthread_mutex_unlock(&mutex);


    if (managingUser->numGroups != 0) {
        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "Choose Group chat: \n");

        for (int i = 0; i < managingUser->numGroups; ++i) {
            int val = i;
            char sid[3];
            sprintf(sid, "%i", val);
            strcat(buffer, sid);
            strcat(buffer, ". ");
            strcat(buffer, managingUser->groupChats[i]->chatName);
            strcat(buffer, "\n");
        }

        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        int chosenFrd = -1 ;
        sscanf(buffer, "%d", &chosenFrd);

        if ((chosenFrd >= 0) && (chosenFrd < managingUser->numGroups)) {

            pthread_mutex_lock(&mutex);
            groupChat *textedGroup;// = (groupChat *) malloc(sizeof(groupChat));
            for (int i = 0; i < numberChats; ++i) {
                if (strcmp(groupChats[i]->chatName, managingUser->groupChats[chosenFrd]->chatName) == 0) {
                    textedGroup = groupChats[i];
                }
            }
            pthread_mutex_unlock(&mutex);
            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Write message: \n");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

            bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
            chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));
            char editedMSG[MSGBUFFSIZE];
            bzero(editedMSG, MSGBUFFSIZE);
            strcpy(editedMSG, managingUser->username);
            strcat(editedMSG, ": ");
            strcat(editedMSG, buffer);
            addGroupMessage(textedGroup->chatName, editedMSG, managingUser->username);

            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Message sent! \n");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
        } else {
            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Invalid group number! \n press 0 to continue");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

            bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
            chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Make sure you enter only the number of group \n");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
        }


    } else {
        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "You aren't a member of any groupchats \n Press 0 to continue");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "You can create a groupchat in following menu  \n");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "Don't forget to add group members :)  \n");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

    }
    loggedMenuServ(newsockfd);
}

void getGroupMessages(int newsockfd, char *username) {
    char buffer[MSGBUFFSIZE];
    pthread_mutex_lock(&mutex);
    user *managingUser;// = (user *) malloc(sizeof(user));
    for (int i = 0; i < numberUsers; ++i) {
        if (strcmp(users[i]->username, username) == 0) {
            managingUser = users[i];
        }
    }
    pthread_mutex_unlock(&mutex);

    if (managingUser->numGroups != 0) {
        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "Choose Chat: \n");

        for (int i = 0; i < managingUser->numGroups; ++i) {
            int val = i;
            char sid[3];
            sprintf(sid, "%i", val);
            strcat(buffer, sid);
            strcat(buffer, ". ");
            strcat(buffer, managingUser->groupChats[i]->chatName);
            strcat(buffer, "\n");
        }

        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        int chosenFrd= -1;
        sscanf(buffer, "%d", &chosenFrd);

        if ((chosenFrd >= 0) && (chosenFrd < managingUser->numGroups)) {

            pthread_mutex_lock(&mutex);
            groupChat *textedGroup ;//= (groupChat *) malloc(sizeof(groupChat));
            for (int i = 0; i < numberChats; ++i) {
                if (strcmp(groupChats[i]->chatName, managingUser->groupChats[chosenFrd]->chatName) == 0) {
                    textedGroup = groupChats[i];
                }
            }

            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Here is the Chat log: \n");

            for (int i = 0; i < textedGroup->numMsg; ++i) {
                strcat(buffer, textedGroup->messages[i]->text);
            }
            pthread_mutex_unlock(&mutex);

            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
        } else {
            bzero(buffer, MSGBUFFSIZE);
            strcpy(buffer, "Invalid input! \n");
            chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));
        }



    } else {
        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "You aren't a member of any groupchats \n Press 0 to continue");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE); //vynulujem buffer
        chScRErr(read(newsockfd, buffer, MSGBUFFSIZE));

        bzero(buffer, MSGBUFFSIZE);
        strcpy(buffer, "You can create a groupchat in following menu  \n");
        chScWErr(write(newsockfd, buffer, MSGBUFFSIZE));

    }
    loggedMenuServ(newsockfd);
}
