

#ifndef SOCKETY_SERVER_H
#define SOCKETY_SERVER_H

void trimNL(char* arr, int length);
void authServ(int newsockfd);
void updateAccountsLoad();
void updateAccountsSave();
void registerUser(int newsockfd);
void deleteUser(char* name);
void rcvFileServ(int newsockfd);
int server(int argc, char *argv[]);


void addFriend(int newsockfd, char* username);
void updateFriendlist(char * usersName, char * friendsName);
void manageRequests(int newsockfd, char* username);
void sendRequest(char * fromUser, char* toUser);
void establishFriendship(char * friendOne, char * friendTwo);
void addMessage( char* toUserName, char* text, char* fromUserName);
void getMessages(int newsockfd, char* msgOfUser);
void getMessagesFrom(int newsockfd, char* msgOfUser, char* msgFromUser);
void removeFriend(int newsockfd, char* username);
void sendMessage(int newsockfd, char* username);

#endif //SOCKETY_SERVER_H
