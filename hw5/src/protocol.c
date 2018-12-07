#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "protocol.h"
#include "debug.h"
#include "csapp.h"
#include "hw5.h"


int proto_send_packet(int fd, XACTO_PACKET *pkt, void *data){
    if(pkt == NULL)
        return -1;
    int size = pkt->size;
    pkt->size = htonl(pkt->size);
    pkt->timestamp_sec = htonl(pkt->timestamp_sec);
    pkt->timestamp_nsec = htonl(pkt->timestamp_nsec);

    if(rio_writen(fd, pkt, sizeof(XACTO_PACKET)) < sizeof(XACTO_PACKET))
        return -1;

    if(size > 0 && data != NULL){
        if(rio_writen(fd, data, size) < size)
            return -1;
    }

    return 0;
}

int proto_recv_packet(int fd, XACTO_PACKET *pkt, void **datap){
    if(pkt == NULL)
        return -1;

    int read = 0;

    if((read = rio_readn(fd, pkt, sizeof(XACTO_PACKET))) < sizeof(XACTO_PACKET)){
        debug("EOF on fd %i", fd);
        return -1;
    }

    pkt->size = ntohl(pkt->size);
    pkt->timestamp_sec = ntohl(pkt->timestamp_sec);
    pkt->timestamp_nsec = ntohl(pkt->timestamp_nsec);

    if(pkt->size > 0 && datap != NULL){

        if(((*datap) = malloc(pkt->size + 1)) < 0){
            errorMessage("Unable to malloc data");
            return -1;
        }

        if(rio_readn(fd, (*datap), pkt->size) < pkt->size)
            return -1;

    }else if(pkt->null > 0)
        datap = NULL;

    return 0;
}