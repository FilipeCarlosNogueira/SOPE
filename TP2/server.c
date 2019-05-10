#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "sope.h"
#include "types.h"


int main(int argc, char const *argv[]){
        char *tnum, *password;

        if(argc != 3) {
                printf("Invalid Arguments Number\n");
                return 0;
        }
        
        tnum = malloc(sizeof(argv[1])+1);
        strcpy(tnum, argv[1]);
        if(atoi(tnum) > MAX_BANK_OFFICES)
                printf("Invalid Thread Number\n");
        else
                printf("%s ",tnum);

        password = malloc(sizeof(argv[2])+1);
        strcpy(password, argv[2]);
        if(strlen(password) < MIN_PASSWORD_LEN || strlen(password) > MAX_PASSWORD_LEN)
                printf("Invalid Password\n");
        else
                printf("%s\n", password);
        return 0;
}
