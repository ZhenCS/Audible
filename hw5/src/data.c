#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include "debug.h"
#include "data.h"
#include "hw5.h"

BLOB *blob_create(char *content, size_t size){
    BLOB *blob;
    if((blob = (BLOB *)malloc(sizeof(BLOB))) < 0){
        errorMessage("Unable to create blob");
        return NULL;
    }
    if(pthread_mutex_init(&blob->mutex, NULL) == 1){
        errorMessage("Unable to init mutex of blob");
        return NULL;
    }
    if(content != NULL)
        blob->content = strndup(content, size);
    else blob->content = NULL;
    if(DEBUGMODE){
        if(content != NULL)
            blob->prefix = strndup(content, size);
        else blob->prefix = NULL;
    }
    blob->size = size;
    blob->refcnt = 0;

    if(content != NULL)
        debug("Created blob with content %p, size %i -> %p", &content, (int) size, &blob->content);
    else
        debug("Created blob with content NULL, size 0 -> %p", &blob->content);

    blob_ref(blob, "new blob created");
    return blob;
}

BLOB *blob_ref(BLOB *bp, char *why){
    if(bp == NULL)
        return NULL;

    pthread_mutex_lock(&bp->mutex);
    if(why != NULL)
        debug("Increased reference count of blob %p [%s] (%i -> %i) because %s",
                &bp->content, bp->content, bp->refcnt, bp->refcnt + 1, why);

    bp->refcnt += 1;
    pthread_mutex_unlock(&bp->mutex);

    return bp;

}

void blob_unref(BLOB *bp, char *why){
    if(bp == NULL)
        return;

    pthread_mutex_lock(&bp->mutex);

    if(why != NULL)
        debug("Decreasing reference count of blob %p [%s] (%i -> %i) because %s",
                &bp->content, bp->content, bp->refcnt, bp->refcnt - 1, why);

    bp->refcnt -= 1;

    if(bp->refcnt == 0){
        debug("Free blob %p [%s]", &bp->content, bp->content);
        free(bp->content);
        if(DEBUGMODE)
            free(bp->prefix);

        free(bp);
        return;
    }

    pthread_mutex_unlock(&bp->mutex);
}

int blob_compare(BLOB *bp1, BLOB *bp2){
    if(bp1 == NULL || bp2 == NULL)
        return -1;

    int cmp;
    pthread_mutex_lock(&bp1->mutex);
    pthread_mutex_lock(&bp2->mutex);

    cmp = strcmp(bp1->content, bp2->content);

    pthread_mutex_unlock(&bp1->mutex);
    pthread_mutex_unlock(&bp2->mutex);
    return cmp;
}

int blob_hash(BLOB *bp){
    pthread_mutex_lock(&bp->mutex);
    char *str = strdup(bp->content);
    char *start = str;
    unsigned long hash = 5381;
    int c;
    while((c = *str++)){
        hash = ((hash << 5) + hash) + c;
    }
    pthread_mutex_unlock(&bp->mutex);
    free(start);

    return hash;
}

KEY *key_create(BLOB *bp){
    KEY *key;
    if((key = (KEY *)malloc(sizeof(KEY))) < 0){
        errorMessage("Unable to create key");
        return NULL;
    }

    debug("Created key from blob %p -> %p [%s]", &key, &bp->content, bp->content);
    key->hash = blob_hash(bp);
    key->blob = bp;

    return key;
}

void key_dispose(KEY *kp){
    debug("Dispose of key %p [%s]", &kp,  kp->blob->content);
    blob_unref(kp->blob, "key disposed");
    free(kp);
}

int key_compare(KEY *kp1, KEY *kp2){
    if(kp1 == NULL || kp2 == NULL)
        return -1;

    int cmp;
    cmp = abs(kp1->hash - kp2->hash);
    cmp += blob_compare(kp1->blob, kp2->blob);

    return cmp;
}

VERSION *version_create(TRANSACTION *tp, BLOB *bp){
    VERSION *version;
    if((version = (VERSION *)malloc(sizeof(VERSION))) < 0){
        errorMessage("Unable to create version of blob");
        return NULL;
    }
    if(bp == NULL)
        debug("Created NULL version for transaction %i -> %p", tp->id, &version);
    else
        debug("Created version of blob %p [%s] for transaction %i -> %p", &bp->content, bp->content, tp->id, &version);
    version->creator = tp;
    version->blob = bp;
    version->prev = NULL;
    version->next = NULL;

    trans_ref(tp, "new version created");
    return version;
}

void version_dispose(VERSION *vp){
    debug("Dispose of version %p", &vp);
    trans_unref(vp->creator, "version disposed");
    blob_unref(vp->blob, "version disposed");

    free(vp);
}
//version = non circular double link list
void version_dispose_all(VERSION *vp){
    while(vp != NULL){
        version_dispose(vp);
        vp = vp->next;
    }
}

void version_show_all(VERSION *vp){
    while(vp != NULL){

        if(trans_get_status(vp->creator) == TRANS_COMMITTED){
            if(vp->blob != NULL && vp->blob->content != NULL)
                fprintf(stderr, "{creator=%i (committed), blob=%p [%s]}", vp->creator->id, &vp->blob, vp->blob->content);
            else
                fprintf(stderr, "{creator=%i (committed), (NULL blob)}", vp->creator->id);
        }else if(trans_get_status(vp->creator) == TRANS_ABORTED){
            if(vp->blob != NULL && vp->blob->content != NULL)
                fprintf(stderr, "{creator=%i (aborted), blob=%p [%s]}", vp->creator->id, &vp->blob, vp->blob->content);
            else
                fprintf(stderr, "{creator=%i (aborted), (NULL blob)}", vp->creator->id);
        }else{
            if(vp->blob != NULL && vp->blob->content != NULL)
                fprintf(stderr, "{creator=%i (pending), blob=%p [%s]}", vp->creator->id, &vp->blob, vp->blob->content);
            else
                fprintf(stderr, "{creator=%i (pending), (NULL blob)}", vp->creator->id);
        }

        vp = vp->next;
    }
}