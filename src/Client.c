#include "packet.h"
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <memory.h>
#include <unistd.h>

int main(void)
{
    int sockPri;
    struct sockaddr_in addr;
    if((sockPri=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
    {
        return 1;
    }
    memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    addr.sin_family=AF_INET;
    addr.sin_port=htons(PRIMARY_SERVER_PORT);
    if(connect(sockPri,(struct sockaddr*)&addr,sizeof(struct sockaddr_in))<0)
    {
        close(sockPri);
        return 1;
    }
    packet_t packet;
    packet.operation=PACKET_OPERATION_LOOKUP;
    packet.val=3;
    send(sockPri,&packet,sizeof(packet_t),0);
    memset(&packet,0,sizeof(packet_t));
    recv(sockPri,&packet,sizeof(packet_t),0);
    printf("persistence: %d\n",packet.persistence);
    close(sockPri);
    return 0;
}