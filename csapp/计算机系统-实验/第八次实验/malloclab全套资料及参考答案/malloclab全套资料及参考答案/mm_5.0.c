/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Your student_id */
    "16307130350",
    /* Your full name */
    "王明霞",
    /* Your email address */
    "16307130350@fudan.edu.cn",
    /* leave blank  */
    "",
    /* leave blank */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4
#define DSIZE 8          

#define CHUNKSIZE (1<<12) //extend heap by this amount(bytes)

#define MAX(x,y)    ((x)>(y)?(x):(y))

#define PACK(size,alloc)    ((size) | (alloc)) //pack a size and allocated bit into a word

#define GET(p)  (*(unsigned int *)(p)) //read and write a word at address p
#define PUT(p,val)  (*(unsigned int *)(p) = (val))

#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

#define HDRP(bp)    ((char *)(bp)-WSIZE)
#define FTRP(bp)    ((char *)(bp)+GET_SIZE(HDRP(bp))-DSIZE) /*合并方式采取带边界标记的合并*/

/* Compute address of next and previous blocks*/
#define NEXT_BLKP(bp)   ((char *)(bp)+GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp)   ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

#define PREV_LINKNODE_RP(bp) ((char*)(bp)) 			/*bp的前一个node：pred*/
#define NEXT_LINKNODE_RP(bp) ((char*)(bp)+WSIZE) 	/*bp的后一个node：succ*/

 
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
inline char *find_free_list_header(size_t size);
inline void delete_from_list(char *p);
inline void insert_to_link(char *p);
static void place(void *bp,size_t asize);
static void* find_fit(size_t size);
void* memcpy_new(void *Dest, const void *Src, size_t Count);
static void *realloc_coalesce(void *bp, size_t newSize, int *bp_not_change);
static void realloc_place(void *bp,size_t asize);
static char *heap_listp = NULL;
static char *free_list_start = NULL;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void){

    if((heap_listp = mem_sbrk(14*WSIZE))==(void *)-1) //使用mem_sbrk请求额外的堆处理器
		return -1;

	/* 使用简单分离存储 */
    PUT(heap_listp,0);              //block size <=8
    PUT(heap_listp+(1*WSIZE),0);    //block size <=16
    PUT(heap_listp+(2*WSIZE),0);    //block size <=32
    PUT(heap_listp+(3*WSIZE),0);    //block size <=64
    PUT(heap_listp+(4*WSIZE),0);    //block size <=128
    PUT(heap_listp+(5*WSIZE),0);    //block size <=256
    PUT(heap_listp+(6*WSIZE),0);    //block size <=512
    PUT(heap_listp+(7*WSIZE),0);    //block size <=2048
    PUT(heap_listp+(8*WSIZE),0);    //block size <=4096
    PUT(heap_listp+(9*WSIZE),0);    //block size >4096
    PUT(heap_listp+(10*WSIZE),0);
    PUT(heap_listp+(11*WSIZE),PACK(DSIZE,1)); //prologue header 序言块
    PUT(heap_listp+(12*WSIZE),PACK(DSIZE,1)); //prologue footer 序言块
    PUT(heap_listp+(13*WSIZE),PACK(0,1));     //epilogue header 结尾块
	
	free_list_start = heap_listp;			  //分类链表的首指针
	heap_listp += (12*WSIZE);                 //heap_listp定位到footer序号块（是堆的起始位置）
	
	if((extend_heap(CHUNKSIZE/DSIZE))==NULL)  //extend the empty heap with a free block of CHUNKSIZE bytes
		return -1;
	return 0;
}

/*根据堆的大小扩展堆*/
static void *extend_heap(size_t words){
    char *bp;
    size_t size;
	
    /* Allocate an even number of dwords to maintain alignment 8*/
	/*向上舍入最接近二字，8字节的倍数*/
    size = (words % 2) ? (words+1) * DSIZE : words * DSIZE;
    if((long)(bp = mem_sbrk(size))==(void *)-1) /*bp：堆的起始位置*/
        return NULL;

	/* Initialize free block headerr/footerd*/
    PUT(HDRP(bp),PACK(size,0)); /*Free block header*/
    PUT(FTRP(bp),PACK(size,0)); /*Free block footer*/

    PUT(NEXT_LINKNODE_RP(bp),NULL);
    PUT(PREV_LINKNODE_RP(bp),NULL);

    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1)); /*New epilogue header*/

    return coalesce(bp);
}

static void *coalesce(void *bp){
    size_t  prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));	/* 得到block的大小 */
    size_t  next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /*coalesce the block and change the point*/
    if(prev_alloc && next_alloc){ }						
	/*提前判断 避免否定后面的3种情况才确定，节省时间*/
    else if(prev_alloc && !next_alloc) /*next_alloc==null*/{
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        delete_from_list(NEXT_BLKP(bp));	
        PUT(HDRP(bp), PACK(size,0));	/* 更新header footer */
        PUT(FTRP(bp), PACK(size,0));
    }
    else if(!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        delete_from_list(PREV_BLKP(bp));
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);		/*更新bp*/
    }
    else{
        size += ( GET_SIZE(FTRP(NEXT_BLKP(bp)))+ GET_SIZE(HDRP(PREV_BLKP(bp))));
        delete_from_list(PREV_BLKP(bp));
        delete_from_list(NEXT_BLKP(bp));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    insert_to_link(bp);	/*把bp连接到bp所属于的free_list_link中*/
    return bp;
}


inline char *find_free_list_header(size_t size){
    int i = 0;
    if(size<=8) i=0;
    else if(size<=16) i= 1;
    else if(size<=32) i= 2;
    else if(size<=64) i= 3;
    else if(size<=128) i= 4;
    else if(size<=256) i= 5;
    else if(size<=512) i= 6;
    else if(size<=2048) i= 7;
    else if(size<=4096) i= 8;
    else i= 9;
	
    return free_list_start+(i*WSIZE);
}

inline void delete_from_list(char *p){	
    char *free_list_header = find_free_list_header((GET_SIZE(HDRP(p))));
    char *prevp = GET(PREV_LINKNODE_RP(p));		/* p的前一个node */
    char *nextp = GET(NEXT_LINKNODE_RP(p));		/* p的后一个node */
	
    if(prevp == NULL){	/*必然要保证prevp先填满才可以填nextp*/
			if(nextp != NULL)
				PUT(PREV_LINKNODE_RP(nextp),0);
			PUT(free_list_header,nextp);
    }
    else {
        if(nextp != NULL)
			PUT(PREV_LINKNODE_RP(nextp),prevp);
        PUT(NEXT_LINKNODE_RP(prevp),nextp);
    }
	/*将当前的block p放空*/
    PUT(NEXT_LINKNODE_RP(p),NULL);
    PUT(PREV_LINKNODE_RP(p),NULL);
}

inline void insert_to_link(char *p){
    char *free_list_header = find_free_list_header(GET_SIZE(HDRP(p)));
    char *prevp = free_list_header;
    char *nextp = GET(free_list_header); /*获得free_list指向的块的地址*/

    while(nextp != NULL){
        if(GET_SIZE(HDRP(nextp))>=GET_SIZE(HDRP(p))) 
			break;
        prevp = nextp;
        nextp = GET(NEXT_LINKNODE_RP(nextp));
    }
	
    if(prevp == free_list_header){
        PUT(free_list_header,p);
        PUT(NEXT_LINKNODE_RP(p),nextp);
        PUT(PREV_LINKNODE_RP(p),NULL);
        if(nextp!=NULL) 
			PUT(PREV_LINKNODE_RP(nextp),p);
    }
    else{
        PUT(NEXT_LINKNODE_RP(prevp),p);
        PUT(PREV_LINKNODE_RP(p),prevp);
        PUT(NEXT_LINKNODE_RP(p),nextp);
        if(nextp!=NULL) 
			PUT(PREV_LINKNODE_RP(nextp),p);
    }

}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size){
    size_t asize;
    size_t extendsize; /*Amount to extend heap if no fit*/
    char *bp;
	
    if(size == 0) return NULL;
    if(size <= DSIZE){	/*Adjust block size to include overhead and aligment reqs*/
        asize = 2*(DSIZE);
    }
    else{
        asize = (DSIZE)*((size+(DSIZE)+(DSIZE-1)) / (DSIZE));
    }
	
    if((bp = find_fit(asize))!= NULL){
        place(bp,asize);
        return bp;
    }
	
    /*Not fit found, get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);
    if((bp = extend_heap(extendsize/DSIZE)) == NULL)
        return NULL;
    place(bp,asize);
    return bp;
}

static void* find_fit(size_t size){
    char *free_list_header = find_free_list_header(size);
    char *temp;
    for(; free_list_header!=(heap_listp-(2*WSIZE)); free_list_header+=WSIZE){
        temp = GET(free_list_header);
        while(temp != NULL){
            if(GET_SIZE(HDRP(temp)) >= size) /* 空闲块由大到小排序 */
				return temp;
            temp = GET(NEXT_LINKNODE_RP(temp));
        }
    }
	return NULL;
}

/* 将asize的数据存放在bp指向的块中 */
static void place(void *bp,size_t asize){
    size_t csize = GET_SIZE(HDRP(bp));
	
	delete_from_list(bp);	//把bp从空闲块中删除掉
	if((csize-asize) >= (2*DSIZE)){ /*剩余块的大小>16字节（最小块的大小） 说明可划分*/
        PUT(HDRP(bp),PACK(asize,1));
        PUT(FTRP(bp),PACK(asize,1));
		
        bp = NEXT_BLKP(bp);		/*进行块的划分*/
        PUT(HDRP(bp),PACK(csize-asize,0));
        PUT(FTRP(bp),PACK(csize-asize,0));
		PUT(NEXT_LINKNODE_RP(bp),0);
        PUT(PREV_LINKNODE_RP(bp),0);
        coalesce(bp); /*合并，不成功就insert*/
    }
    else{
        PUT(HDRP(bp),PACK(csize,1));
        PUT(FTRP(bp),PACK(csize,1));
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp){
	if( bp == NULL) return;
    size_t size = GET_SIZE(HDRP(bp));
	
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
	
    PUT(NEXT_LINKNODE_RP(bp),NULL);
    PUT(PREV_LINKNODE_RP(bp),NULL);
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */

void *mm_realloc(void *ptr, size_t size){
    size_t oldsize = GET_SIZE(HDRP(ptr));
    void *newptr;
    size_t asize;

    if(size == 0){
        //mm_free(ptr);
        return NULL;
    }

    if(ptr == NULL) 
		return mm_malloc(size);

    if(size <= DSIZE){
        asize = 2*(DSIZE);
    }else{
        asize = (DSIZE)*((size+(DSIZE)+(DSIZE-1)) / (DSIZE));
    }

	if(oldsize >= asize){
		place(ptr,asize);
        return ptr;
	}else{
		/*先考虑ptr块与前后进行合并，得到合并后的块看是否满足要求，是就place进去；
		 *合并后仍不满足条件的话就malloc(size)
		 */
        int bp_not_change = 0;
        char *bp = realloc_coalesce(ptr, asize, &bp_not_change);
        if( bp_not_change == 1){/*仅后为空进行合并后的*/
            realloc_place(bp,asize);
            return bp;
        } else if( (bp_not_change == 0) && (bp != ptr) ){ /*前为空或者前后均为空进行合并后的*/
            memcpy(bp, ptr, size);
            realloc_place(bp,asize);
            return bp;
        }
        else{
            newptr = mm_malloc(size);
            memcpy(newptr, ptr, size);
            mm_free(ptr);
            return newptr;
        }
    }
}

/*使用合并空闲块，与coalesce不同的是利用bp_not_change来标记合并后bp位置是否变动以来决定是否copy*/
static void *realloc_coalesce(void *bp, size_t newSize, int *bp_not_change){
    size_t  prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t  next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    *bp_not_change = 0;

    if(prev_alloc && next_alloc){}
    else if(prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        if(size>=newSize){
            delete_from_list(NEXT_BLKP(bp));
            PUT(HDRP(bp), PACK(size,1));
            PUT(FTRP(bp), PACK(size,1));
            *bp_not_change = 1;
            return bp;
        }
    }
    else if(!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        if(size>=newSize){
            delete_from_list(PREV_BLKP(bp));
            PUT(FTRP(bp),PACK(size,1));
            PUT(HDRP(PREV_BLKP(bp)),PACK(size,1));
            bp = PREV_BLKP(bp);
            return bp;
        }
    }
    else{
        size += ( GET_SIZE(FTRP(NEXT_BLKP(bp)))+ GET_SIZE(HDRP(PREV_BLKP(bp))) );
        if(size >= newSize){
            delete_from_list(PREV_BLKP(bp));
            delete_from_list(NEXT_BLKP(bp));
            PUT(FTRP(NEXT_BLKP(bp)),PACK(size,1));
            PUT(HDRP(PREV_BLKP(bp)),PACK(size,1));
            bp = PREV_BLKP(bp);
        }
    }
    return bp;
}

/*重新实现realloc_place，默认bp为已经分配的块*/
static void realloc_place(void *bp,size_t asize){
    size_t csize = GET_SIZE(HDRP(bp));
	
	//delete_from_list(bp);		/*与place的区别：默认bp为已分配，所以不用delete*/
	if((csize-asize) >= (2*DSIZE)){ /*剩余块的大小>16字节（最小块的大小） 说明可划分*/
        PUT(HDRP(bp),PACK(asize,1));
        PUT(FTRP(bp),PACK(asize,1));
		
        bp = NEXT_BLKP(bp);		/*进行块的划分*/
        PUT(HDRP(bp),PACK(csize-asize,0));
        PUT(FTRP(bp),PACK(csize-asize,0));
		PUT(NEXT_LINKNODE_RP(bp),0);
        PUT(PREV_LINKNODE_RP(bp),0);
        coalesce(bp); /*合并，不成功就insert*/
    }
    else{
        PUT(HDRP(bp),PACK(csize,1));
        PUT(FTRP(bp),PACK(csize,1));
    }
}