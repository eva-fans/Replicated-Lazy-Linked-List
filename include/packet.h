#include <sys/types.h>
#include <arpa/inet.h>
#include "intset.h"

#define BACKUP_SERVER_IP ("127.0.0.1")
#define BACKUP_SERVER_PORT 8081
#define BACKUP_CLIENT_PORT 8082
#define PRIMARY_SERVER_PORT 8080
#define PACKET_SOURCE_SERVER 1
#define PACKET_SOURCE_CLIENT 2
#define PACKET_OPERATION_TERMINATE 1
#define PACKET_OPERATION_INSERT 2
#define PACKET_OPERATION_REMOVE 3
#define PACKET_OPERATION_LOOKUP 4
#define PACKET_OPERATION_RECOVER 5

#define IN
#define OUT

#pragma pack(1)
typedef struct _packet
{
    uint type;
    uint operation;
    uint source;
    uint persistence;
    uint error;
    val_t val;
} packet_t;
#pragma pack()

ssize_t get_packet(int IN sock, packet_t OUT *packet);