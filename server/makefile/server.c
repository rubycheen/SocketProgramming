#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>



#define FALSE 0
#define TRUE 1
#define BACKLOG 10
#define MaxNameLen 10
#define INET_ADDRSTRLEN 16


typedef struct pthread_arg_t {
    int new_socket_fd;
    struct sockaddr_in client_address;
    /* TODO: Put arguments passed to threads here. See lines 116 and 139. */
} pthread_arg_t;

/* Thread routine to serve connection to client. */
void *pthread_routine(void *arg);
char name[BACKLOG][MaxNameLen];
char acceptPort[BACKLOG][16];
int account[BACKLOG] = {0};
int regNum = 0;
int threadNum[BACKLOG] = {0};
char clientIP[BACKLOG][INET_ADDRSTRLEN];


/* Signal handler to handle SIGTERM and SIGINT signals. */
void signal_handler(int signal_number);

int main(int argc, char *argv[]) {
    int port, socket_fd, new_socket_fd;
    struct sockaddr_in address;
    pthread_attr_t pthread_attr;
    pthread_arg_t *pthread_arg;
    pthread_t pthread;
    socklen_t client_address_len;
    
    /* Get port from command line arguments or stdin. */
    port = argc > 1 ? atoi(argv[1]) : 0;
    if (!port) {
        printf("Enter Port: ");
        scanf("%d", &port);
    }
    
    /* Initialise IPv4 address. */
    memset(&address, 0, sizeof address);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    
    /* Create TCP socket. */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    
    /* Bind address to socket. */
    if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("bind");
        exit(1);
    }

    
    /* Listen on socket. */
    if (listen(socket_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    
    /* Assign signal handlers to signals. */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    
    /* Initialise pthread attribute to create detached threads. */
    if (pthread_attr_init(&pthread_attr) != 0) {
        perror("pthread_attr_init");
        exit(1);
    }
    if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED) != 0) {
        perror("pthread_attr_setdetachstate");
        exit(1);
    }
    
    while (1) {
        /* Create pthread argument for each connection to client. */
        pthread_arg = (pthread_arg_t *)malloc(sizeof *pthread_arg);
        if (!pthread_arg) {
            perror("malloc");
            continue;
        }
        
        /* Accept connection to client. */
        client_address_len = sizeof pthread_arg->client_address;
        new_socket_fd = accept(socket_fd, (struct sockaddr *)&pthread_arg->client_address, &client_address_len);
        if (new_socket_fd == -1) {
            perror("accept");
            free(pthread_arg);
            continue;
        }
        
        /* Initialise pthread argument. */
        pthread_arg->new_socket_fd = new_socket_fd;

        
        /* Create thread to serve connection to client. */
        if (pthread_create(&pthread, &pthread_attr, pthread_routine, (void *)pthread_arg) != 0) {
            perror("pthread_create");
            free(pthread_arg);
            continue;
        }
        else
        {
            threadNum[regNum] = new_socket_fd;
            printf("new thread = ");
            printf("%d\n",new_socket_fd);
        }
    }
    
    return 0;
}

void *pthread_routine(void *arg) {
    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    int new_socket_fd = pthread_arg->new_socket_fd;
    struct sockaddr_in client_address = pthread_arg->client_address;
    
    free(arg);
    
    char message[1024] = {};
    char client_message[1024] = {};
    int n = 0;
    n = write(new_socket_fd, "Hello :)\n", strlen("Hello :)\n"));
    n = write(new_socket_fd, "Let's create your account!\n", strlen("Let's create your account!\n"));

    //Receive msg from client
    while(recv(new_socket_fd , client_message , 1024 , 0) > 0)
    {
        if(NULL != strstr(client_message, "REGISTER"))//REGISTER
        {
            int check = TRUE;
            char *ret = strchr(client_message,'#')+1; // UserName

            for(int i = 0; i < regNum; i++)
            {
                if(0 == strcmp(ret, name[i]))
                {
                    check = FALSE;
                    n = write(new_socket_fd, "210 FAIL\n", strlen("210 FAIL\n"));
                    break;
                }
            }
            
            if(check == TRUE)
            {
                strcpy(name[regNum], ret);
                name[regNum][strlen(name[regNum])-1] = 0;
                account[regNum] = 10000;
                inet_ntop(AF_INET, &client_address.sin_addr, clientIP[regNum], sizeof(client_address));
                regNum = regNum+1;
                n = write(new_socket_fd, "100 OK\n", strlen("100 OK\n"));
            }
            memset(ret, '\0', sizeof(ret));
            memset(client_message, '\0', sizeof(client_message));
        }
        
        else if(NULL != strchr(client_message,'#')) // TRANSACTION
        {
            int check = FALSE;
            char *ret = strchr(client_message,'#')+1; // acceptPort
            
            for(int i = 0; i < regNum; i++)
            {
                char *pos = strstr(client_message, name[i]); //檢查有沒有註冊過
                if(pos != NULL) //還有重複問題
                {
                    check = TRUE;
                    strcpy(acceptPort[i], ret); // 最後有\n
                    acceptPort[i][strlen(acceptPort[i])-1] = 0;
                    char* changeLine = "\n";
                    char c[4] = {};
                    char money[64] = {};
                    sprintf(c,"%d",regNum);
                    strcat(c,changeLine);
                    sprintf(money,"%d",account[i]);
                    strcat(money,changeLine);
                    n = write(new_socket_fd, money, strlen(money));
                    n = write(new_socket_fd, "number of accounts online: ", strlen("number of accounts online: "));
                    n = write(new_socket_fd, c, strlen(c));
                }
            }
            
            if(check == TRUE)
            {
                for(int i = 0; i < regNum; i++)
                {

                    char state [1024] = {};
                    char flag = '#';
                    char* changeLine = "\n";
                    strcat(state,name[i]);
                    strcat(state,&flag);
                    strcat(state,clientIP[i]);
                    strcat(state,&flag);
                    strcat(state,acceptPort[i]);
                    strcat(state,changeLine);
                    n = write(new_socket_fd, state, strlen(state));
                }
            }
            else
            {
                n = write(new_socket_fd, "220 AUTH_FAIL\n", strlen("220 AUTH_FAIL\n"));
            }
            memset(ret, '\0', sizeof(ret));
            memset(client_message, '\0', sizeof(client_message));
        }
        else if (0 == strcmp("List\n",client_message))
        {
            char* changeLine = "\n";
            char c[4] = {};
            sprintf(c,"%d",regNum);
            strcat(c,changeLine);
            for(int i = 0; i < regNum; i++)
            {
                if(new_socket_fd == threadNum[i])
                {
                    char acc[16] = {};
                    sprintf(acc,"%d\n",account[i]);
                    n = write(new_socket_fd, acc, strlen(acc));
                    break;
                }
            }
            n = write(new_socket_fd, "number of accounts online: ", strlen("number of accounts online: "));
            n = write(new_socket_fd, c, strlen(c));
            for(int i = 0; i < regNum; i++)
            {
                char state [1024] = {};
                char flag = '#';
                char* changeLine = "\n";
                strcat(state,name[i]);
                strcat(state,&flag);
                strcat(state,clientIP[i]);
                strcat(state,&flag);
                strcat(state,acceptPort[i]);
                strcat(state,changeLine);
                n = write(new_socket_fd, state, strlen(state));
            }
        }
        else if (0 == strcmp("Exit\n",client_message)) // Exit
        {
            for(int i = 0; i < regNum; i++)
            {
                if(new_socket_fd == threadNum[i] && i > 0)
                {
                    for(int j = i; j < regNum-1; j++)
                    {
                        memset(name[j], '\0', sizeof(name[j]));
                        strcpy(name[j] ,name[j+1]);
                        memset(acceptPort[j], '\0', sizeof(acceptPort[j]));
                        strcpy(acceptPort[j] ,acceptPort[j+1]);
                        memset(clientIP[j], '\0', sizeof(clientIP[j]));
                        strcpy(clientIP[j] ,clientIP[j+1]);
                        account[j] = account[j+1];
                        threadNum[j] = threadNum[j+1];
                    }
                    break;
                }
                else if(new_socket_fd == threadNum[i] && i == 0)
                {
                    memset(name[0], '\0', sizeof(name[0]));
                    strcpy(name[0] ,name[1]);
                    memset(acceptPort[0], '\0', sizeof(acceptPort[0]));
                    strcpy(acceptPort[0] ,acceptPort[1]);
                    memset(clientIP[0], '\0', sizeof(clientIP[0]));
                    strcpy(clientIP[0] ,clientIP[1]);
                    account[0] = account[1];
                    threadNum[0] = threadNum[1];
                }
            }
            regNum--;
            n = write(new_socket_fd, "Bye\n", strlen("Bye\n"));
            n = write(new_socket_fd, "Have a nice day~\n", strlen("Have a nice day~\n"));
            close(new_socket_fd);
        }
        else if (0 == strcmp("help\n",client_message)) // help
        {
            n = write(new_socket_fd, "***** **** ** **** *** ** * ** *** **** ** **** ******\n", strlen("******************************************************\n"));
            n = write(new_socket_fd, "* *     *    *   *  * * GUIDE * *  *   *    *     *  *\n", strlen("* *  *   *   *   *  * * GUIDE * *  *   *   *   *   * *\n"));
            n = write(new_socket_fd, "** ***** **** *** ** *********** ** *** **** ***** ***\n", strlen("******************************************************\n"));
            n = write(new_socket_fd, "*                                                    *\n", strlen("******************************************************\n"));
            n = write(new_socket_fd, "*    [REGISTER] -- create your account               *\n", strlen("******************************************************\n"));
            n = write(new_socket_fd, "*    [user#port] -- checkout your registeration      *\n", strlen("******************************************************\n"));
            n = write(new_socket_fd, "*    [List] -- check the other online accounts       *\n", strlen("******************************************************\n"));
            n = write(new_socket_fd, "*    [Exit] -- leave the proccess                    *\n", strlen("******************************************************\n"));
            n = write(new_socket_fd, "*                                                    *\n", strlen("******************************************************\n"));
            n = write(new_socket_fd, "***** **** ** **** *** ** * ** *** **** ** **** ******\n", strlen("******************************************************\n"));

        }
        else
        {
            n = write(new_socket_fd, "use [help] to find out your command\n", strlen("use [help] to find out your command\n"));
            memset(client_message, '\0', sizeof(client_message));
        }
    }
    
    close(new_socket_fd);
    return NULL;
}

void signal_handler(int signal_number) {
    exit(0);
}
