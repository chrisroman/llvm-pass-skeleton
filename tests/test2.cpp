#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include "track_nullptr.h"

void deref_null(char *p) {
    *p = 42;
}

int main(int argc, char **argv) {
    deref_null(nullptr);
    return 0;
}
