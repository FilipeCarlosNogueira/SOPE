#include <stdlib.h>
#include <string.h>

#include "sope.h"

int main(int argc, char const *argv[]){
        char *id, *password, *delay, *operation;
        int newid, saldo, iddest, montante;
        char newpassword[MAX_PASSWORD_LEN];


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
        if(atoi(operation) == 0 || atoi(operation) == 1 || atoi(operation) == 2 || atoi(operation) == 3)
                printf("%s ",operation);
        else
                printf("Invalid Operation Number\n");

        if(atoi(operation) == 0 || atoi(operation) == 2) {
                if(argc != 6) {
                        printf("Invalid Arguments Number\n");
                        return 0;
                }

                if(atoi(operation) == 0) {
                        newid = atoi(strtok((char *) argv[5], " "));
                        saldo = atoi(strtok(NULL, " "));
                        strcpy(newpassword, strtok(NULL, " "));

                        if(newid > MAX_BANK_ACCOUNTS)
                                printf("Invalid newID Number\n");
                        else
                                printf("%d ", newid);

                        if(saldo > MAX_BALANCE || saldo < MIN_BALANCE)
                                printf("Invalid Saldo Number\n");
                        else
                                printf("%d ", saldo);

                        if(strlen(newpassword) < MIN_PASSWORD_LEN)
                                printf("Invalid newPassword\n");
                        else
                                printf("%s\n", newpassword);
                }
                else {
                        iddest = atoi(strtok((char *) argv[5], " "));
                        montante = atoi(strtok(NULL, " "));

                        if(iddest > MAX_BANK_ACCOUNTS)
                                printf("Invalid newID Number\n");
                        else
                                printf("%d ", iddest);

                        printf("%d ", montante);
                }
        }
        return 0;
}
