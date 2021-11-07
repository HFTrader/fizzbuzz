
#include <stdio.h>
#include <stdint.h>

int main()
{
    uint64_t maxnum = 10000000ULL;
    for (uint64_t i = 1; i < maxnum; i++)
    {
        if ((i % 3 == 0) && (i % 5 == 0))
        {
            printf("FizzBuzz\n");
        }
        else if (i % 3 == 0)
        {
            printf("Fizz\n");
        }
        else if (i % 5 == 0)
        {
            printf("Buzz\n");
        }
        else
        {
            printf("%ld\n", i);
        }
    }
}
