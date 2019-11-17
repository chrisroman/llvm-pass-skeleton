#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include <stdio.h>

void deref_null() {
    int *num = new int(6120);
    int **pp = &num;
    *pp = nullptr;
    printf("num = %d\n", *num);
}

int main(int argc, char **argv) {
    deref_null();
    return 0;
}
