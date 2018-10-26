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
    if(!size)
        return NULL;

    int newSize = getNewSize(size);
    sf_free_list_node *currentNode = sf_free_list_head.next;
    sf_header *header;

    if(sf_mem_start() == sf_mem_end()){
        if((header = mem_init()) == NULL)
            return NULL;
    }
    else{
        for(;currentNode != &sf_free_list_head; currentNode = currentNode->next){
            if(currentNode->size >= newSize){
                header = currentNode->head.links.next;
                if(header != &(currentNode->head))
                    break;
            }
        }

        if(currentNode != &sf_free_list_head)
            removeHeader(header);
        else header->info.block_size = 0;
    }

    while(header->info.block_size<<4 < newSize){
        if((header = mem_grow()) == NULL){
            sf_errno = ENOMEM;
            return NULL;
        }
    }

    int old_block_size = header->info.block_size<<4;
    //allocated header
    header->info.block_size = newSize>>4;
    header->info.requested_size = size;
    header->info.allocated = 1;
    header->info.prev_allocated = 1;

    int splitSize = old_block_size - newSize;

    if(splitSize < 32)
        header->info.block_size = old_block_size>>4;
    else splitBlock(header, splitSize);

    getNextHeaderPointer(header)->info.prev_allocated = 1;

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
            getNextHeaderPointer(splitBlock(hp, splitSize))->info.prev_allocated = 0;
        }
    }

    //set prev alloc of next header to 0

    return pp;
}


//############################################
//HELPER FUNCTIONS
//############################################

/*
 * Grows the heap when the first malloc is called.
 * creates the prologue, epilouge and the first free block header and footer.
 *
 * @return If heap size is nonzero, then NULL is returned without setting sf_errno.
 * If sf_mem_grow returns NULL, then NULL is returned without setting sf_errno.
 * If successful, then the pointer to the free block is returned.
 */
void *mem_init(){
    if(sf_mem_start() != sf_mem_end())
        return NULL;

    if(sf_mem_grow() == NULL)
        return NULL;

    createPrologue();
    createEpilogue();

    int freeSize = (sf_mem_end() - sf_mem_start() - sizeof(sf_prologue) - sizeof(sf_epilogue));

    sf_header *freeHeader = setFreeHeader(sf_mem_start() + sizeof(sf_prologue), freeSize);
    setFooter(getFooterPointer(freeHeader), freeSize);

    return freeHeader;
}

/*
 * Grows the heap using sf_mem_grow and coalesce if necessary.
 * removes the header from the free list if coalesce with a free block is neccesary.
 * creates the free block header starting from the epilogue or the previous free block header.
 *
 * @return If sf_mem_grow returns NULL, then NULL is returned and
 * the remaining free block is added to the free list.
 * If successful, then the pointer to the free block is returned.
 */
sf_header *mem_grow(){
    sf_epilogue *epilogue = (sf_epilogue *)sf_mem_end() - 1;
    sf_footer *freeFooter = (sf_footer *)epilogue - 1;
    sf_header *freeHeader = (sf_header *)epilogue;

    if(sf_mem_grow() == NULL){
        freeHeader = getHeaderPointer(freeFooter);
        addHeaderToFreeList(freeHeader, getSentinelOfSize(freeHeader->info.block_size<<4));
        return NULL;
    }

    if(!epilogue->footer.info.prev_allocated){
        freeHeader = getHeaderPointer(freeFooter);

        if(sf_free_list_head.next != &sf_free_list_head)
            removeHeader(freeHeader);
    }

    createEpilogue();
    coalesce(freeHeader, PAGE_SZ);

    return freeHeader;
}

/*
 * Combines two free blocks by setting the first free block size as the sum of the two.
 * Recreates the first free block header and and creates the footer.
 *
 * @param freeHeader Address of a free block that will be combined.
 * It will also be the address of the header for the coalesce block.
 * @param size Size of the second free block which will be combined.
 * Includes header,footer and any padding.
 *
 * @return Address of the coalesced blocks.
 */
sf_header *coalesce(sf_header *freeHeader, size_t size){

    int newSize = (freeHeader->info.block_size<<4) + size;
    setFreeHeader(freeHeader, newSize);
    setFooter(getFooterPointer(freeHeader), newSize);

    return freeHeader;
}

/*
 * Removes the header from the free list.
 *
 * @param freeHeader Address of a free block that will be removed from the free list.
 *
 * @return Address of the free block.
 */
sf_header *removeHeader(sf_header *freeHeader){
    freeHeader->links.prev->links.next = freeHeader->links.next;
    freeHeader->links.next->links.prev = freeHeader->links.prev;

    return freeHeader;
}

/*
 * Splits the specified block into one allocated block and one free block.
 * Coalesces if the block after the created free block is also free.
 *
 * @param allocHeader Address of an allocated block which will be split.
 * @param newSize Size of the free block which will be split from allocHeader.
 * Must be greater than the minimum size of a block.
 *
 * @return Address of the free block.
 */
sf_header *splitBlock(sf_header *allocHeader, size_t newSize){

    if(newSize < MIN_SZ)
        return NULL;

    //create free block header + footer
    sf_header *freeHeader = setFreeHeader(getNextHeaderPointer(allocHeader), newSize);
    sf_header *nextHeader = getNextHeaderPointer(freeHeader);

    if(!nextHeader->info.allocated){

        removeHeader(nextHeader);
        coalesce(freeHeader, nextHeader->info.block_size<<4);
        setFooter(getFooterPointer(freeHeader), freeHeader->info.block_size<<4);
        addHeaderToFreeList(freeHeader, getSentinelOfSize(freeHeader->info.block_size<<4));
    }else{
        addHeaderToFreeList(freeHeader, getSentinelOfSize(newSize));
        setFooter(getFooterPointer(freeHeader), newSize);
    }

    return freeHeader;
}

/*
 * Returns the free list node of the specified size.
 * Creates a new free list node of the specified size if none is present.
 *
 * @param newSize Size of the free list node that is desired.
 *
 * @return Free list node of the desired size.
 */
sf_free_list_node *getSentinelOfSize(size_t newSize){
    sf_free_list_node *currentNode = getSizeNode(newSize);
    sf_free_list_node *sentinel;

    if(newSize == currentNode->size) sentinel = currentNode;
    else sentinel = sf_add_free_list(newSize, currentNode);

    return sentinel;
}

/*
 * Adds the header into the free list node of the same size.
 *
 * @param freeHeader Address of a free block that will be added to the free list node.
 * @param sentinel Node which the free block will be placed in.
 *
 * @return Address of the free block.
 */
sf_header *addHeaderToFreeList(sf_header *freeHeader, sf_free_list_node *sentinel){
    freeHeader->links.next = sentinel->head.links.next;
    freeHeader->links.prev = &(sentinel->head);

    sentinel->head.links.next->links.prev = freeHeader;
    sentinel->head.links.next = freeHeader;

    return freeHeader;
}

/*
 * Sets the block info of a free block header.
 *
 * @param freeHeader Address of a free block header.
 * @param size Size of the specified free block.
 * Includes header,footer and any padding.
 *
 * @return Address of the free block footer.
 */

sf_header *setFreeHeader(sf_header *freeHeader, size_t size){
    freeHeader->info.allocated = 0;
    freeHeader->info.prev_allocated = 1;
    freeHeader->info.block_size = size>>4;
    freeHeader->info.requested_size = 0;

    return freeHeader;
}

/*
 * Sets the block info of a free block footer.
 *
 * @param freeFooter Address of a free block footer.
 * @param size Size of the specified free block.
 *
 * @return Address of the free block footer.
 */

sf_footer *setFooter(sf_footer *freeFooter, size_t size){
    freeFooter->info.allocated = 0;
    freeFooter->info.prev_allocated = 1;
    freeFooter->info.block_size = size>>4;
    freeFooter->info.requested_size = 0;


    return freeFooter;
}

/*
 * Returns the footer of a free block given a free block header.
 *
 * @param freeHeader Header address of the specified free block.
 *
 * @return Address of the free block footer.
 */
sf_footer *getFooterPointer(sf_header *freeHeader){
    return (sf_footer*)freeHeader + (freeHeader->info.block_size<<1) - 1;
}

/*
 * Returns the header of a free block given a free block footer.
 *
 * @param freeFooter Footer address of the specified free block.
 *
 * @return Address of the free block header.
 */
sf_header *getHeaderPointer(sf_footer *freeFooter){
    return (sf_header*)(freeFooter - (freeFooter->info.block_size<<1) + 1);
}

/*
 * Returns the header of the next block.
 *
 * @param hp Address of the block before the desired block.
 *
 * @return Address of the next block header.
 */
sf_header *getNextHeaderPointer(sf_header *hp){
    return (sf_header*)((sf_footer*)hp + (hp->info.block_size<<1));
}

/*
 * Returns the header of the block given the payload pointer.
 *
 * @param pp Address of the payload.
 *
 * @return Address of the block header.
 */
sf_header *getPayloadHeader(void *pp){
    return (sf_header *)((char *)pp - sizeof(sf_block_info));
}

/*
 * Creates the prologue and sets its info.
 *
 * @return Address of the prologue.
 */
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

/*
 * Creates the epilogue and sets its info.
 *
 * @return Address of the epilouge.
 */
sf_epilogue *createEpilogue(){
    sf_epilogue *epilogue = (sf_epilogue *) sf_mem_end() - 1;
    epilogue->footer.info.allocated = 1;
    epilogue->footer.info.prev_allocated = 0;
    epilogue->footer.info.block_size = sizeof(sf_epilogue)>>4;
    return epilogue;
}

/*
 * Gets the free list node of the desired size.
 *
 * @param size Size of the desired free list node.
 *
 * @return If a node of the desired size is present, the address of the node is returned;
 *  If no node of the desired size is present, the address of the sf_free_list_header is returned;
 */
sf_free_list_node *getSizeNode(size_t size){
    for(sf_free_list_node *currentNode = sf_free_list_head.next;
        currentNode != &sf_free_list_head; currentNode = currentNode->next){
        if(currentNode->size >= size)
            return currentNode;
    }

    return &sf_free_list_head;
}

/*
 * Verifies that the specified payload pointer is in a valid allocated block.
 * The address of the payload must not be before the prologue or after the epilogue.
 * The block must be allocated and have a size that is divisible by the word alignment
 * and a size of the mimimum block size or greater.
 * If the previous block is a free block, the free block must have its allocated bit set to 0.
 * If any of these criteria are not met, abort() is called.
 *
 * @param pp Address of the payload to be verified.
 */
void sf_abort(void *pp){
    sf_header *hp = (sf_header *)((char *)pp - sizeof(sf_block_info));
    if(pp == NULL || (sf_epilogue *)pp >= (sf_epilogue *)sf_mem_end() - 1|| (sf_prologue *)pp < (sf_prologue *)sf_mem_start() + 1)
        abort();

    if(!hp->info.allocated || hp->info.block_size<<4 < MIN_SZ ||
        (hp->info.block_size<<4)%16 != 0 ||
        hp->info.block_size<<4 < hp->info.requested_size + sizeof(sf_block_info))
        abort();

    if(!hp->info.prev_allocated){
        sf_footer * previousFooter = (sf_footer *)hp - 1;
        if(previousFooter->info.allocated)
            abort();
    }

    return;
}

/*
 * Gets the block size of the request size.
 *
 * @param size Requested size.
 *
 * @return Size of the block which is greater or equal to the minimum block size and
 * is divisible by the word alignement.
 */

int getNewSize(size_t size){
    int newSize = sizeof(sf_block_info) + size;
    if(newSize > MIN_SZ){
        if((newSize % ALIGNMENT) != 0)
            newSize = newSize + (ALIGNMENT - (newSize % ALIGNMENT));
    }else return MIN_SZ;

    return newSize;
}