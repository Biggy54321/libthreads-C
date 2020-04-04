#include <stdio.h>
#include <errno.h>

void main() {

    int i = errno;

    printf("%d\n", i);

    printf("%d\n", errno);
}
