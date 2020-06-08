#include <stdio.h>

int main()
{
    int x=0x77000000;

    char *c=&x;
    printf("定义数:0x77000000");
    printf("\n该数在内存中的存放为：");
    printf(" %2.2x",c[0]);
    printf(" %2.2x",c[1]);
    printf(" %2.2x",c[2]);
    printf(" %2.2x",c[3]);
    if(c[0]==0x00&&c[1]==0x00&&c[2]==0x00&&c[3]==0x77)
        printf("\n该机器采用小端法");
    else
        printf("\n该机器采用大端法");
    return 0;
}
