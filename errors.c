//
// Created by pc on 3. 1. 2022.
//

#include <stdio.h>
#include <stdlib.h>
#include "errors.h"

void chScRErr(int n) {
    if (n < 0)
    {
        perror("Error reading from socket");
        exit(1);
    }
}
void chScWErr(int n) {
    if (n < 0) {
        perror("Error writing to socket");
        exit(1);
    }
}
void chScACErr(int n) {
    if (n < 0)
    {
        perror("ERROR on accept");
        exit(1);
    }
}
void chScBDErr(int n) {
    if (n < 0)
    {
        perror("Error binding socket address");
        exit(1);
    }
}
void chScCRErr(int n) {
    if (n < 0)
    {
        perror("Error creating socket");
        exit(1);
    }
}
void chScCNErr(int n) {
    if (n < 0)
    {
        perror("Error connecting to socket");
        exit(1);
    }
}