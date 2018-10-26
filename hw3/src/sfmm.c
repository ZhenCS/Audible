/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include "sfmm_helper.h"
//>>4 when assigning
//<<4 when getting the true value
void *sf_malloc(size_t size) {
    if(size == 0)
        return NULL;
    //printf("%s\n", "pass1");
    int newSize = getNewSize(size);
    //printf("%s\n", "pass2");
    if(sf_mem_start() == sf_mem_end())
        if(mem_init() == NULL)
            return NULL;
    //printf("%s\n", "pass3");
    sf_header *header;
    sf_free_list_node *currentNode = sf_free_list_head.next;

    for(;currentNode != &sf_free_list_head; currentNode = currentNode->next){
        if(currentNode->size >= newSize){
            header = currentNode->head.links.next;
            if(header != &(currentNode->head))
                break;
        }
    }
    //printf("%li\n", currentNode->size);
    //printf("%s\n", "pass4");

    if(currentNode == &sf_free_list_head){
        do{
            header = mem_grow();
            if(header == NULL){
                sf_errno = ENOMEM;
              return NULL;
            }
        }while(header->info.block_size<<4 < newSize);
    }
    //printf("%s\n", "pass5");
    int old_block_size = header->info.block_size<<4;

    //allocated header
    header->info.block_size = newSize>>4;
    header->info.requested_size = size;
    header->info.allocated = 1;
    header->info.prev_allocated = 1;
    //printf("%s\n", "pass6");
    //printf("%i\n", old_block_size);
    //printf("%i\n", newSize);
    //add new free blocks to free list
    if(!splitBlock(header, old_block_size - newSize)){
        header->info.block_size = old_block_size>>4;
    }
    //printf("%s\n", "pass7");
    getNextHeaderPointer(header)->info.prev_allocated = 1;
    //set previous allocation of next header to 1
    //printf("%s\n", "pass8");
    removeHeader(header);
    //printf("%s\n", "pass9");
    return (char *)header + sizeof(sf_block_info);
}

void sf_free(void *pp) {
    sf_abort(pp);

    sf_header *hp = getPayloadHeader(pp);
    sf_header *newHeader = hp;

    if(!hp->info.prev_allocated){
        sf_footer *previousFooter = (sf_footer *)hp - 1;
        removeHeader(getHeaderPointer(previousFooter));
        newHeader = coalesce(getHeaderPointer(previousFooter), hp->info.block_size<<4);
    }

    sf_header *nextHeader = getNextHeaderPointer(hp);
    if(!nextHeader->info.allocated){
        removeHeader(nextHeader);
        coalesce(newHeader, nextHeader->info.block_size<<4);
    }

    coalesce(newHeader, 0);
    addHeaderToFreeList(newHeader, getSentinelOfSize(newHeader->info.block_size<<4));

    nextHeader = getNextHeaderPointer(newHeader);
    nextHeader->info.prev_allocated = 0;

    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    sf_abort(pp);

    if(!rsize){
        sf_free(pp);
        return NULL;
    }

    sf_header *hp = getPayloadHeader(pp);

    int newSize = getNewSize(rsize);

    if(hp->info.requested_size < rsize){
        void *newPayload = sf_malloc(rsize);
        if(newPayload == NULL)
            return NULL;

        sf_free(pp);
        return newPayload;
    }
    else if(hp->info.requested_size > rsize){
        int splitSize = (hp->info.block_size<<4) - newSize;
        hp->info.requested_size = rsize;

        if(splitSize >= MIN_SZ){
            hp->info.block_size = newSize>>4;
            splitBlock(hp, splitSize);
            //getNextHeaderPointer(getNextHeaderPointer(hp))->info.prev_allocated = 0;
        }
    }

    //set prev alloc of next header to 0

    return pp;
}


/***************************
helper functions
***************************/



void *mem_init(){
    if(sf_mem_grow() == NULL)
        return NULL;
    createPrologue();
    createEpilogue();

    int freeSize = (sf_mem_end() - sf_mem_start() - sizeof(sf_prologue) - sizeof(sf_epilogue));
    sf_free_list_node *sentinel = sf_add_free_list(freeSize, &sf_free_list_head);

    sf_header *freeHeader = addHeaderToFreeList(setFreeHeader(sf_mem_start() + sizeof(sf_prologue), freeSize), sentinel);
    setFooter(getFooterPointer(freeHeader), freeSize);


    return freeHeader;
}


sf_header *removeHeader(sf_header *freeHeader){
    freeHeader->links.prev->links.next = freeHeader->links.next;
    freeHeader->links.next->links.prev = freeHeader->links.prev;

    return freeHeader;
}

//allocHeader size has already changed
//newSize is the size of the free block
//returns pointer to free block
int splitBlock(sf_header *allocHeader, size_t newSize){

    if(newSize < MIN_SZ)
        return 0;

    sf_free_list_node *sentinel = getSentinelOfSize(newSize);

    //create free block header + footer
    sf_header *freeHeader = addHeaderToFreeList(setFreeHeader(getNextHeaderPointer(allocHeader), newSize), sentinel);
    setFooter(getFooterPointer(freeHeader), newSize);

    sf_header *nextHeader = getNextHeaderPointer(freeHeader);
    if(!nextHeader->info.allocated){
        removeHeader(freeHeader);
        removeHeader(nextHeader);
        coalesce(freeHeader, nextHeader->info.block_size<<4);
        addHeaderToFreeList(freeHeader, getSentinelOfSize(freeHeader->info.block_size<<4));
    }

    return 1;
}

//creates a new free list of size newSize if none is present
sf_free_list_node *getSentinelOfSize(size_t newSize){
    sf_free_list_node *currentNode = getSizeNode(newSize);
    sf_free_list_node *sentinel;

    if(newSize == currentNode->size) sentinel = currentNode;
    else sentinel = sf_add_free_list(newSize, currentNode);

    return sentinel;
}


sf_header *addHeaderToFreeList(sf_header *freeHeader, sf_free_list_node *sentinel){
    freeHeader->links.next = sentinel->head.links.next;
    freeHeader->links.prev = &(sentinel->head);

    sentinel->head.links.next->links.prev = freeHeader;
    sentinel->head.links.next = freeHeader;

    return freeHeader;
}

sf_header *mem_grow(){
    sf_epilogue *epilogue = (sf_epilogue *)sf_mem_end() - 1;
    if(sf_mem_grow() == NULL)
        return NULL;

    sf_header *freeHeader = (sf_header *)epilogue;
    if(!epilogue->footer.info.prev_allocated){
        sf_footer *freeFooter = (sf_footer *)epilogue - 1;
        freeHeader = getHeaderPointer(freeFooter);
        removeHeader(freeHeader);
        //printf("pass1 %i\n", freeHeader->info.block_size<<4);
    }

    createEpilogue();
    //printf("pass2 %i\n", freeHeader->info.block_size<<4);
    coalesce(freeHeader, PAGE_SZ);
    //printf("pass3 %i\n", freeHeader->info.block_size<<4);
    addHeaderToFreeList(freeHeader, getSentinelOfSize(freeHeader->info.block_size<<4));
    //printf("pass4 %i\n", freeHeader->info.block_size<<4);
    return freeHeader;
}
//header - first free block
//size - size of second block
sf_header *coalesce(sf_header *freeHeader, size_t size){

    int newSize = (freeHeader->info.block_size<<4) + size;
    setFreeHeader(freeHeader, newSize);
    setFooter(getFooterPointer(freeHeader), newSize);

    return freeHeader;
}

sf_header *setFreeHeader(sf_header *freeHeader, size_t size){
    freeHeader->info.allocated = 0;
    freeHeader->info.prev_allocated = 1;
    freeHeader->info.block_size = size>>4;
    freeHeader->info.requested_size = 0;

    return freeHeader;
}

sf_footer *setFooter(sf_footer *freeFooter, size_t size){
    freeFooter->info.allocated = 0;
    freeFooter->info.prev_allocated = 1;
    freeFooter->info.block_size = size>>4;
    freeFooter->info.requested_size = 0;


    return freeFooter;
}

sf_footer *getFooterPointer(sf_header *freeHeader){ //free header to free footer
    return (sf_footer*)freeHeader + (freeHeader->info.block_size<<1) - 1;
}

sf_header *getHeaderPointer(sf_footer *freeFooter){//free footer to free header
    return (sf_header*)(freeFooter - (freeFooter->info.block_size<<1) + 1);
}

sf_header *getNextHeaderPointer(sf_header *allocHeader){//allocated header to free header
    return (sf_header*)((sf_footer*)allocHeader + (allocHeader->info.block_size<<1));
}

sf_header *getPayloadHeader(void *pp){
    return (sf_header *)((char *)pp - sizeof(sf_block_info));
}

sf_prologue *createPrologue(){
    sf_prologue *prologue = (sf_prologue *) sf_mem_start();
    prologue->header.info.allocated = 1;
    prologue->header.info.prev_allocated = 0;
    prologue->header.info.block_size = 48>>4;
    prologue->footer.info.allocated = 1;
    prologue->footer.info.prev_allocated = 0;
    prologue->footer.info.block_size = 48>>4;

    return prologue;
}

sf_epilogue *createEpilogue(){
    sf_epilogue *epilogue = (sf_epilogue *) sf_mem_end() - 1;
    epilogue->footer.info.allocated = 1;
    epilogue->footer.info.prev_allocated = 0;
    epilogue->footer.info.block_size = sizeof(sf_epilogue)>>4;
    return epilogue;
}

sf_free_list_node *getSizeNode(size_t size){
    for(sf_free_list_node *currentNode = sf_free_list_head.next;
        currentNode != &sf_free_list_head; currentNode = currentNode->next){
        if(currentNode->size >= size)
            return currentNode;
    }

    return &sf_free_list_head;
}

void sf_abort(void *pp){
    sf_header *hp = (sf_header *)((char *)pp - sizeof(sf_block_info));
    if(pp == NULL || (sf_epilogue *)pp >= (sf_epilogue *)sf_mem_end() - 1|| (sf_prologue *)pp < (sf_prologue *)sf_mem_start() + 1)
        abort();

    if(!hp->info.allocated || hp->info.block_size<<4 < MIN_SZ ||
        (hp->info.block_size<<4)%16 != 0 ||
        hp->info.block_size<<4 < hp->info.requested_size + sizeof(sf_block_info))
        abort();

    sf_footer *previousFooter = NULL;

    if(!hp->info.prev_allocated){
        previousFooter = (sf_footer *)hp - 1;
        if(previousFooter->info.allocated)
            abort();
    }

    return;
}

int getNewSize(size_t size){
    int newSize = sizeof(sf_block_info) + size;
    if(newSize > MIN_SZ){
        if((newSize % ALIGNMENT) != 0)
            newSize = newSize + (ALIGNMENT - (newSize % ALIGNMENT));
    }else return MIN_SZ;

    return newSize;
}