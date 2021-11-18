#include "packet.h"

ssize_t get_packet(int IN sock, packet_t OUT *packet)
{
    return recv(sock,&packet,sizeof(packet_t),0);
}