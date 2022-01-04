//
// Created by pc on 3. 1. 2022.
//

#ifndef CHAT_CLIENTHANDLER_H
#define CHAT_CLIENTHANDLER_H

void welcomeCli(int sockfd);
void loggedMenuCli(int sockfd, char name[10]);
void msgMenuCli(int sockfd);

#endif //CHAT_CLIENTHANDLER_H
