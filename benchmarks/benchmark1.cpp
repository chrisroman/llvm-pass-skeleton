#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include <stdio.h>
#include "track_nullptr.h"

int main(int argc, char **argv) {
    int *num = nullptr;
    if (argc == 1) {
        num = (int*)malloc(sizeof(int));
        *num = 0;
    }
    for (int i = 0; i < argc * 100000000; ++i) {
        *num = *num + 1;
        escape(num);
    }
    printf("num = %d\n", *num);
    return 0;
}
