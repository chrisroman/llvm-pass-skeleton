#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include <stdio.h>
//#include "track_nullptr.h"

void deref_null(int argc) {
    int *p = nullptr;
    *p = 100;
    //escape(&p);
}

int main(int argc, char **argv) {
    //escape(&nullp);
    deref_null(argc);
    return 0;
}
