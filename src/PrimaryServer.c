#include "packet.h"
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <memory.h>
#include <unistd.h>



int main(void)
{
    
    int sockPri,sockBck,sockClt;
    intset_l_t *set=set_new_l();
    if((sockPri=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0 || (sockBck=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
    {
        printf("socket function fails\n");
        return 1;
    }
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_addr.s_addr=inet_addr(BACKUP_SERVER_IP);
    addr.sin_family=AF_INET;
    addr.sin_port=htons(BACKUP_SERVER_PORT);
    if(connect(sockBck,(struct sockaddr*)&addr,sizeof(struct sockaddr_in))<0)
    {
        close(sockPri);
        close(sockBck);
        printf("connecting to backup server fail\n");
        return 1;
    }
    memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(PRIMARY_SERVER_PORT);
    if(bind(sockPri,(struct sockaddr*)&addr,sizeof(struct sockaddr_in))<0)
    {
        close(sockPri);
        printf("bind socket to localhost fails\n");
        return 1;
    }
    if(listen(sockPri,5)<0)
    {
        close(sockPri);
        printf("listen to the port fails\n");
        return 1;
    }
    memset(&addr,0,sizeof(struct sockaddr_in));
    socklen_t socklen=sizeof(struct sockaddr_in);   
    if((sockClt=accept(sockPri, (struct sockaddr*)&addr,&socklen))<0)
    {
        close(sockPri);
        set_delete_l(set);
        printf("accepting client's connection fails\n");
        return 1;
    }
    packet_t request;
    packet_t response;
    ssize_t size;
    do
    {
        size=get_packet(sockPri, &request);
        switch(request.operation)
        {
            case PACKET_OPERATION_REMOVE:
                set_remove_l(set,request.val,0);
                break;
            case PACKET_OPERATION_INSERT:
                set_add_l(set,request.val,0);
                break;
            case PACKET_OPERATION_LOOKUP:
                response.persistence=set_contains_l(set,request.val,0);
                break;
            case PACKET_OPERATION_TERMINATE:
                // do nothing
                break;
            default:
                response.operation=request.operation=PACKET_OPERATION_TERMINATE;
                break;
        }
        size=send(sockBck,&request,sizeof(packet_t),0);
        size=send(sockClt,&response,sizeof(packet_t),0);
    } while (request.operation!=PACKET_OPERATION_TERMINATE && size>0);
    close(sockClt);
    close(sockPri);
    set_delete_l(set);
    return 0;
}

