#include "intset.h"
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <memory.h>
#include <unistd.h>

#define BACKUP_SERVER_IP ("127.0.0.1")
#define BACKUP_SERVER_PORT 8081
#define PRIMARY_SERVER_PORT 8080
#define PACKET_SOURCE_SERVER 1
#define PACKET_SOURCE_CLIENT 2
#define PACKET_OPERATION_TERMINATE 1
#define PACKET_OPERATION_INSERT 2
#define PACKET_OPERATION_REMOVE 3
#define PACKET_OPERATION_LOOKUP 4


#define IN
#define OUT

#pragma pack(1)
typedef struct _packet
{
    uint8_t type;
    uint8_t operation;
    uint8_t source;
    uint8_t persistence;
    val_t val;
} packet_t;
#pragma pack()

ssize_t get_packet(int IN sock, struct sockaddr OUT *addr, socklen_t IN OUT *socklen, packet_t OUT *packet);

int main(void)
{
    
    int sockPri;
    int sockBck;
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
        printf("connecting to backup server fails\n");
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
    if(accept(sockPri, (struct sockaddr*)&addr,&socklen)<0)
    {
        close(sockPri);
        set_delete_l(set);
        printf("accepting client's connection fails\n");
        return 1;
    }
    packet_t packet;
    ssize_t size;
    do
    {
        socklen_t socklen=sizeof(struct sockaddr_in);
        size=get_packet(sockPri, (struct sockaddr*)&addr, &socklen, &packet);
        switch(packet.operation)
        {
            case PACKET_OPERATION_REMOVE:
                set_remove_l(set,packet.val,0);
                break;
            case PACKET_OPERATION_INSERT:
                set_remove_l(set,packet.val,0);
                break;
            case PACKET_OPERATION_LOOKUP:
                set_contains_l(set,packet.val,0);
                break;
            case PACKET_OPERATION_TERMINATE:
                // do nothing
                break;
            default:
                close(sockPri);
                set_delete_l(set);
                return 1;
        }
    } while (packet.operation!=PACKET_OPERATION_TERMINATE && size>0);
    close(sockPri);
    set_delete_l(set);
    return 0;
}

ssize_t get_packet(int IN sock, struct sockaddr OUT *addr, socklen_t IN OUT *socklen, packet_t OUT *packet)
{
    return recvfrom(sock,packet,sizeof(packet_t),0,addr,socklen);
}