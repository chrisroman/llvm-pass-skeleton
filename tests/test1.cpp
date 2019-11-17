#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include "track_nullptr.h"

void foo(void *p) {
    return;
}

// TODO:
// create a nullp
// call escale(rnullp)
// replace all instances of nullptr with nullp
void deref_null(int argc) {
    //void *nullp = nullptr;
    //escape(&nullp);
    int *p = (int*) nullptr;
    if (argc == 1) {
        p = new int(5);
    }
    int *q = p;
    if (argc == 2) {
        q = new int(10);
    }
    foo(p);
    foo(q);
    //int *q = p;
    //int x = *p;
}

int main(int argc, char **argv) {
    deref_null(argc);
    return 0;
}
