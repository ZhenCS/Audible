#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "debug.h"
#include "client_registry.h"
#include "transaction.h"
#include "store.h"
#include "server.h"
#include "csapp.h"
#include "hw5.h"

static void terminate(int status);

int serverfd;

int main(int argc, char* argv[]){
    int portIndex;
    char *port;
    if((portIndex = getFlag("-p", argc, argv)) > 0 && portIndex < argc - 1)
        port = argv[portIndex + 1];

    if(port == 0){
        errorMessage("Provide a valid port number.");
        exit(EXIT_FAILURE);
    }

    // Perform required initializations of the client_registry,
    // transaction manager, and object store.
    client_registry = creg_init();
    trans_init();
    store_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function xacto_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = terminate;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGHUP, &sa, NULL) == -1){
        errorMessage("Unable to install handler.");
        exit(EXIT_FAILURE);
    }

    int *connfdp;
    socklen_t clientlen = sizeof(struct sockaddr_storage);
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    serverfd = Open_listenfd(port);

    while(1){
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(serverfd, (SA *) &clientaddr, &clientlen);
        Pthread_create(&tid, NULL, xacto_client_service, connfdp);
    }


    fprintf(stderr, "You have to finish implementing main() "
	    "before the Xacto server will function.\n");

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);
    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    trans_fini();
    store_fini();
    close(serverfd);
    debug("Xacto server terminating");
    exit(EXIT_SUCCESS);
}

int getFlag(char * flag, int argc, char **argv){

  for(int i = 0; i < argc; i++)
    if(!strcmp(flag, argv[i]))
      return i;

  return -1;
}
