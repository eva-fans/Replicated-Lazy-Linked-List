#include "intset.h"

int main(void)
{
    printf("test\n");
    intset_l_t *set=set_new_l();
    set_add_l(set,4,1);
    printf("%d\n",set_contains_l(set,5,1));
    return 0;
}