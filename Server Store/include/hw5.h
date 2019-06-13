#ifndef HW5_H
#define HW5_H

#define MAX_CLIENTS 1024
#define DEBUGMODE 1
#include <semaphore.h>
#include "protocol.h"
#include "transaction.h"
#include "data.h"
#include "store.h"

typedef struct client_registry{
    sem_t sem;
    pthread_mutex_t mutex;
    int clients;
    int clientList[MAX_CLIENTS];
}CLIENT_REGISTRY;

int getFlag(char * flag, int argc, char **argv);
void errorMessage(char *message);



void add_trans_list(TRANSACTION *tp);
void remove_trans_list(TRANSACTION *tp);
DEPENDENCY *dependency_create(TRANSACTION *tp);
void releaseDependents(TRANSACTION *tp);

XACTO_PACKET *packet_create(XACTO_PACKET_TYPE type, TRANS_STATUS status);
TRANS_STATUS xacto_put(int fd, TRANSACTION *trans);
TRANS_STATUS xacto_get(int fd, TRANSACTION *trans, BLOB **valueBlob);
TRANS_STATUS xacto_commit(int fd, TRANSACTION *trans);
int xacto_reply(int fd, XACTO_PACKET_TYPE type, TRANS_STATUS status, BLOB *blob);
void set_pkt_time(XACTO_PACKET *pkt);

void version_dispose_all(VERSION *vp);
void version_show_all(VERSION *vp);
MAP_ENTRY *map_entry_create(KEY *key);
int get_map_index(KEY *key);
MAP_ENTRY *find_map_entry(KEY *key);
VERSION *add_version(MAP_ENTRY *entry, TRANSACTION *tp, BLOB *value);
void garbageCollector(MAP_ENTRY *entry);
void remove_version_list(MAP_ENTRY *entry, VERSION *version);
void replace_version_list(MAP_ENTRY *entry, VERSION *new, VERSION *old);


#endif