#ifndef _REQUEST_HPP
#define _REQUEST_HPP

#include <stdint.h>
#include <intset.h>

#define REQUEST_SOURCE_SERVER 1
#define REQUEST_SOURCE_CLIENT 2
#define REQUEST_OPERATION_INSERT 1
#define REQUEST_OPERATION_REMOVE 2
#define REQUEST_OPERATION_LOOKUP 3
#define REQUEST_OPERATION_TERMINATE 4

#pragma pack(1)
typedef struct request
{
    uint8_t source; // identify where the request is from (client or server)
    uint8_t operation; // the operation of request
    val_t val;
} request_t;
#pragma pack()

#endif // _REQUEST_HPP