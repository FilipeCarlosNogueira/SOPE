#include <stdlib.h>
#include <string.h>

#include "sope.h"

int main(int argc, char const *argv[]){
        char *password;
        int id, delay, operation, newid, saldo, iddest, montante;
        char newpassword[MAX_PASSWORD_LEN];


        if(argc != 5 && argc != 6) {
                printf("Invalid Arguments Number\n");
                return 0;
        }

        id = atoi(argv[1]);
        if(id > MAX_BANK_ACCOUNTS)
                printf("Invalid ID Number\n");
        else
                printf("%d ",id);

        password = malloc(sizeof(argv[2])+1);
        strcpy(password, argv[2]);
        if(strlen(password) < MIN_PASSWORD_LEN || strlen(password) > MAX_PASSWORD_LEN)
                printf("Invalid Password\n");
        else
                printf("%s ", password);

        delay = atoi(argv[3]);
        if(delay > MAX_OP_DELAY_MS)
                printf("Invalid Delay Number\n");
        else
                printf("%d ", delay);

        operation = atoi(argv[4]);
        if(operation == 0 || operation == 1 || operation == 2 || operation == 3)
                printf("%d ",operation);
        else
                printf("Invalid Operation Number\n");

        if(operation == 0 || operation == 2) {
                if(argc != 6) {
                        printf("Invalid Arguments Number\n");
                        return 0;
                }

                if(operation == 0) {
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
