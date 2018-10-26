#include "sfmm.h"

#ifndef SFMM_HELPER_H
#define SFMM_HELPER_H
#define MIN_SZ 32
#define ALIGNMENT 16

void *mem_init();
sf_footer *getFooterPointer(sf_header *freeHeader);
sf_header *getHeaderPointer(sf_footer *freeFooter);
sf_header *getNextHeaderPointer(sf_header *hp);
sf_header *getPayloadHeader(void *pp);
sf_prologue *createPrologue();
sf_epilogue *createEpilogue();
sf_header *mem_grow();
sf_header *coalesce(sf_header *header, size_t size);
sf_header *removeHeader(sf_header *freeHeader);
sf_header *splitBlock(sf_header *header, size_t oldSize);
sf_header *setFreeHeader(sf_header *freeHeader, size_t size);
sf_footer *setFooter(sf_footer *freeFooter, size_t size);
sf_header *addHeaderToFreeList(sf_header *freeHeader, sf_free_list_node *sentinel);
sf_free_list_node *getSizeNode(size_t size);
sf_free_list_node *getSentinelOfSize(size_t newSize);
void sf_abort(void *pp);
int getNewSize(size_t size);

#endif