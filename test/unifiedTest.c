#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <ctype.h>

#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS 8
#endif

void hexdump(unsigned int *mem, unsigned int len)
{
	int i;
	for (i = 0; i < len; i ++)
		printf("%08x: %08x\n", (unsigned int)(mem+i), mem[i]);
}

void stm(unsigned long* regs)
{
	unsigned long reg0 = regs[0], reg1 = regs[1], reg2=regs[2];
	asm volatile (
	    "stmia  %0, {%1,%2}     \n\t"
	    :
	    :"r" (reg0), "r" (reg1), "r" (reg2)
	    :
	);
}

void ldm(unsigned long* regs)
{
	unsigned long reg0 = regs[0], reg1 = regs[3], reg2=regs[4], reg3, reg4;
	asm volatile (
	    "ldmia %2, {%0,%1}      \n\t"
	    :"=r" (reg3), "=r" (reg4)
	    :"r" (reg0)
	    :
	);

	regs[3]=reg3;
	regs[4]=reg4;
}

unsigned long check_eq(unsigned long* regs)
{
	unsigned long reg1 = regs[1], reg2=regs[2], reg3=regs[3],reg4=regs[4],
	              reg5=regs[5];
	asm volatile (
	    "cmp %1,%3    \n\t"
	    "cmpeq %2,%4    \n\t"
	    "moveq %0,#0x1    \n\t"
	    :"=r"(reg5)
	    :"r" (reg1), "r" (reg2), "r" (reg3), "r" (reg4)
	    :
	);

	return reg5;
}


void test_basic_ldst()
{
	char cat_maps[50];
	char clear_refs[50];
	volatile int * hugePointer, *hugePointer_1;
	void * assign;
	void * assign_1;
	int i,j, dummy;
	char c[100];
	pid_t pid = getpid();



	posix_memalign(&assign, 4096, 11*4096);
	hugePointer = (volatile int *)assign;
        sleep(2);
	//hugePointer_old = hugePointer;
	//posix_memalign(&assign_1, 4096, 11*4096);
	//hugePointer_1 = (volatile int *)assign_1;
	//printf("Touching new pages on the heap with store instructions\n");
        //sleep(2);
	sprintf(cat_maps,"cat /proc/%d/maps | grep -A 1 heap \0", pid);
	sprintf(clear_refs,"echo 1 > /proc/%d/clear_refs", pid);
        sleep(2);
		fflush(stdout);
		system(cat_maps);
	for (i = 1; i <= 11; i++) {
		fflush(stdout);
		system(cat_maps);
		for (j = 1; j <= i; j++) {
		fflush(stdout);
		system(cat_maps);
			hugePointer[i*1024] = j;
		}

		fflush(stdout);
		system(cat_maps);

		//printf("Verify correct value: ");

		if (hugePointer[i*1024] == i) printf("Success\n");
		else printf("Failure\n");
                sleep(1);

		fflush(stdout);
		system(cat_maps);
	}

	//printf("Clear out the memory references");
	//fflush(stdout);
	//system(clear_refs);
	//system(cat_maps);

	//printf("Touching new pages on the heap with load instructions\n");

	//hugePointer_old = hugePointer;
	//posix_memalign(&assign, 4096, 11*4096);
	//hugePointer = (volatile int *)assign;

/*
	for (i = 0; i < 6; i++) {
		printf("Hit page %d times\n",i);
		fflush(stdout);
		for (j = 1; j <= i; j++) {
			dummy = hugePointer_1[i*1024+j];
		}

		fflush(stdout);
		system(cat_maps);

		printf("Write and verify: ");

		hugePointer_1[i*1024+j] = j;

		if (hugePointer_1[i*1024+j] == j) printf("Success\n");
		else printf("Failure\n");

		fflush(stdout);
		system(cat_maps);
	}
*/
        printf("Heap test done\n");
	//free(hugePointer);
	//free(hugePointer_1);

}

void pc_relative_store(unsigned int a);
unsigned int pc_relative_load();

void test_basic_pc_relative()
{
	char cat_maps[100];
	char clear_refs[50];
	int i, j;
	pid_t pid = getpid();

	sprintf(cat_maps,
	        "cat /proc/%d/maps | grep -A 1 unified | grep -A 1 rwxp | tail -n 2 \0", pid);

	printf("Execute a pc relative instruction on a data abort\n");
	printf("These functions aligned at the beginning of a page and reference previous page\n");

	for (i = 0; i < 3; i++) {
		printf("Storing %d\n",i);
		pc_relative_store(i);

		fflush(stdout);
		system(cat_maps);

		printf("Loading back\n");
		j = pc_relative_load();
		fflush(stdout);
		system(cat_maps);
		if (j == i) printf("Success\n");
		else printf("Failure, received 0x%08x\n", j);
	}
}


void prefetch_1();
int prefetch_2(int a, int b);
int prefetch_3(int a, int b);
int prefetch_4(int * a);
int prefetch_5(int a, int b);
void prefetch_6();
void prefetch_7();
void prefetch_8();
void prefetch_9();
void prefetch_10();
void prefetch_11();

void test_prefetch()
{
	int i, j;
	int counter;
	int result;
	char cat_maps[100];

	pid_t pid = getpid();

	sprintf(cat_maps,"cat /proc/%d/maps | grep -A 1 unified | head -n 2 \0", pid);

	printf("Check the prefetch counting abilities\n");
	printf("For each size, branch to a small assembly routine\n");
	printf("Some do small tasks, others are just nops\n");

	printf("1: Return\n");
	prefetch_1();
	fflush(stdout);
	system(cat_maps);

	printf("2: Add: ");
	result = prefetch_2(4,5);
	if (result == 9) printf("Success\n");
	else printf("Failure: 4+5 != %d\n", result);
	fflush(stdout);
	system(cat_maps);

	printf("3: Compare: ");
	result = prefetch_3(3,3);
	if (result == 1) printf("Success\n");
	else printf("Failure: result=%d, expected 1\n",result);
	fflush(stdout);
	system(cat_maps);

	printf("4: load and And: ");
	counter = 0xFE1;
	result = prefetch_4(&counter);
	if (result == (0xFE1&0xFF0)) printf("Success\n");
	else printf("Failure: result=%d, expected 1\n",result);
	fflush(stdout);
	system(cat_maps);

	printf("5: Compare: ");
	result = prefetch_5(5,6);
	if (result == 5) printf("Success\n");
	else printf("Failure: result=%d, expected 5\n",result);
	fflush(stdout);
	system(cat_maps);

	printf("6: nops\n");
	prefetch_6();
	fflush(stdout);
	system(cat_maps);

	printf("7: nops\n");
	prefetch_7();
	fflush(stdout);
	system(cat_maps);

	printf("8: nops\n");
	prefetch_8();
	fflush(stdout);
	system(cat_maps);

	printf("9: nops\n");
	prefetch_9();
	fflush(stdout);
	system(cat_maps);

	printf("10: nops\n");
	prefetch_10();
	fflush(stdout);
	system(cat_maps);

	printf("11: nops\n");
	prefetch_11();
	fflush(stdout);
	system(cat_maps);

}

void test_mmap()
{
	int fd;
	int i;
	void *p;
	unsigned long volatile val;

	char cat_maps[50];
	unsigned long regs[6] = {0x40000000,0xA,0xB,0,0,0};

	pid_t pid = getpid();
	sprintf(cat_maps,"cat /proc/%d/maps | grep -A 1 zero \0",pid);

	fd = open("/dev/zero", O_RDWR);
	if (!fd) {
		perror("test_mmap");
		return;
	}

	p = mmap((void *)0x40000000, 1600, PROT_READ | PROT_WRITE,
	         MAP_PRIVATE | MAP_FILE, fd, 0);

	if (!p) {
		perror("test_mmap");
		return;
	}

	printf("mmap p %08x\n", (unsigned long)p);

	//      sleep(10);

#define SEC 1
	for (i = 0; i < 5; i++) {
		//printf("reading...\n");
		// val = *(volatile unsigned long *)(p);

		printf("storing multiple...\n");
		stm(regs);
		system(cat_maps);

		printf("loading multiple...\n");
		ldm(regs);
		system(cat_maps);

		if(check_eq(regs)) {
			printf("successful ldm/stm: %x,%x,%x,%x!\n",regs[1],regs[2],regs[3],regs[4]);
		} else {
			printf("failed ldm/stm: %x,%x,%x,%x!\n",regs[1],regs[2],regs[3],regs[4]);
		}

	}
}



int main()
{
        char command[50];

        //strcpy( command, "cat /proc/\'pgrep test\'/maps" );
        //system(command);

	char mesg[] = "HelloWorld";
	test_basic_ldst();
	//test_mmap();
	//test_basic_pc_relative();
	//test_prefetch();
	/*We need time to check process stats*/
//      sleep(100);
	//printf("%s\n", mesg);
//      hexdump((unsigned int *)0x8600, 128);
	printf("bye!\n");
	return 0;
}
