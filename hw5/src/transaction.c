#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "transaction.h"
#include "debug.h"
#include "csapp.h"
#include "hw5.h"



void trans_init(void){
    trans_list.next = &trans_list;
    trans_list.prev = &trans_list;
    debug("Initializing transaction manager");
}

void trans_fini(void){
    TRANSACTION *trans = trans_list.next;

    while(trans != &trans_list){
        trans_abort(trans);
        trans = trans->next;
    }

}

TRANSACTION *trans_create(void){
    TRANSACTION *trans;

    if((trans = (TRANSACTION *)malloc(sizeof(TRANSACTION))) < 0){
        errorMessage("Unable to create transaction");
        return NULL;
    }
    static int id = 0;

    trans->id = id;
    trans->refcnt = 0;
    trans->status = TRANS_PENDING;
    trans->depends = NULL;
    trans->waitcnt = 0;
    trans->next = NULL;
    trans->prev = NULL;

    sem_init(&trans->sem, 0, 1);
    pthread_mutex_init(&trans->mutex, NULL);

    add_trans_list(trans);
    debug("Created new transaction %i", id);
    trans_ref(trans, "transaction created");


    id++;
    return trans;
}

TRANSACTION *trans_ref(TRANSACTION *tp, char *why){
    pthread_mutex_lock(&tp->mutex);

    if(why != NULL)
        debug("Increased ref count on transaction %i (%i -> %i) because %s",
                tp->id, tp->refcnt, tp->refcnt + 1, why);

    tp->refcnt += 1;
    pthread_mutex_unlock(&tp->mutex);

    return tp;
}

void trans_unref(TRANSACTION *tp, char *why){
    pthread_mutex_lock(&tp->mutex);

    if(why != NULL)
        debug("Decreased ref count on transaction %i (%i -> %i) because %s",
                tp->id, tp->refcnt, tp->refcnt - 1, why);

    tp->refcnt -= 1;

    /*if(tp->refcnt == 0){
        debug("Free transaction %i", tp->id);
        remove_trans_list(tp);
        free(tp);
        return;
    }*/
    pthread_mutex_unlock(&tp->mutex);
}

void trans_add_dependency(TRANSACTION *tp, TRANSACTION *dtp){
    P(&tp->sem);
    DEPENDENCY *depend = tp->depends;
    DEPENDENCY *newDepend;

    if((newDepend = dependency_create(dtp)) == NULL)
        exit(EXIT_FAILURE);

    newDepend->next = depend;
    tp->depends = newDepend;
    debug("transaction %i has a dependecy of %i", tp->id, dtp->id);
    trans_ref(dtp, "transaction has a new dependancy");
    V(&tp->sem);
}

TRANS_STATUS trans_commit(TRANSACTION *tp){
    TRANS_STATUS status = TRANS_COMMITTED;
    P(&tp->sem);
    debug("transaction %i trying to commit", tp->id);
    DEPENDENCY *depend = tp->depends;

    while(depend != NULL){
        debug("transaction %i checking status of dependecy %i", tp->id, depend->trans->id);
        if(trans_get_status(depend->trans) == TRANS_ABORTED){
            status = TRANS_ABORTED;
            break;
        }else if(trans_get_status(depend->trans) == TRANS_PENDING){
            pthread_mutex_lock(&depend->trans->mutex);
            depend->trans->waitcnt += 1;
            pthread_mutex_unlock(&depend->trans->mutex);
            debug("transaction %i waiting on dependecy %i", tp->id, depend->trans->id);

            while(trans_get_status(depend->trans) == TRANS_PENDING){
                P(&depend->trans->sem);
                V(&depend->trans->sem);
            }

            if(trans_get_status(depend->trans) == TRANS_ABORTED)
                status = TRANS_ABORTED;
            debug("transaction %i finished waiting for dependecy %i", tp->id, depend->trans->id);
        }

        depend = depend->next;
    }

    if(status == TRANS_COMMITTED)
        debug("transaction %i commits", tp->id);
    else if(status == TRANS_ABORTED)
        debug("transaction %i aborts because a dependecy has aborted", tp->id);

    tp->status = status;
    releaseDependents(tp);
    trans_unref(tp, "attempted to commit transaction");
    V(&tp->sem);

    return tp->status;

}

TRANS_STATUS trans_abort(TRANSACTION *tp){
    if(trans_get_status(tp) == TRANS_COMMITTED){
        errorMessage("FATAL TRANSACTION ERROR");
        exit(EXIT_FAILURE);
    }

    debug("transaction %i trying to abort", tp->id);

    if(trans_get_status(tp) == TRANS_PENDING){
        P(&tp->sem);
        tp->status = TRANS_ABORTED;

        DEPENDENCY *depend = tp->depends;

        while(depend != NULL){
            trans_abort(depend->trans);
            depend = depend->next;
        }

        debug("transaction %i has aborted", tp->id);
        trans_unref(tp, "transaction aborted");
        releaseDependents(tp);
        V(&tp->sem);
    }else{
        debug("transaction %i has already aborted", tp->id);
    }

    return TRANS_ABORTED;
}

TRANS_STATUS trans_get_status(TRANSACTION *tp){
    TRANS_STATUS status;

    pthread_mutex_lock(&tp->mutex);
    status = tp->status;
    pthread_mutex_unlock(&tp->mutex);

    return status;
}

void trans_show(TRANSACTION *tp){
    fprintf(stderr, "[id:%i status:%i ref:%i] ", tp->id, tp->status, tp->refcnt);
}

void trans_show_all(void){
    TRANSACTION *trans = trans_list.next;

    while(trans != &trans_list){
        trans_show(trans);
        trans = trans->next;
    }
    fprintf(stderr, "\n");
}




DEPENDENCY *dependency_create(TRANSACTION *tp){
    DEPENDENCY *depend;
    if((depend = (DEPENDENCY *)malloc(sizeof(DEPENDENCY))) < 0){
        errorMessage("Unable to create dependency");
        return NULL;
    }

    depend->trans = tp;
    depend->next = NULL;

    return depend;
}

void releaseDependents(TRANSACTION *tp){
    if(tp == NULL)
        return;

    debug("Released %i waiters on transaction %i", tp->waitcnt, tp->id);

    pthread_mutex_lock(&tp->mutex);
    while(tp->waitcnt > 0){
        tp->waitcnt -= 1;
        V(&tp->sem);
    }
    pthread_mutex_unlock(&tp->mutex);
}

void add_trans_list(TRANSACTION *tp){
    tp->next = &trans_list;
    tp->prev = trans_list.prev;

    trans_list.prev->next = tp;
    trans_list.prev = tp;
}

void remove_trans_list(TRANSACTION *tp){
    tp->prev->next = tp->next;
    tp->next->prev = tp->prev;
}