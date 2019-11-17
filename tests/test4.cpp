#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include <stdio.h>

struct Foo {
    int x;
    int y;
    int z;
};

void modify_obj(Foo *foo) {
    if (foo) {
        foo->x = 1;
        foo->y = 2;
        foo->z = 3;
    }
}

int main(int argc, char** argv) {
    //printf("argc = %d\n", argc);
    Foo *foo = nullptr;
    if (argc == 1) {
        Foo tmp = Foo();
        foo = new Foo();
    }

    modify_obj(foo);
    printf("sum = %d\n", foo->x + foo->y + foo->z);
}
