#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_mem_init();
    /*void *x = sf_malloc(3* PAGE_SZ - sizeof(sf_prologue) - sizeof(sf_epilogue) - 32);
    printf("%p\n", x);
    sf_show_heap();*/
    /*sf_malloc(sizeof(int));
    sf_malloc(sizeof(int));
    printf("%p\n", sf_free_list_head.next);
    printf("%p\n", sf_free_list_head.prev);

    sf_show_heap();*/

    /*void *x = sf_malloc(sizeof(double) * 8); sf_show_heap();
    sf_realloc(x, sizeof(int));
    sf_show_heap();*/



    /*double *ppp = sf_malloc(sizeof(double)); sf_show_heap();
    int *pp = sf_malloc(100); sf_show_heap();
    int *pppp = sf_malloc(48); sf_show_heap();
    //sf_show_heap();
    sf_malloc(3950);
    sf_show_heap();
    sf_free(ppp);
    sf_show_heap();
    //printf("%p\n", pp);
    //sf_show_heap();
    sf_realloc(pp, 4);
    sf_show_heap();
    sf_malloc(4);
    sf_show_heap();
    sf_realloc(pppp, 8);
    sf_show_heap();
    sf_malloc(4);
    sf_show_heap();*/
    double* ptr = sf_malloc(sizeof(double));

    *ptr = 320320320e-320;

    printf("%f\n", *ptr);

    sf_free(ptr);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
