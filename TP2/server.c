#include <stdlib.h>
#include <string.h>

#include "sope.h"


int main(int argc, char const *argv[]){
        int tnum;
        char *password;

        if(argc != 3) {
                printf("Invalid Arguments Number\n");
                return 0;
        }

        tnum = atoi(argv[1]);
        if(tnum > MAX_BANK_OFFICES)
                printf("Invalid Thread Number\n");
        else
                printf("%d ",tnum);

        password = malloc(sizeof(argv[2])+1);
        strcpy(password, argv[2]);
        if(strlen(password) < MIN_PASSWORD_LEN || strlen(password) > MAX_PASSWORD_LEN)
                printf("Invalid Password\n");
        else
                printf("%s\n", password);
        return 0;
}
