#include "variables.h"
#include "file.h"
#include "interface.h"

struct forensic fs;
int nfile;
int ndir;
char out[20];

void sig_handler(int sig)
{
        if(sig == SIGINT) {
                printf("\nExiting Process, Ctrl+C. \n");
                _exit(1);
        }
        if(sig == SIGUSR1) {
                ndir++;
                printf("New directory: %d/%d directories/files at this time\n", ndir, nfile);
        }
        if(sig == SIGUSR2)
        {
                nfile++;
        }
}

void parsingArg(int argc, char const *argv[]){
        char *token;
        char h_aux[100];
        char *delim = ",";

        //char error_message[] = "Usage: forensic [-r] [-h [md5[,sha1[,sha256]]] [-o <outfile>] [-v] <file|dir>\n";

        for(int i = 0; i < argc-1; i++) {

                //directory
                if(strcmp(argv[i], "-r") == 0) {
                        fs.r_flag = true;
                }

                //output_file
                else if(strcmp(argv[i], "-o") == 0) {
                        fs.output_file = open(argv[i+1], O_WRONLY|O_CREAT|O_APPEND|O_TRUNC,0600);
                        strcpy(out,argv[i+1]);
                        i++;
                }

                //execution register
                else if(strcmp(argv[i], "-v") == 0) {
                        fs.execution_register = open(getenv("LOGFILENAME"), O_RDWR|O_CREAT|O_APPEND|O_TRUNC,0600);
                }

                //digital prints
                else if (strcmp(argv[i], "-h") == 0) {
                        token = strtok((char *)argv[i+1], delim);

                        while(token != NULL) {

                                if(strcmp(token, "md5") == 0) fs.md5 = true;
                                if(strcmp(token, "sha1") == 0) fs.sha1 = true;
                                if(strcmp(token, "sha256") == 0) fs.sha256 = true;

                                token = strtok(NULL, delim);
                        }

                        printf("\n");

                        i++;
                }
        }

        //it can be a file or a dir.
        strcpy(fs.name, argv[argc-1]);

        if(lstat(fs.name, &fs.last) != 0) {
                perror("file|directory stat");
                exit(EXIT_FAILURE);
        }

        //if user specified the execution register
        if(fs.execution_register != -1) {
                fs.start = times(&fs.time);

                strcpy(h_aux, "COMAND forensic ");

                for(int i = 1; i < argc-1; i++) {
                        strcat(h_aux, argv[i]);
                        strcat(h_aux, " ");
                }

                strcat(h_aux, "./");
                strcat(h_aux, argv[argc-1]);
                strcat(h_aux, "\n");

                write(fs.execution_register, h_aux, strlen(h_aux)*sizeof(char));
        }
}

int main(int argc, char const *argv[])
{
        //criar handler para sigint
        struct sigaction stop;

        stop.sa_handler=sig_handler;
        sigemptyset(&stop.sa_mask);
        stop.sa_flags=SA_RESTART;

        sigaction(SIGINT,&stop,NULL);

        struct sigaction dir;

        dir.sa_handler=sig_handler;
        sigemptyset(&dir.sa_mask);
        dir.sa_flags=SA_RESTART;

        sigaction(SIGUSR1,&dir,NULL);

        struct sigaction file;

        file.sa_handler=sig_handler;
        sigemptyset(&file.sa_mask);
        file.sa_flags=SA_RESTART;

        sigaction(SIGUSR2,&file,NULL);
        //  sleep(1);

        init(&fs);

        //Receber, tratar e guardar os argumentos e variáveis de ambiente.
        parsingArg(argc, argv);

        //Extrair a informação solicitada de apenas um ficheiro e imprimi-la na saída padrão de acordo com os argumentos passados.
        recurs(&fs);

        if(fs.output_file != -1) {
                printf("New directory: %d/%d directories/files at this time\n", ndir, nfile);
                printf("Data saved on file %s\n", out);
        }
        if(fs.execution_register != -1)
                printf("Execution records saved on file %s\n", getenv("LOGFILENAME"));

        return 0;
}
