#include <stdio.h>

typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer start,size_t len)
{
    size_t i;
    printf("����:");
    for(i=0;i<len;i++)
        printf(" %.2x",start[i]);
}


int main()
{
    int i=12345;
    unsigned int ui=12345;
    float j=61.419998;
    short k=123;
    long l=123456789;
    char m='m';
    double n=1234.56789;
    void *x=NULL;
    int *xi=&i;
    struct test
    {
        int i;
        float j;
    }pl;
    pl.i=123;
    pl.j=123.0;

    enum Color
    {
        RED=1,
        GREEN=2,
        BLUE=4
    }epl;


    union  Demo
    {
        int a;
        char b;
        long long c;
    }dpl;
    dpl.a=123;

    printf("\n�з������ͣ�int��:         ");
    printf(" ֵ:%d          ",i);
    show_bytes((byte_pointer)&i,sizeof(int));
    printf("              ��ŵ�ַ: %p",&i);
    printf("\n");

    printf("\n�޷������ͣ�unsigned int��:");
    printf(" ֵ:%u          ",i);
    show_bytes((byte_pointer)&ui,sizeof(unsigned int));
    printf("              ��ŵ�ַ: %p",&ui);
    printf("\n");

    printf("\n�����ͣ�long��:            ");
    printf(" ֵ:%ld      ",l);
    show_bytes((byte_pointer)&l,sizeof(long));
    printf("              ��ŵ�ַ: %p",&l);
    printf("\n");

    printf("\n�����ͣ�short��:           ");
    printf(" ֵ:%ld            ",k);
    show_bytes((byte_pointer)&k,sizeof(short));
    printf("                    ��ŵ�ַ: %p",&k);
    printf("\n");

    printf("\n�����ȸ������ͣ�float��:   ");
    printf(" ֵ:%f      ",j);
    show_bytes((byte_pointer)&j,sizeof(float));
    printf("              ��ŵ�ַ: %p",&j);
    printf("\n");

    printf("\n˫���ȸ������ͣ�double��:  ");
    printf(" ֵ:%lf    ",n);
    show_bytes((byte_pointer)&n,sizeof(double));
    printf("  ��ŵ�ַ: %p",&n);
    printf("\n");

    printf("\n�ַ��ͣ�char��:            ");
    printf(" ֵ:%c              ",m);
    show_bytes((byte_pointer)&m,sizeof(char));
    printf("                       ��ŵ�ַ: %p",&m);
    printf("\n");

    printf("\n��ָ�������void *��:      ");
    printf(" ֵ:%p       ",x);
    show_bytes((byte_pointer)&x,sizeof(void *));
    printf("              ��ŵ�ַ: %p",x);
    printf("\n");

    printf("\nָ��i��ָ�������int *��:  ");
    printf(" ֵ:%p       ",xi);
    show_bytes((byte_pointer)&xi,sizeof(void *));
    printf("              ��ŵ�ַ: %p",&xi);
    printf("\n");

    printf("\n�ṹ�����ͣ�struct��:      ");
    printf(" ֵ:%d %f ",pl.i,pl.j);
    show_bytes((byte_pointer)&pl,sizeof(pl));
    printf("  ��ŵ�ַ: %p",&pl);
    printf("\n");


    printf("\nö�����ͣ�enum��:          ");
    printf(" ֵ:%d   %d   %d      ",RED,GREEN,BLUE);
    show_bytes((byte_pointer)&epl,sizeof(epl));
    printf("              ��ŵ�ַ: %p",&epl);
    printf("\n");
    printf("          ע��ö�������зֱ�������������RED=1,GREEN=2,BLUE=4\n");

    printf("\n���������ͣ�union��:       ");
    printf(" ֵ:%d            ",dpl.a);
    show_bytes((byte_pointer)&dpl,sizeof(dpl));
    printf("  ��ŵ�ַ: %p",&dpl);
    printf("\n");
    dpl.b='m';
    printf("\n                           ");
    printf(" ֵ:%c              ",dpl.b);
    show_bytes((byte_pointer)&dpl,sizeof(dpl));
    printf("  ��ŵ�ַ: %p",&dpl);
    printf("\n");
    dpl.c=13456789;
    printf("\n                           ");
    printf(" ֵ:%lld       ",dpl.c);
    show_bytes((byte_pointer)&dpl,sizeof(dpl));
    printf("  ��ŵ�ַ: %p",&dpl);
    printf("\n");
    printf("          ע������ö������ÿ��ֻ�ܴ��һ��ֵ���˴�ÿ����ʾֵ֮ǰ�������³�ʼ��\n          ö�����͵��������ֱ���Ϊint��char��long long����\n");

    printf("\n main�����ĵ�ַ��   %p\n",main);
    printf("\n printf�����ĵ�ַ:  %p\n",printf);
    printf("\n scanf�����ĵ�ַ:   %p\n",scanf);

    return 0;
}
