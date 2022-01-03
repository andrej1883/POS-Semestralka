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
    }
}
void chScWErr(int n) {
    if (n < 0) {
        perror("Error writing to socket");
    }
}
void chScACErr(int n) {
    if (n < 0)
    {
        perror("ERROR on accept");
    }
}
void chScBDErr(int n) {
    if (n < 0)
    {
        perror("Error binding socket address");
    }
}
void chScCRErr(int n) {
    if (n < 0)
    {
        perror("Error creating socket");
    }
}
