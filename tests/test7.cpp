#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include <stdio.h>
#include "track_nullptr.h"

void deref_null(int argc) {
    int *a = (int*) malloc(sizeof(int));
    int **p = &a;
    escape(&a);
    escape(&p);
    *p = nullptr;
    *a = 100;
}

int main(int argc, char **argv) {
    deref_null(argc);
    return 0;
}
