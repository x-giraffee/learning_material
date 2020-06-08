//#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include "cachelab.h"

//#define DEBUG_ON 
#define ADDRESS_LENGTH 64

/* Type: Memory address */
typedef unsigned long long int mem_addr_t;

/* Type: Cache line
   LRU is a counter used to implement LRU replacement policy  */
typedef struct cache_line {
	char valid;
	mem_addr_t tag;
	unsigned long long int lru;//lru�����������ʹ�õĴ��������û���ȥ
} cache_line_t;//������һ�����ٻ�����������

typedef cache_line_t* cache_set_t;//ָ����ٻ������ĵ�ַ
typedef cache_set_t* cache_t;//��ַ�ĵ�ַ��

/* Globals set by command line args */
int verbosity = 0; /* print trace if set */
int s = 0; /* set index bits */
int b = 0; /* block offset bits */
int E = 0; /* associativity */
char* trace_file = NULL;

/* Derived from command line args */
int S; /* number of sets */
int B; /* block size (bytes) */

/* Counters used to record cache statistics */
int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;
unsigned long long int lru_counter = 1;

/* The cache we are simulating */
cache_t cache;
mem_addr_t set_index_mask;

/*
 * initCache - Allocate memory, write 0's for valid and tag and LRU
 * also computes the set_index_mask
 */
void initCache()//��ʼ��һ����������
{
	//todo...
   /*
   Ӧ����γ�ʼ����
   ���������SEBΪcache����ռ䣬�ֱ����ǣ���������ƫ�Ƶ�ַ�Ĵ�С
   */
	int i, j;
	cache = (cache_set_t*)malloc(sizeof(cache_set_t)*S);
	for (i = 0; i < S; i++)
	{
		cache[i] = (cache_line_t*)malloc(sizeof(cache_line_t)*E);
		for (j = 0; j < E; j++)
		{
			cache[i][j].valid = 0;
			cache[i][j].tag = 0;
			cache[i][j].lru = 0;
		}
	}
	//�ǵ����Լ��Ļ���һ�£���Ȼ�ᱻʶ���ǳ��𰸵�
	//������ע��Ҳ������ɾ��
	//�������Ǽ�����ν��set index mask
	set_index_mask = (mem_addr_t)(pow(2, s) - 1);
}


/*
 * freeCache - free allocated memory
 */
void freeCache()
{
	//todo...
	int i, j;
	for (i = 0; i < S; i++)
		free(cache[i]);
	free(cache);

}


/*
 * accessData - Access data at memory address addr.
 *   If it is already in cache, increast hit_count
 *   If it is not in cache, bring it in cache, increase miss count.
 *   Also increase eviction_count if a line is evicted.
 */
void accessData(mem_addr_t addr)
{
	//todo...Ҫ�������룬��Ҫ�ۼӼ�������
	unsigned long long int min_lru = 100000;
	int hitted = 0;
	int not_full = 0;
	mem_addr_t input_tag;
	int group_index;
	group_index = (int)((addr << (ADDRESS_LENGTH - b - s) >> (ADDRESS_LENGTH - s)));
	input_tag = addr >> (b + s);
	int j;
	for (j = 0; j < E; j++)
	{
		if (cache[group_index][j].valid && (cache[group_index][j].tag == input_tag))//���λ��Ч�ұ��λƥ��
		{
			hit_count++;
			cache[group_index][j].lru++;
			hitted = 1;
			break;
		}
		if (!cache[group_index][j].valid)
			not_full = 1;
	}
	if (!hitted)
	{
		miss_count++;
		if (not_full)
		{
			for (j = 0; j < E; j++)
			{
				if (!cache[group_index][j].valid)
				{
					cache[group_index][j].valid = 1;
					cache[group_index][j].tag = input_tag;
				}
			}
		}
		else
		{
			eviction_count++;
			for (j = 0; j < E; j++)//�ҵ���Сlru
			{
				if (cache[group_index][j].lru <= min_lru)
					min_lru = cache[group_index][j].lru;
			}
			for (j = 0; j < E; j++)
			{
				if (cache[group_index][j].lru == min_lru)//Ϊ��Сlru��Ӧ��λ������ԭֵ
				{
					cache[group_index][j].valid = 1;
					cache[group_index][j].tag = input_tag;
				}
			}
		}
	}
}


/*
 * replayTrace - replays the given trace file against the cache
 ˵���˾������������װ��
 */
void replayTrace(char* trace_fn)
{
	char buf[1000];
	mem_addr_t addr = 0;
	unsigned int len = 0;//ƫ�Ƶ�ַ��
	FILE* trace_fp = fopen(trace_fn, "r");

	if (!trace_fp) {
		fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
		exit(1);
	}

	while (fgets(buf, 1000, trace_fp) != NULL) {
		if (buf[1] == 'S' || buf[1] == 'L' || buf[1] == 'M') {
			sscanf(buf + 3, "%llx,%u", &addr, &len);

			if (verbosity)
				printf("%c %llx,%u ", buf[1], addr, len);

			accessData(addr);//��

			/* If the instruction is R/W then access again */
			if (buf[1] == 'M')
				accessData(addr);//д(�����Ҫ�Ļ�)

			if (verbosity)
				printf("\n");
		}
	}

	fclose(trace_fp);
}

/*
 * printUsage - Print usage info
 */
void printUsage(char* argv[])
{
	printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
	printf("Options:\n");
	printf("  -h         Print this help message.\n");
	printf("  -v         Optional verbose flag.\n");
	printf("  -s <num>   Number of set index bits.\n");
	printf("  -E <num>   Number of lines per set.\n");
	printf("  -b <num>   Number of block offset bits.\n");
	printf("  -t <file>  Trace file.\n");
	printf("\nExamples:\n");
	printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
	printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
	exit(0);
}

/*
 * main - Main routine
 */
int main(int argc, char* argv[])
{
	char c;

	while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
		switch (c) {
		case 's':
			s = atoi(optarg);
			break;
		case 'E':
			E = atoi(optarg);
			break;
		case 'b':
			b = atoi(optarg);
			break;
		case 't':
			trace_file = optarg;
			break;
		case 'v':
			verbosity = 1;
			break;
		case 'h':
			printUsage(argv);
			exit(0);
		default:
			printUsage(argv);
			exit(1);
		}
	}

	/* Make sure that all required command line args were specified */
	if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
		printf("%s: Missing required command line argument\n", argv[0]);
		printUsage(argv);
		exit(1);
	}

	/* Compute S, E and B from command line args */
	//sEb��֪
	S = (int)pow(2, (double)s);
	B = (int)pow(2, (double)b);


	/* Initialize cache */
	initCache();

#ifdef DEBUG_ON
	printf("DEBUG: S:%u E:%u B:%u trace:%s\n", S, E, B, trace_file);
	printf("DEBUG: set_index_mask: %llu\n", set_index_mask);
#endif

	replayTrace(trace_file);

	/* Free allocated memory */
	freeCache();

	/* Output the hit and miss statistics for the autograder */
	printSummary(hit_count, miss_count, eviction_count);
	return 0;
}
