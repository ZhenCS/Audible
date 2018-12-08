#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include "server.h"
#include "transaction.h"
#include "protocol.h"
#include "data.h"
#include "store.h"
#include "csapp.h"
#include "debug.h"
#include "hw5.h"

CLIENT_REGISTRY *client_registry;

void *xacto_client_service(void *arg){
    int fd = *(int *)arg;
    free(arg);

    debug("[%i] Starting client service", fd);

    pthread_detach(pthread_self());
    creg_register(client_registry, fd);



    TRANSACTION *trans = trans_create();
    XACTO_PACKET *pkt = packet_create(0, 0);
    TRANS_STATUS status = TRANS_PENDING;
    BLOB **getBlob = Malloc(sizeof(BLOB *));

    while(1){
        if(proto_recv_packet(fd, pkt, NULL) < 0){
            free(pkt);
            free(getBlob);
            debug("[%i] Ending client service ", fd);
            trans_abort(trans);
            creg_unregister(client_registry, fd);
            return NULL;
        }

        (*getBlob) = NULL;

        if(pkt->type == XACTO_PUT_PKT){
            status = xacto_put(fd, trans);
        }else if(pkt->type == XACTO_GET_PKT){
            status = xacto_get(fd, trans, getBlob);
        }else if(pkt->type == XACTO_COMMIT_PKT){
            status = xacto_commit(fd, trans);
        }

        xacto_reply(fd, pkt->type, status, *getBlob);

        store_show();
        trans_show_all();

        if(pkt->type == XACTO_COMMIT_PKT || status == TRANS_ABORTED){
            free(pkt);
            free(getBlob);
            debug("[%i] Ending client service ", fd);
            creg_unregister(client_registry, fd);

            if(status == TRANS_ABORTED)
                trans_abort(trans);

            return NULL;
        }
    }
    return NULL;
}

int xacto_reply(int fd, XACTO_PACKET_TYPE type, TRANS_STATUS status, BLOB *blob){
    XACTO_PACKET *response;

    response = packet_create(XACTO_REPLY_PKT, status);

    set_pkt_time(response);
    proto_send_packet(fd, response, NULL);

    if(type == XACTO_GET_PKT){
        response->type = XACTO_DATA_PKT;
        set_pkt_time(response);
        if(blob != NULL){
            response->size = blob->size;
            proto_send_packet(fd, response, blob->content);
        }
        else{
            response->null = 1;
            proto_send_packet(fd, response, NULL);
        }
    }

    free(response);
    return 0;
}

void set_pkt_time(XACTO_PACKET *pkt){
    struct timespec timeStamp;
    clock_gettime(CLOCK_MONOTONIC, &timeStamp);

    pkt->timestamp_sec = timeStamp.tv_sec;
    pkt->timestamp_nsec = timeStamp.tv_nsec;
}

TRANS_STATUS xacto_commit(int fd, TRANSACTION *trans){
    debug("[%i] COMMIT packet recieved", fd);
    return trans_commit(trans);

}

TRANS_STATUS xacto_get(int fd, TRANSACTION *trans, BLOB **valueBlob){
    debug("[%i] GET packet recieved", fd);

    XACTO_PACKET *keyPkt = packet_create(0, 0);
    void **key = Malloc(sizeof(void *));
    (*key) = NULL;

    TRANS_STATUS status = TRANS_PENDING;

    if(proto_recv_packet(fd, keyPkt, key) < 0){
        errorMessage("Unable to recieve key packet");
        return TRANS_ABORTED;
    }

    if((*key) != NULL)
        debug("[%i] Recieved key, size %i", fd, keyPkt->size);
    else debug("[%i] Key is NULL", fd);

    KEY* keyKey = key_create(blob_create(*key, keyPkt->size));
    status = store_get(trans, keyKey, valueBlob);

    if((*key) != NULL) free(*key);
    free(key);
    free(keyPkt);

    if((*valueBlob) != NULL){
        debug("[%i] value is %p [%s]", fd, (*valueBlob)->content, (*valueBlob)->content);
        //blob_unref(*valueBlob, "obtained from store_get");
    }
    else
        debug("[%i] value is NULL", fd);

    return status;
}

TRANS_STATUS xacto_put(int fd, TRANSACTION *trans){
    debug("[%i] PUT packet recieved", fd);
    XACTO_PACKET *keyPkt = packet_create(0, 0);
    XACTO_PACKET *valuePkt = packet_create(0, 0);

    void **key = Malloc(sizeof(void *));
    void **value = Malloc(sizeof(void *));

    (*key) = NULL;
    (*value) = NULL;
    TRANS_STATUS status = TRANS_PENDING;

    if(proto_recv_packet(fd, keyPkt, key) < 0){
        errorMessage("Unable to recieve key packet");
        return TRANS_ABORTED;
    }

    if((*key) != NULL)
        debug("[%i] Recieved key, size %i", fd, keyPkt->size);
    else debug("[%i] Key is NULL", fd);

    if(proto_recv_packet(fd, valuePkt, value) < 0){
        errorMessage("Unable to recieve value packet");
        return TRANS_ABORTED;
    }

    if((*value) != NULL)
        debug("[%i] Recieved value, size %i", fd, valuePkt->size);
    else debug("[%i] Value is NULL", fd);

    KEY *keyKey = key_create(blob_create(*key, keyPkt->size));
    BLOB *valueBlob = blob_create(*value, valuePkt->size);

    status = store_put(trans, keyKey, valueBlob);

    if((*key) != NULL) free(*key);
    if((*value) != NULL) free(*value);
    free(key);
    free(value);
    free(keyPkt);
    free(valuePkt);


    return status;
}

XACTO_PACKET *packet_create(XACTO_PACKET_TYPE type, TRANS_STATUS status){
    XACTO_PACKET *packet;
    if((packet = (XACTO_PACKET *)malloc(sizeof(XACTO_PACKET))) < 0){
        errorMessage("Unable to create packet.");
        return NULL;
    }
    memset(packet, 0, sizeof(XACTO_PACKET));
    packet->type = type;
    packet->status = status;
    packet->null = 0;
    packet->size = 0;
    packet->timestamp_sec = 0;
    packet->timestamp_nsec = 0;

    return packet;
}