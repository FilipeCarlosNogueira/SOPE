#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "sope.h"
#include "types.h"


int main(int argc, char const *argv[]){
        char *id, *password, *delay, *operation, *arguments = "";

        if(argc != 5 && argc != 6) {
                printf("Invalid Arguments Number\n");
                return 0;
        }

        id = malloc(sizeof(argv[1])+1);
        strcpy(id, argv[1]);
        if(atoi(id) > MAX_BANK_ACCOUNTS)
                printf("Invalid ID Number\n");
        else
                printf("%s ",id);

        password = malloc(sizeof(argv[2])+1);
        strcpy(password, argv[2]);
        if(strlen(password) < MIN_PASSWORD_LEN || strlen(password) > MAX_PASSWORD_LEN)
                printf("Invalid Password\n");
        else
                printf("%s ", password);

        delay = malloc(sizeof(argv[3])+1);
        strcpy(delay, argv[3]);
        if(atoi(delay) > MAX_OP_DELAY_MS)
                printf("Invalid Delay Number\n");
        else
                printf("%s ", delay);

        operation = malloc(sizeof(argv[4])+1);
        strcpy(operation, argv[4]);
        if(atoi(operation) != 0 || atoi(operation) != 1 || atoi(operation) != 2 || atoi(operation) != 3)
                printf("Invalid Operation Number\n");
        else
                printf("%s ",operation);

        if(atoi(operation) == 0 || atoi(operation) == 2) {
                arguments = malloc(sizeof(argv[5])+1);
                strcpy(arguments, argv[5]);
                printf("%s\n", arguments);
        }
        return 0;
}
