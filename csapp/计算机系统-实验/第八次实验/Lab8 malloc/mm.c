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
team_t team =
{
	/* Team name */
	"The Great 6064",
	/* First member's full name */
	"zsz - 6064",
	/* First member's email address */
	"NULL",
	/* Second member's full name (leave blank if none) */
	"",
	/* Second member's email address (leave blank if none) */
	""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

//=============================================================================
#define WSIZE 4        
#define DSIZE 8                
#define CHUNKSIZE (1<<12)       
#define MINFB_SIZE 2

#define MAX(x,y)    ((x)>(y)?(x):(y))

/* Pack a size and allocated bit into a word */
#define PACK(size,alloc)    ((size) | (alloc))

/* Read the size and allocated fields from address p */
#define GET(p)  (*(unsigned int *)(p))    
#define PUT(p,val)  (*(unsigned int *)(p) = (unsigned int)(val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)  
#define GET_ALLOC(p)    (GET(p) & 0x1)  

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)    ((char *)(bp)-WSIZE)                    
#define FTRP(bp)    ((char *)(bp)+GET_SIZE(HDRP(bp))-DSIZE)

/*Given free block ptr bp ,compute addredss of next ane previous free block */
#define PREV_LINKNODE_RP(bp) ((char*)(bp))          
#define NEXT_LINKNODE_RP(bp) ((char*)(bp)+WSIZE)    
/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)   ((char *)(bp)+GET_SIZE(((char *)(bp)-WSIZE))) 
#define PREV_BLKP(bp)   ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE))) 
//=============================================================================

static void* extend_heap(size_t size);

static void* find_fit(size_t size);

static void* coalesce(void* block_ponter);

static void place(void* block_pointer, size_t asize);

static void *realloc_coalesce(void *bp, size_t newSize, int *flag);

void Insert(char *p);

void Delete(char *p);

char *Get(size_t size);



static char *heap_listp = NULL;
static char *block_list_start = NULL;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
	if ((heap_listp = mem_sbrk(12 * WSIZE)) == (void *)-1) return -1;

	PUT(heap_listp, 0);              /*block size list<=32*/
	PUT(heap_listp + (1 * WSIZE), 0);    /*block size list<=64*/
	PUT(heap_listp + (2 * WSIZE), 0);    /*block size list<=128*/
	PUT(heap_listp + (3 * WSIZE), 0);    /*block size list<=256*/
	PUT(heap_listp + (4 * WSIZE), 0);    /*block size list<=512*/
	PUT(heap_listp + (5 * WSIZE), 0);    /*block size list<=2048*/
	PUT(heap_listp + (6 * WSIZE), 0);    /*block size list<=4096*/
	PUT(heap_listp + (7 * WSIZE), 0);    /*block size list>4096*/
	PUT(heap_listp + (8 * WSIZE), 0);	/* for alignment*/
	PUT(heap_listp + (9 * WSIZE), PACK(DSIZE, 1));
	PUT(heap_listp + (10 * WSIZE), PACK(DSIZE, 1));
	PUT(heap_listp + (11 * WSIZE), PACK(0, 1));

	block_list_start = heap_listp;
	heap_listp += (10 * WSIZE);

	if ((extend_heap(CHUNKSIZE / DSIZE)) == NULL) return -1;

	return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
	size_t asize;
	size_t extendsize;
	char *bp;
	if (size == 0) 
		return NULL;
	
	if (size <= DSIZE) 
		asize = 2 * (DSIZE);//������С���С
	else 
		asize = (DSIZE)*((size + (DSIZE)+(DSIZE - 1)) / (DSIZE));//�������Ҫ��

	if ((bp = find_fit(asize)) != NULL) 
	{
		place(bp, asize);
		/*printf("\nsize malloc address = x%x   size = %u", bp, asize);*/
		return bp;
	}

	extendsize = MAX(asize, CHUNKSIZE);
	if ((bp = extend_heap(extendsize / DSIZE)) == NULL)
		return NULL;
	place(bp, asize);
	return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *block_pointer)
{
	if (block_pointer == NULL)//�����쳣�������ָ��ֱ�ӷ���
		return;
	size_t size = GET_SIZE(HDRP(block_pointer));

	PUT(HDRP(block_pointer), PACK(size, 0));
	PUT(FTRP(block_pointer), PACK(size, 0));
	PUT(NEXT_LINKNODE_RP(block_pointer), NULL);
	PUT(PREV_LINKNODE_RP(block_pointer), NULL);
	coalesce(block_pointer);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *block_pointer, size_t size)
{
	size_t oldsize = GET_SIZE(HDRP(block_pointer));//�ɵĿ��С
	void *newblock_pointer;
	size_t asize,csize;

	//printf("\n\nnew realloc function\nbefore realloc:");
	//printblock(block_pointer);
	//printf("\ntargte size : %u\n", size);

	if (size == 0) {
		mm_free(block_pointer);
		return 0;
	}

	if (block_pointer == NULL) return mm_malloc(size);//�������������������

	/*compute the total size,which contanins header + footer + payload and fit the alignment requirement*/
	if (size <= DSIZE) {
		asize = 2 * (DSIZE);
	}
	else {
		asize = (DSIZE)*((size + (DSIZE)+(DSIZE - 1)) / (DSIZE));
	}

	if (oldsize == asize) return block_pointer;

	if (oldsize < asize)
	{
		int flag;
		char *bp = realloc_coalesce(block_pointer, asize, &flag);//����coalesce֮���ܲ��ܾͽ�ʹ��
		if (flag == 1)
		{ /*next block is free*/
			csize = GET_SIZE(HDRP(bp));
			PUT(HDRP(bp), PACK(csize, 1));
			PUT(FTRP(bp), PACK(csize, 1));
			//printf("after alloc, coalesce next free block\n");
			//printblock(bp);
			//printf("end realloc function");
			return bp;
		}
		else if (flag == 0 && bp != block_pointer)
		{ /*previous block is free, move the point to new address,and move the payload*/
			memmove(bp, block_pointer, size);//ʹ��memmove��memcpy�����ڴ�λ���ص��ĸ��������δ����ģ������������˼��bug��
			//printf("\nmemcpy copied %u bytes from 0x%x to 0x%x", size, block_pointer, bp);
			csize = GET_SIZE(HDRP(bp));
			PUT(HDRP(bp), PACK(csize, 1));
			PUT(FTRP(bp), PACK(csize, 1));
			//printf("\nold address 0x%x", block_pointer);
			//printf("\nnew address 0x%x", bp);
			//printf("after alloc, coalesce last free block\n");
			//printblock(bp);
			//printf("end realloc function");
			return bp;
		}
		else
		{
			/*realloc_coalesce is fail*/
			newblock_pointer = mm_malloc(size);
			memcpy(newblock_pointer, block_pointer, size);
			mm_free(block_pointer);
			//printf("after alloc, malloc a brand new block\n");
			//printblock(newblock_pointer);
			//printf("end realloc function");
			return newblock_pointer;
		}
	}
	else
	{/*just change the size of block_pointer*/
		csize = GET_SIZE(HDRP(block_pointer));
		PUT(HDRP(block_pointer), PACK(csize, 1));
		PUT(FTRP(block_pointer), PACK(csize, 1));
		//printf("after alloc, the old one is big enough, nothing changed\n");
		//printblock(block_pointer);
		//printf("end realloc function");
		return block_pointer;
	}
}

static void *extend_heap(size_t dwords)
{
	char *bp;
	size_t size;

	size = (dwords % 2) ? (dwords + 1) * DSIZE : dwords * DSIZE; /*compute the size fit the 16 bytes alignment*/

	if ((long)(bp = mem_sbrk(size)) == (void *)-1)		/*move the brk pointer for bigger heap*/
		return NULL;

	/*init the head and foot fields*/
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));

	/*init the prev and next free pointer fields*/
	PUT(NEXT_LINKNODE_RP(bp), NULL);//��ǰ����һ�����������еĹ¶��Ŀ�
	PUT(PREV_LINKNODE_RP(bp), NULL);

	/*the  epilogue header*/
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

	return coalesce(bp);
}

static void *coalesce(void *bp)
{
	size_t  prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t  next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));


	/*coalesce the block and change the point*/
	if (prev_alloc && next_alloc) {		//case1
	}
	else if (prev_alloc && !next_alloc)  //case2
	{
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		Delete(NEXT_BLKP(bp)); /*remove the next free block from the free list */
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}
	else if (!prev_alloc && next_alloc)	//case 3
	{
		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
		Delete(PREV_BLKP(bp)); /*remove the prev free block from the free list */
		PUT(FTRP(bp), PACK(size, 0));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		bp = PREV_BLKP(bp);
	}
	else	//case 4
	{
		size += GET_SIZE(FTRP(NEXT_BLKP(bp))) + GET_SIZE(HDRP(PREV_BLKP(bp)));
		Delete(PREV_BLKP(bp));
		Delete(NEXT_BLKP(bp));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		bp = PREV_BLKP(bp);
	}
	/*insert the new free block*/
	Insert(bp);
	return bp;
}

static void *find_fit(size_t size)
{

	char *root = Get(size);
	char *tmpP = GET(root);
	for (; root != (heap_listp - (2 * WSIZE)); root += WSIZE)
	{
		char *tmpP = GET(root);
		while (tmpP != NULL)
		{
			if (GET_SIZE(HDRP(tmpP)) >= size) 
				return tmpP;
			tmpP = GET(NEXT_LINKNODE_RP(tmpP));
		}
	}
	return NULL;
}

static void place(void *block_pointer, size_t asize)
{
	size_t csize = GET_SIZE(HDRP(block_pointer));
	Delete(block_pointer);/*remove from empty_list*/
	if ((csize - asize) >= (MINFB_SIZE*DSIZE))
	{
		PUT(HDRP(block_pointer), PACK(asize, 1));
		PUT(FTRP(block_pointer), PACK(asize, 1));
		block_pointer = NEXT_BLKP(block_pointer);

		PUT(HDRP(block_pointer), PACK(csize - asize, 0));
		PUT(FTRP(block_pointer), PACK(csize - asize, 0));

		PUT(NEXT_LINKNODE_RP(block_pointer), 0);
		PUT(PREV_LINKNODE_RP(block_pointer), 0);
		coalesce(block_pointer);//insert����ֻ��coalesce��free������ã�
	}
	else
	{
		PUT(HDRP(block_pointer), PACK(csize, 1));
		PUT(FTRP(block_pointer), PACK(csize, 1));
	}
}

//reallocʱ���Դ����Ѱ�����ڵĿ��ÿ������Ч��
static void *realloc_coalesce(void *bp, size_t newSize, int *flag)
{
	size_t  prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t  next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));
	*flag = 0;
	/*coalesce the block and change the point*/
	if (prev_alloc && next_alloc)
	{

	}
	else if (prev_alloc && !next_alloc)
	{
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		if (size >= newSize)
		{
			Delete(NEXT_BLKP(bp));
			PUT(HDRP(bp), PACK(size, 1));
			PUT(FTRP(bp), PACK(size, 1));
			*flag = 1;
			return bp;
		}
	}
	else if (!prev_alloc && next_alloc)
	{
		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
		if (size >= newSize)
		{
			Delete(PREV_BLKP(bp));
			PUT(FTRP(bp), PACK(size, 1));
			PUT(HDRP(PREV_BLKP(bp)), PACK(size, 1));
			bp = PREV_BLKP(bp);
			return bp;
		}

	}
	else
	{
		size += GET_SIZE(FTRP(NEXT_BLKP(bp))) + GET_SIZE(HDRP(PREV_BLKP(bp)));
		if (size >= newSize)
		{
			Delete(PREV_BLKP(bp));
			Delete(NEXT_BLKP(bp));
			PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 1));
			PUT(HDRP(PREV_BLKP(bp)), PACK(size, 1));
			bp = PREV_BLKP(bp);
			//printf("  note:next block used   ");
		}

	}
	return bp;
}

//��size��С���м����ڷ������������е�Ѱַ
 char *Get(size_t size)
{
	int i = 0;
	if (size <= 8) 
		i = 0;
	else if (size <= 16) 
		i = 0;
	else if (size <= 32) 
		i = 0;
	else if (size <= 64) 
		i = 1;
	else if (size <= 128) 
		i = 2;
	else if (size <= 256) 
		i = 3;
	else if (size <= 512) 
		i = 4;
	else if (size <= 2048) 
		i = 5;
	else if (size <= 4096) 
		i = 6;
	else 
		i = 7;
	return block_list_start + (i*WSIZE);
}

 //�ڷ������������в���һ�����
 void Insert(char *p)
{
	char *root = Get(GET_SIZE(HDRP(p)));
	char *prevp = root;
	char *nextp = GET(root);

	/*find the postion to insert, smaller < p < bigger*/
	while (nextp != NULL) 
	{
		if (GET_SIZE(HDRP(nextp)) >= GET_SIZE(HDRP(p))) 
			break;
		prevp = nextp;
		nextp = GET(NEXT_LINKNODE_RP(nextp));
	}
	/*insert*/
	if (prevp == root) {//��ǰ����ǵȼ���ĵ�һ�����
		PUT(root, p);
		PUT(NEXT_LINKNODE_RP(p), nextp);
		PUT(PREV_LINKNODE_RP(p), NULL);//��һ���������Ƚ��ΪNULL
		if (nextp != NULL) 
			PUT(PREV_LINKNODE_RP(nextp), p);
	}
	else {
		PUT(NEXT_LINKNODE_RP(prevp), p);
		PUT(PREV_LINKNODE_RP(p), prevp);
		PUT(NEXT_LINKNODE_RP(p), nextp);
		if (nextp != NULL) 
			PUT(PREV_LINKNODE_RP(nextp), p);
	}

}

 //�ӷ�������������ɾ��һ�����
 void Delete(char *p)
{
	char *root = Get(GET_SIZE(HDRP(p)));
	char *prevp = GET(PREV_LINKNODE_RP(p));
	char *nextp = GET(NEXT_LINKNODE_RP(p));

	if (prevp == NULL) 
	{
		if (nextp != NULL)
			PUT(PREV_LINKNODE_RP(nextp), 0);
		PUT(root, nextp);
	}
	else 
	{
		if (nextp != NULL)
			PUT(PREV_LINKNODE_RP(nextp), prevp);
		PUT(NEXT_LINKNODE_RP(prevp), nextp);
	}


	PUT(NEXT_LINKNODE_RP(p), NULL);
	PUT(PREV_LINKNODE_RP(p), NULL);
}