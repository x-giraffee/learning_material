#include <stdio.h>

int main()
{
    int x=0x77000000;

    char *c=&x;
    printf("������:0x77000000");
    printf("\n�������ڴ��еĴ��Ϊ��");
    printf(" %2.2x",c[0]);
    printf(" %2.2x",c[1]);
    printf(" %2.2x",c[2]);
    printf(" %2.2x",c[3]);
    if(c[0]==0x00&&c[1]==0x00&&c[2]==0x00&&c[3]==0x77)
        printf("\n�û�������С�˷�");
    else
        printf("\n�û������ô�˷�");
    return 0;
}
