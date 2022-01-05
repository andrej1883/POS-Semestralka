

#ifndef SOCKETY_CLIENT_H
#define SOCKETY_CLIENT_H

void authClie(int sockfd);
void registerClie(int sockfd);
void fileMenuCli(int sockfd);
int client(int argc, char *argv[]);

void getMessagesClie(int sockfd);
void getMessagesFromClie(int sockfd);
void addFriendClie(int sockfd);
void manageRequestsClie(int sockfd);
void removeFriendClie(int sockfd);
void backTologMenu(int sockfd);
void sendMessageClie(int sockfd);
void sendFileClie(char* filename,int sockfd, char* toUser);
#endif //SOCKETY_CLIENT_H
