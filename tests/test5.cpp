#include <stdlib.h>     /* exit, EXIT_FAILURE */

void deref_null() {
    int *p = new int();
}

void test() {
    int x;
    int *p = &x;
}

int main() {
    deref_null();
    return 0;
}
