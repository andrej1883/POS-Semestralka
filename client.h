

#ifndef SOCKETY_CLIENT_H
#define SOCKETY_CLIENT_H

void authClie(int sockfd);
void registerClie(int sockfd);
int client(int argc, char *argv[]);

void getMessagesClie(int sockfd);
void getMessagesFromClie(int sockfd);
void addFriendClie(int sockfd);
manageRequestsClie(int sockfd);

#endif //SOCKETY_CLIENT_H
