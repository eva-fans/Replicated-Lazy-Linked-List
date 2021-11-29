#include "packet.h"
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <memory.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>

int main(void)
{
    int sockPri, sockBck, sockClt; // Primary server socket, backup server socket, client socket
    packet_t request; // the request sent to primary server from client
    packet_t response; // the response which primary server send to client
    ssize_t size; // the size of network packet

    intset_l_t *set=set_new_l(); // relicated linked list
    FILE *log=fopen("log","r+"); // open the log file
    if(log==NULL) // cant open log file
    {
        printf("open log file fails\n"); 
        return 1;
    }
    
    // create primary server socket and backup server socket
    if((sockPri=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0 || (sockBck=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
    {
        fclose(log);
        printf("socket function fails\n");
        return 1;
    }
    struct sockaddr_in addr; // the address backup server
    memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_addr.s_addr=inet_addr(BACKUP_SERVER_IP);
    addr.sin_family=AF_INET;
    addr.sin_port=htons(BACKUP_SERVER_PORT);
    // connect to backup server first
    if(connect(sockBck,(struct sockaddr*)&addr,sizeof(struct sockaddr_in))<0)
    {
        close(sockPri);
        close(sockBck);
        fclose(log);
        printf("connecting to backup server fail\n");
        return 1;
    }

    do
    {
        char operation[100]; // the operation that is read from log
        val_t val; // operation number
        packet_t req; // the request sent to backup server
        // ask backup server recover
        req.operation=PACKET_OPERATION_RECOVER; 
        size=send(sockBck,&req,sizeof(packet_t),0);
        fscanf(log,"%s %ld\n",operation,&val);
        if(strcmp(operation,"insert")==0) // insert
        {
            set_add_l(set,val,0);
            req.operation=PACKET_OPERATION_INSERT;
            req.val=val;
            printf("insert %ld\n",val);
            size=send(sockBck,&req,sizeof(packet_t),0);
        }
        else if(strcmp(operation,"remove")==0) // remove
        {
            set_remove_l(set,val,0);
            req.operation=PACKET_OPERATION_REMOVE;
            req.val=val;
            printf("remove %ld\n",val);
            size=send(sockBck,&req,sizeof(packet_t),0);
        }
    } while (!feof(log) && size>0);

    // listen at PRIMARY_SERVER_PORT
    memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(PRIMARY_SERVER_PORT);
    if(bind(sockPri,(struct sockaddr*)&addr,sizeof(struct sockaddr_in))<0)
    {
        close(sockPri);
        fclose(log);
        printf("bind socket to localhost fails\n");
        return 1;
    }
    if(listen(sockPri,5)<0)
    {
        close(sockPri);
        fclose(log);
        printf("listen to the port fails\n");
        return 1;
    }

    // accept a client
    memset(&addr,0,sizeof(struct sockaddr_in));
    socklen_t socklen=sizeof(struct sockaddr_in);
    if((sockClt=accept(sockPri, (struct sockaddr*)&addr,&socklen))<0)
    {
        close(sockPri);
        fclose(log);
        printf("accepting client's connection fails\n");
        return 1;
    }

    // set backup socket to nonblock mode
    int flags=fcntl(sockBck,F_GETFL,0);
    fcntl(sockBck,F_SETFL,flags | O_NONBLOCK);
    do
    {
        // detect whether backup server crash
        memset(&request,0,sizeof(packet_t));
        size=recv(sockBck,&request,sizeof(packet_t),0);
        // if backup server crash
        if(size>0 && request.operation==PACKET_OPERATION_RECOVER)
        {
            fseek(log,SEEK_SET,0);
            do
            {
                char operation[100];
                val_t val;
                packet_t req;
                req.operation=PACKET_OPERATION_RECOVER;
                size=send(sockBck,&req,sizeof(packet_t),0);
                fscanf(log,"%s %ld\n",operation,&val);
                if(strcmp(operation,"insert")==0)
                {
                    req.operation=PACKET_OPERATION_INSERT;
                    req.val=val;
                    printf("insert %ld\n",val);
                    size=send(sockBck,&req,sizeof(packet_t),0);
                }
                else if(strcmp(operation,"remove")==0)
                {
                    req.operation=PACKET_OPERATION_REMOVE;
                    req.val=val;
                    printf("remove %ld\n",val);
                    size=send(sockBck,&req,sizeof(packet_t),0);
                }
            } while (!feof(log) && size>0);
        }

        // receive client's request
        memset(&request,0,sizeof(packet_t));
        memset(&response,0,sizeof(packet_t));
        size=recv(sockClt,&request,sizeof(packet_t),0);
        if(request.operation!=PACKET_OPERATION_LOOKUP)
        {
            size=send(sockBck,&request,sizeof(packet_t),0); // send the request from the client to the backup server
        }

        // handle request from the client
        switch(request.operation)
        {
            case PACKET_OPERATION_REMOVE:
                printf("remove %ld\n",request.val);
                fprintf(log,"remove %ld\n",request.val);
                set_remove_l(set,request.val,0);
                break;
            case PACKET_OPERATION_INSERT:
                printf("insert %ld\n",request.val);
                fprintf(log,"insert %ld\n",request.val);
                set_add_l(set,request.val,0);
                break;
            case PACKET_OPERATION_LOOKUP:
                printf("lookup %ld\n",request.val);
                response.val=request.val;
                response.persistence=set_contains_l(set,request.val,0);
                size=send(sockClt,&response,sizeof(packet_t),0);
                break;
            case PACKET_OPERATION_TERMINATE:
                printf("terminate\n");
                break;
            default:
                response.operation=request.operation=PACKET_OPERATION_TERMINATE;
                size=send(sockClt,&response,sizeof(packet_t),0);
                break;
        }
    } while (request.operation!=PACKET_OPERATION_TERMINATE && size>0);
    close(sockClt);
    close(sockPri);
    fclose(log);
    set_delete_l(set);
    return 0;
}

