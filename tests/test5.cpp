#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include <stdio.h>

void deref_null() {
    int *num = (int*) malloc(sizeof(int));
    *num = 100;
    printf("num = %d\n", *num);
}

int main() {
    deref_null();
    return 0;
}
