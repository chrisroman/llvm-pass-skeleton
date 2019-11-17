#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include "track_nullptr.h"

void deref_null(int argc) {
    int *p = nullptr;
    if (argc == 1) {
        p = &argc;
    }
    *p = 42;
}

int main(int argc, char** argv) {
    //escape(&nullp);
    deref_null(argc);
    return 0;
}
