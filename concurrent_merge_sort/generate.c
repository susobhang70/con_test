#include <stdio.h>
#include <time.h>
int main()
{
    srand(time(NULL));
    while(1)
        printf("%d\n", rand());
    return 0;
}