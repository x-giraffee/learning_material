#include <stdio.h>

typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer start,size_t len)
{
    size_t i;
    printf("内容:");
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

    printf("\n有符号整型（int）:         ");
    printf(" 值:%d          ",i);
    show_bytes((byte_pointer)&i,sizeof(int));
    printf("              存放地址: %p",&i);
    printf("\n");

    printf("\n无符号整型（unsigned int）:");
    printf(" 值:%u          ",i);
    show_bytes((byte_pointer)&ui,sizeof(unsigned int));
    printf("              存放地址: %p",&ui);
    printf("\n");

    printf("\n长整型（long）:            ");
    printf(" 值:%ld      ",l);
    show_bytes((byte_pointer)&l,sizeof(long));
    printf("              存放地址: %p",&l);
    printf("\n");

    printf("\n短整型（short）:           ");
    printf(" 值:%ld            ",k);
    show_bytes((byte_pointer)&k,sizeof(short));
    printf("                    存放地址: %p",&k);
    printf("\n");

    printf("\n单精度浮点类型（float）:   ");
    printf(" 值:%f      ",j);
    show_bytes((byte_pointer)&j,sizeof(float));
    printf("              存放地址: %p",&j);
    printf("\n");

    printf("\n双精度浮点类型（double）:  ");
    printf(" 值:%lf    ",n);
    show_bytes((byte_pointer)&n,sizeof(double));
    printf("  存放地址: %p",&n);
    printf("\n");

    printf("\n字符型（char）:            ");
    printf(" 值:%c              ",m);
    show_bytes((byte_pointer)&m,sizeof(char));
    printf("                       存放地址: %p",&m);
    printf("\n");

    printf("\n空指针变量（void *）:      ");
    printf(" 值:%p       ",x);
    show_bytes((byte_pointer)&x,sizeof(void *));
    printf("              存放地址: %p",x);
    printf("\n");

    printf("\n指向i的指针变量（int *）:  ");
    printf(" 值:%p       ",xi);
    show_bytes((byte_pointer)&xi,sizeof(void *));
    printf("              存放地址: %p",&xi);
    printf("\n");

    printf("\n结构体类型（struct）:      ");
    printf(" 值:%d %f ",pl.i,pl.j);
    show_bytes((byte_pointer)&pl,sizeof(pl));
    printf("  存放地址: %p",&pl);
    printf("\n");


    printf("\n枚举类型（enum）:          ");
    printf(" 值:%d   %d   %d      ",RED,GREEN,BLUE);
    show_bytes((byte_pointer)&epl,sizeof(epl));
    printf("              存放地址: %p",&epl);
    printf("\n");
    printf("          注：枚举类型中分别定义了三个量：RED=1,GREEN=2,BLUE=4\n");

    printf("\n共用体类型（union）:       ");
    printf(" 值:%d            ",dpl.a);
    show_bytes((byte_pointer)&dpl,sizeof(dpl));
    printf("  存放地址: %p",&dpl);
    printf("\n");
    dpl.b='m';
    printf("\n                           ");
    printf(" 值:%c              ",dpl.b);
    show_bytes((byte_pointer)&dpl,sizeof(dpl));
    printf("  存放地址: %p",&dpl);
    printf("\n");
    dpl.c=13456789;
    printf("\n                           ");
    printf(" 值:%lld       ",dpl.c);
    show_bytes((byte_pointer)&dpl,sizeof(dpl));
    printf("  存放地址: %p",&dpl);
    printf("\n");
    printf("          注：由于枚举类型每次只能存放一个值，此处每次显示值之前对其重新初始化\n          枚举类型的三个两分别定义为int，char和long long类型\n");

    printf("\n main函数的地址：   %p\n",main);
    printf("\n printf函数的地址:  %p\n",printf);
    printf("\n scanf函数的地址:   %p\n",scanf);

    return 0;
}
