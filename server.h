

#ifndef SOCKETY_SERVER_H
#define SOCKETY_SERVER_H

void trimNL(char* arr, int length);
void authServ(int newsockfd);
void updateAccountsLoad();
void updateAccountsSave();
void registerUser(int newsockfd);
void deleteUser(char* name);
int server(int argc, char *argv[]);

#endif //SOCKETY_SERVER_H
