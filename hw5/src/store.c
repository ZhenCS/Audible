#include "store.h"
#include "data.h"
#include "transaction.h"
#include "hw5.h"
#include "debug.h"
#include "csapp.h"


/*struct map store_map;

void store_init(void){
    store_map.table = Malloc(sizeof(MAP_ENTRY *) * NUM_BUCKETS);
    store_map.num_buckets = NUM_BUCKETS;
    pthread_mutex_init(&store_map.mutex, NULL);

    for(int i = 0; i < NUM_BUCKETS; i++){
        store_map.table[i] = NULL;
    }

    debug("Initializing object store");
}

void store_fini(void){
    pthread_mutex_lock(&store_map.mutex);
    for(int i = 0; i < NUM_BUCKETS; i++){
        //free versions
        MAP_ENTRY *entry = store_map.table[i];

        while(entry != NULL){
            if(entry->versions != NULL)
                version_dispose_all(entry->versions);
            if(entry->key != NULL)
                free(entry->key);

            entry = entry->next;
            free(entry);
        }
    }
    free(store_map.table);
    pthread_mutex_unlock(&store_map.mutex);
}

TRANS_STATUS store_put(TRANSACTION *tp, KEY *key, BLOB *value){
    debug("Put mapping (key=%p [%s] -> value=%p [%s]) in store for transaction %i",
            &key, key->blob->content, &value, value->content, tp->id);

    MAP_ENTRY *entry = find_map_entry(key);
    VERSION *version = add_version(entry, tp, value);

    if(version == NULL)
        return TRANS_ABORTED;

    return TRANS_PENDING;
}

TRANS_STATUS store_get(TRANSACTION *tp, KEY *key, BLOB **valuep){
    debug("Get mapping of key=%p [%s] in store for transaction %i", &key, key->blob->content, tp->id);

    MAP_ENTRY *entry = find_map_entry(key);
    VERSION *version = add_version(entry, tp, NULL);

    if(version == NULL)
        return TRANS_ABORTED;

    if(version->blob == NULL)
        (*valuep) = NULL;
    else (*valuep) = version->blob;

    return TRANS_PENDING;
}

void store_show(void){
    fprintf(stderr, "CONTENTS OF STORE:\n");

    for(int i = 0; i < NUM_BUCKETS; i++){
        fprintf(stderr, "%i: ", i);

        MAP_ENTRY *entry = store_map.table[i];

        while(entry != NULL){
            fprintf(stderr, "\t{key: %p [%s], versions: ", &entry->key, entry->key->blob->content);
            version_show_all(entry->versions);
            fprintf(stderr, "}");

            entry = entry->next;
            if(entry != NULL)
                fprintf(stderr, "\n");
        }

        fprintf(stderr, "\n");
    }
}

int get_map_index(KEY *key){
    return abs(key->hash % NUM_BUCKETS);
}

MAP_ENTRY *find_map_entry(KEY *key){
    pthread_mutex_lock(&store_map.mutex);
    MAP_ENTRY *entry;
    for(int i = 0; i < NUM_BUCKETS; i++){
        entry = store_map.table[i];

        while(entry != NULL){
            if(key_compare(entry->key, key) == 0){
                debug("Matching entry exists, dispose of redundant key %p [%s]", &key, key->blob->content);
                key_dispose(key);
                pthread_mutex_unlock(&store_map.mutex);
                return entry;
            }

            entry = entry->next;
        }
    }
    pthread_mutex_unlock(&store_map.mutex);

    int index = get_map_index(key);
    debug("Create new map entry for key %p [%s] at table index %i", &key, key->blob->content, index);
    MAP_ENTRY *newEntry = map_entry_create(key);

    pthread_mutex_lock(&store_map.mutex);
    if(store_map.table[index]  == NULL)
        store_map.table[index] = newEntry;
    else{
        entry = store_map.table[index];

        while(entry->next != NULL){
            entry = entry->next;
        }

        entry->next = newEntry;
    }
    pthread_mutex_unlock(&store_map.mutex);

    return newEntry;
}

MAP_ENTRY *map_entry_create(KEY *key){
    MAP_ENTRY *entry;
    if((entry = (MAP_ENTRY *)malloc(sizeof(MAP_ENTRY))) < 0){
        errorMessage("Unable to create map entry");
        exit(EXIT_FAILURE);
    }

    entry->key = key;
    entry->versions = NULL;
    entry->next = NULL;

    return entry;
}
void add_version_list(MAP_ENTRY *entry, VERSION *prev, VERSION *new){
    if(prev == NULL)
        entry->versions = new;
    else{
        prev->next = new;
        new->prev = prev;
    }
}

void remove_version_list(MAP_ENTRY *entry, VERSION *version){
    if(version->prev == NULL){
        entry->versions = version->next;

        if(version->next != NULL)
            version->next->prev = NULL;

    }else{
        version->prev->next = version->next;

        if(version->next != NULL)
            version->next->prev = version->prev;
    }
}

void replace_version_list(MAP_ENTRY *entry, VERSION *new, VERSION *old){
    if(old->prev == NULL){
        entry->versions = old;
    }else{
        old->prev = old->prev;
        old->prev->next = old;
    }
    old->next = old->next;
    if(old->next != NULL)
        old->next->prev = old;
}

void garbageCollector(MAP_ENTRY *entry){
    VERSION *lastCommited = NULL;
    VERSION *version = entry->versions;
    VERSION *dispose = NULL;
    pthread_mutex_lock(&store_map.mutex);
    while(version != NULL){
        if(trans_get_status(version->creator) == TRANS_ABORTED){
            dispose = version;
            version = version->next;

            remove_version_list(entry, dispose);
            version_dispose(dispose);
        }else if(trans_get_status(version->creator) == TRANS_COMMITTED){
            lastCommited = version;
            version = version->next;
        }else version = version->next;
    }

    if(lastCommited != NULL){
        version = entry->versions;

        while(version->creator->id < lastCommited->creator->id){
            dispose = version;
            version = version->next;

            remove_version_list(entry, version);
            version_dispose(dispose);
        }
    }
    pthread_mutex_unlock(&store_map.mutex);
}

VERSION *add_version(MAP_ENTRY *entry, TRANSACTION *tp, BLOB *value){
    garbageCollector(entry);
    if(value == NULL)
        debug("Trying to get version in map entry for key %p [%s]", &entry->key, entry->key->blob->content);
    else
        debug("Trying to put version in map entry for key %p [%s]", &entry->key, entry->key->blob->content);

    VERSION *version = NULL;
    VERSION *vp = entry->versions;
    while(vp != NULL){
        debug("Examine version %p for key %p [%s]", &vp, &entry->key, entry->key->blob->content);
        if(vp->creator->id == tp->id){
            version = vp;
            break;
        }
        vp = vp->next;
    }

    if(entry->versions == NULL){
        version = version_create(tp, value);
        add_version_list(entry, NULL, version);
        debug("Add new version for key %p [%s]", &entry->key, entry->key->blob->content);
        debug("No previous version");
        return version;
    }

    if(version != NULL){
        vp = entry->versions;
        while(vp->next != NULL){
            vp = vp->next;
        }

        if(tp->id < vp->creator->id){
            debug("Current transaction ID (%i) is less than version creator (%i) -- aborting", tp->id, vp->creator->id);
            return NULL;
        }

        if(value == NULL){ //get request
            version = version_create(tp, vp->blob);
            blob_ref(vp->blob, "new version created");
        }else
            version = version_create(tp, value);

        debug("Replace existing version for key %p [%s]", &entry->key, entry->key->blob->content);
        debug("tp = %p (%i), creator = %p (%i)", &version->creator, version->creator->id, &vp->creator, vp->creator->id);

        replace_version_list(entry, version, vp);
        version_dispose(vp);
        return version;

    }else{
        vp = entry->versions;
        while(vp->next != NULL){
            vp = vp->next;
        }

        if(tp->id < vp->creator->id){
            debug("Current transaction ID (%i) is less than version creator (%i) -- aborting", tp->id, vp->creator->id);
            return NULL;
        }

        if(value == NULL){ //get request
            version = version_create(tp, vp->blob);
            blob_ref(vp->blob, "new version created");
        }else
            version = version_create(tp, value);

        debug("Add new version for key %p [%s]", &entry->key, entry->key->blob->content);
        if(vp->blob == NULL)
            debug("Previous version is %p (NULL blob)", &vp);
        else
            debug("Previous version is %p [%s]", &vp, vp->blob->content);

        if(trans_get_status(vp->creator) == TRANS_PENDING){
            trans_add_dependency(version->creator, vp->creator);
        }

        add_version_list(entry, vp, version);
        return version;
    }

    return NULL;
}*/