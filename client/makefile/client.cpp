#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <memory.h>


using namespace std;

int main(int argc , char *argv[])
{

    //socket的建立
    int sockfd = 0;
    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if (sockfd == -1){
        printf("Fail to create a socket.");
    }

    //socket的連線
    struct sockaddr_in info;
    bzero(&info, sizeof(info));
    info.sin_family = PF_INET;

    //localhost test
    info.sin_addr.s_addr = inet_addr("127.0.0.1");
    info.sin_port = htons(atoi(argv[1]));

    int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));

    if(err==-1){
        printf("Connection error");
    }

   // Send a message to server
    char receiveMessage[1024] = {};
    memset(receiveMessage, '\0', sizeof(receiveMessage));
    
    try
    {
        recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
    }
    catch (bad_exception)
    {
        printf("ReceiveMessage error");
        return 0;
    }
    printf("%s",receiveMessage);

    string text;
    while(true)
    {
        cin >> text;
        text = text+ "\n\0";
        const char* sendmsg = text.c_str();
        send(sockfd,sendmsg, strlen(sendmsg),0);
        memset(receiveMessage, '\0', sizeof(receiveMessage));
        recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
            
        
        printf("%s", receiveMessage);
    }

    printf("close Socket\n");
    close(sockfd);
    return 0;
    
    }
