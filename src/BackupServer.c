#include "packet.h"
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <memory.h>
#include <unistd.h>
#include <pthread.h>

intset_l_t *set;

void* client_thread(void* arg);

int main()
{
    int sockBck;
    int sockPri;
    struct sockaddr_in addr;
    socklen_t socklen;
    pthread_t pthread;
    set=set_new_l();
    if(pthread_create(&pthread,NULL,client_thread,NULL))
    {
        printf("client thread create fails");
        return 1;
    }

    if((sockBck=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
    {
        return 1;
    }
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
    do
    {
        memset(&request,0,sizeof(packet_t));
        memset(&response,0,sizeof(packet_t));
        socklen=sizeof(struct sockaddr_in);
        size=get_packet(sockPri, &request);
        switch(request.operation)
        {
            case PACKET_OPERATION_INSERT:
                set_add_l(set,request.val,0);
                break;
            case PACKET_OPERATION_LOOKUP:
                set_contains_l(set,request.val,0);
                break;
            case PACKET_OPERATION_REMOVE:
                set_remove_l(set,request.val,0);
                break;
            case PACKET_OPERATION_TERMINATE:
                // do nothing
                break;
            default:
                response.error=1;
                break;
        }
        size = send(sockPri,&response,sizeof(packet_t),0);
    } while (request.operation!=PACKET_OPERATION_TERMINATE && size>0);

    close(sockPri);
    close(sockBck);
    set_delete_l(set);
    return 0;
}

void* client_thread(void* arg)
{
    int sockBck;
    int sockClt;
    struct sockaddr_in addr;
    socklen_t socklen;
    if((sockBck=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
    {
        return 0;
    }
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
        memset(&request,0,sizeof(packet_t));
        memset(&response,0,sizeof(packet_t));
        socklen=sizeof(struct sockaddr_in);
        size=get_packet(sockClt, &request);
        switch(request.operation)
        {
            case PACKET_OPERATION_LOOKUP:
                set_contains_l(set,request.val,0);
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



