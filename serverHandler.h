//
// Created by pc on 3. 1. 2022.
//

#ifndef CHAT_SERVERHANDLER_H
#define CHAT_SERVERHANDLER_H

void welcomeServ(int pnewsockfd,struct sockaddr_un cli_addr,  socklen_t cli_len);
void loggedMenuServ(int newsockfd,struct sockaddr_in cli_addr,  socklen_t cli_len);
void msgMenuServ(int newsockfd, char *username);

void fileMenuServ(int pnewsockfd,struct sockaddr_in cli_addr,  socklen_t cli_len);

void groupMenuServ(int newsockfd, char *username);

#endif //CHAT_SERVERHANDLER_H
