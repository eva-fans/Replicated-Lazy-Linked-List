#include "packet.h"
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <memory.h>
#include <unistd.h>
#include <pthread.h>

intset_l_t *set;
pthread_mutex_t mutex;

void* client_thread(void* arg);

int main()
{
    int sockBck, sockPri; // backup server socket, primary server socket
    struct sockaddr_in addr; // address struct 
    socklen_t socklen; // length of socket address
    pthread_t pthread; // the thread receive the request of the client
    set=set_new_l();
    pthread_mutex_init(&mutex,NULL); // the mutes of set
    // create clieant thread
    if(pthread_create(&pthread,NULL,client_thread,NULL)) 
    {
        printf("client thread create fails");
        return 1;
    }

    // create backup server socket
    if((sockBck=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
    {
        return 1;
    }

    // lisent to primary server
    memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(BACKUP_SERVER_PORT);
    if(bind(sockBck,(struct sockaddr*)&addr,sizeof(struct sockaddr_in))<0)
    {
        close(sockBck);
        return 1;
    }

    if(listen(sockBck,5)<0)
    {
        close(sockBck);
        return 1;
    }

    memset(&addr,0,sizeof(struct sockaddr_in));
    socklen=sizeof(struct sockaddr_in);
    if((sockPri=accept(sockBck,(struct sockaddr*)&addr,&socklen))<0)
    {
        close(sockBck);
        return 1;
    }

    packet_t request;
    packet_t response;
    ssize_t size;

    // send a recovery request to primary server
    request.operation=PACKET_OPERATION_RECOVER;
    send(sockPri,&request,sizeof(packet_t),0); // send backup server recover request

    // execute recover
    do
    {
        memset(&request,0,sizeof(packet_t));
        memset(&response,0,sizeof(packet_t));
        socklen=sizeof(struct sockaddr_in);
        size=recv(sockPri,&request,sizeof(packet_t),0);
        switch(request.operation)
        {
            case PACKET_OPERATION_INSERT:
                pthread_mutex_lock(&mutex);
                set_add_l(set,request.val,0);
                pthread_mutex_unlock(&mutex);
                printf("insert %ld\n",request.val);
                break;
            case PACKET_OPERATION_REMOVE:
                pthread_mutex_lock(&mutex);
                set_remove_l(set,request.val,0);
                pthread_mutex_unlock(&mutex);
                printf("remove %ld\n",request.val);
                break;
            case PACKET_OPERATION_RECOVER:
                pthread_mutex_lock(&mutex);
                set_delete_l(set);
                set=set_new_l();
                pthread_mutex_unlock(&mutex);
                break;
            case PACKET_OPERATION_TERMINATE:
                // do nothing
                break;
            default:
                response.error=1;
                break;
        }
    } while (request.operation!=PACKET_OPERATION_TERMINATE && size>0);

    close(sockPri);
    close(sockBck);
    set_delete_l(set);
    return 0;
}

void* client_thread(void* arg)
{
    int sockBck; // backup server socket
    int sockClt; // client socket
    struct sockaddr_in addr; // socket address struct
    socklen_t socklen; // the length of socket
    if((sockBck=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
    {
        return 0;
    }

    // accept a client socket
    memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(BACKUP_CLIENT_PORT);
    if(bind(sockBck,(struct sockaddr*)&addr,sizeof(struct sockaddr_in))<0)
    {
        close(sockBck);
        return 0;
    }
    if(listen(sockBck,5)<0)
    {
        close(sockBck);
        return 0;
    }

    memset(&addr,0,sizeof(struct sockaddr_in));
    socklen=sizeof(struct sockaddr_in);
    if((sockClt=accept(sockBck,(struct sockaddr*)&addr,&socklen))<0)
    {
        close(sockBck);
        return 0;
    }

    packet_t request;
    packet_t response;
    ssize_t size;

    do
    {
        memset(&request,0,sizeof(packet_t)); // set memory of request to 0
        memset(&response,0,sizeof(packet_t)); // set memory of response to 0
        socklen=sizeof(struct sockaddr_in); // set memory of socket address struct to 0
        size=recv(sockClt,&request,sizeof(packet_t),0); // receive client's request
        // handle request of client
        switch(request.operation)
        {
            case PACKET_OPERATION_LOOKUP: // look up the value
                response.val=request.val;
                pthread_mutex_lock(&mutex);
                response.persistence=set_contains_l(set,request.val,0);
                pthread_mutex_unlock(&mutex);
                printf("lookup %ld\n",request.val);
                break;
            default:
                response.error=1;
                break;
        }
        size = send(sockClt,&response,sizeof(packet_t),0);
    } while (size>0);

    close(sockBck);
    close(sockClt);
    return 0;
}



