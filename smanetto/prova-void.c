#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 4
//#define SIZE 1099511627776

struct boh {
    long type;
    char message[256];
};

void foo() {
    register const char rdi_r asm("rdi");
    printf("%d\n", rdi_r);
}

int main(void)
{
    int num = 5;
    char *string = "Ciao";
    struct boh struct1 = {45, "cane"};
    struct boh *struct2 = (struct boh *) malloc(sizeof(struct boh));
    struct2->type = -23;
    strcpy(struct2->message, "gatto");


    void **locations = (void **) calloc(SIZE, sizeof(void *));

    locations[0] = (void *) &num;
    locations[1] = string;
    locations[2] = &struct1;
    locations[3] = struct2;

    printf("%3d\n", *(int *) *(locations + 0));
    printf("%3s\n", ((char *) locations[1]));
    printf("%3ld\t%3s\n", ((struct boh *) *(locations + 2))->type, ((struct boh *) *(locations + 2))->message);
    printf("%3ld\t%3s\n", ((struct boh *) *(locations + 3))->type, ((struct boh *) *(locations + 3))->message);

#ifdef _DEBUG
    printf("\nDebug:\n");

    char ch = 'A';
    asm volatile("xor rdi, rdi");
    foo(ch);
    printf("\n");
#endif

    return 0;
}
