#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include "client_registry.h"
#include "debug.h"
#include "hw5.h"
#include "csapp.h"


void initClientList(CLIENT_REGISTRY *cr){
    for(int i = 0; i < MAX_CLIENTS; i++){
        cr->clientList[i] = -1;
    }
    debug("Initializing client registry");
}


CLIENT_REGISTRY *creg_init(){
    CLIENT_REGISTRY *client_registry;
    if((client_registry = (CLIENT_REGISTRY *)malloc(sizeof(CLIENT_REGISTRY))) < 0){
        errorMessage("Unable to initialize client register");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(&client_registry->mutex, NULL);
    sem_init(&client_registry->sem, 0, 1);
    client_registry->clients = 0;
    initClientList(client_registry);

    return client_registry;
}

void creg_fini(CLIENT_REGISTRY *cr){
    free(cr);
}

void creg_register(CLIENT_REGISTRY *cr, int fd){
    P(&cr->sem);
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(cr->clientList[i] == -1){
            cr->clientList[i] = fd;

            pthread_mutex_lock(&cr->mutex);
            cr->clients += 1;
            pthread_mutex_unlock(&cr->mutex);
            break;
        }
    }
    debug("Register client %i (total connected: %i)", fd, cr->clients);
    V(&cr->sem);
}

void creg_unregister(CLIENT_REGISTRY *cr, int fd){
    P(&cr->sem);
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(cr->clientList[i] == fd){
            cr->clientList[i] = -1;

            pthread_mutex_lock(&cr->mutex);
            cr->clients -= 1;
            pthread_mutex_unlock(&cr->mutex);
            break;
        }
    }
    debug("Unregister client %i (total connected: %i)", fd, cr->clients);
    V(&cr->sem);
}

void creg_wait_for_empty(CLIENT_REGISTRY *cr){
    while(1){
        P(&cr->sem);
        if(cr->clients > 0)
            V(&cr->sem);
        else break;
    }

    V(&cr->sem);

}

void creg_shutdown_all(CLIENT_REGISTRY *cr){
    P(&cr->sem);
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(cr->clientList[i] > -1)
            shutdown(cr->clientList[i], SHUT_RDWR);
    }
    V(&cr->sem);
}