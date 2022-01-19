#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>


/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

struct memoryList
{
  // doubly-linked list
  struct memoryList *last;
  struct memoryList *next;

  int size;            // How many bytes in this block?
  char alloc;          // 1 if this block is allocated,
                       // 0 if this block is free.
  void *ptr;           // location of block in memory pool.
};

struct memoryList* first_fit(size_t requested);
struct memoryList* best_fit(size_t requested);
struct memoryList* worst_fit(size_t requested);
struct memoryList* next_fit(size_t requested);

strategies myStrategy = NotSet;    // Current strategy


size_t mySize;
void *myMemory = NULL;

static struct memoryList *head;
static struct memoryList *next;


/* initmem must be called prior to mymalloc and myfree.

   initmem may be called more than once in a given exeuction;
   when this occurs, all memory you previously malloc'ed  *must* be freed,
   including any existing bookkeeping data.

   strategy must be one of the following:
		- "best" (best-fit)
		- "worst" (worst-fit)
		- "first" (first-fit)
		- "next" (next-fit)
   sz specifies the number of bytes that will be available, in total, for all mymalloc requests.
*/

void initmem(strategies strategy, size_t sz)
{
	myStrategy = strategy;

	/* all implementations will need an actual block of memory to use */
	mySize = sz;

	if (myMemory != NULL) free(myMemory); /* in case this is not the first time initmem2 is called */

	/* TODO: release any other memory you were using for bookkeeping when doing a re-initialization! */

	if(head != NULL){
		struct memoryList* trav;
		for(trav = head; trav != NULL; trav = trav->next){ // iterate through doubly linked list and free previous node
			free(trav->last);
		}
		free(trav); // free the created pointer
	}

	myMemory = malloc(sz);
	//printf("initmem start\n");

	/* TODO: Initialize memory management structure. */
	struct memoryList* new_node = (struct memoryList*) malloc(sizeof(struct memoryList));
	new_node->next = NULL;
	new_node->last = NULL;
	new_node->size = sz; // the first block size is set to be equal to memory pool size
	new_node->alloc = 0; // not allocated
	new_node->ptr = myMemory; // points to the same memory address as the memory pool
	head = new_node; // initialise head
	next = new_node; // initialise next


}

/* Allocate a block of memory with the requested size.
 *  If the requested block is not available, mymalloc returns NULL.
 *  Otherwise, it returns a pointer to the newly allocated block.
 *  Restriction: requested >= 1 
 */

void *mymalloc(size_t requested)
{	
	assert((int)myStrategy > 0);
	struct memoryList* trav = NULL;
	
	switch (myStrategy)
	  {
	  case NotSet: 
	            return NULL;
	  case First:
	            trav = first_fit(requested); // strategy for first fit
	            break;
	  case Best:
	            trav = best_fit(requested); // strategy for best fit
	            break;
	  case Worst:
	            trav = worst_fit(requested); // strategy for worst fit
	            break;
	  case Next:
	  			
	            trav = next_fit(requested); // strategy for next fit
	            //printf("trav %p\n", trav);
	            break;
	  }

	  if(trav == NULL){
	  	//printf("trav == NULL\n");
	  	return NULL;
	  }
	  
	if(trav->size > requested){
		//printf("trav->size > requested\n");
		struct memoryList* temp =  (struct memoryList*) malloc(sizeof(struct memoryList)); // temporary variable

		temp->last = trav; // add new node after previous node
		temp->next = trav->next; // set new nodes next pointer to the next node
		if(trav->next != NULL){ // if trav->next == tail ignore the code below
			trav->next->last = temp; // set next nodes last pointer to our new node
		}

		trav->next = temp; // next pointer for node before new node is set to new node

		temp->size = trav->size - requested; // subtract requested from the initial block of bytes and set to temp->size
		temp->ptr = trav->ptr + requested; // add requested to the location of the block 
		temp->alloc = 0; // unallocated
		trav->size = requested; // set the block size equal to requested 
		
	}
	
	trav->alloc = 1; // allocated
	return trav->ptr; // return the location of our block
}

// finds the first available block of memory larger or equal than the requested size
struct memoryList* first_fit(size_t requested){
	for(struct memoryList* trav = head; trav != NULL; trav = trav->next){
		if(trav->size >= requested && trav->alloc == 0){

			return trav;

		}
	}
	return NULL;
}

// finds the smallest available block of memory larger or equal than the requested size
struct memoryList* best_fit(size_t requested){
	struct memoryList* best = NULL, *trav;
	for(trav = head; trav != NULL; trav = trav->next){ // iterate through doubly linked list
		if(trav->size >= requested && trav->alloc == 0 && (best == 0 || trav->size < best->size)){ // find smallest that is larger or equal to the requested and save in variable best
			best = trav;
		}
	}
	return best;
}

// find the largest available block of memory larger or equal than the requested size
struct memoryList* worst_fit(size_t requested){
	struct memoryList* worst = NULL, *trav;
	for(trav = head; trav != NULL; trav = trav->next){ // iterate through doubly linked list
		if(trav->size >= requested && trav->alloc == 0 && (worst == 0 || trav->size > worst->size)){ // find largest node that is larger or equal to the requested and save in variable worst
			worst = trav;
		}
	}
	return worst;	
}
 
// find the first suitable block after the last block allocated 
struct memoryList* next_fit(size_t requested){
	if (next == NULL || next->next == NULL){ // set next equal to head if next is null
		next = head;
	}

	struct memoryList* start = next; // save next in start
	struct memoryList *trav = next; // save next in trav

	do{
		if(trav->size >= requested && trav->alloc == 0){ // if the size of trav is larger or equal to requested and its an unallocated block
			next = trav; // save trav in next
			return trav; 
		}
		trav = trav->next; // increment
		if(trav == NULL){ // if at start of doubly linked list
			trav = head; // set trav equal to head
		}
	}while(trav != start); // run as long as trav is different from start
	return NULL;
} 

/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void* block)
{	
	struct memoryList* trav;
	for(trav = head; trav != NULL; trav = trav->next){ // iterate through doubly linked list and set alloc = 0 if the Node is apparent
		if(trav->ptr == block){
			trav->alloc = 0;
			break;
		
		}
	}

	struct memoryList* temp = (struct memoryList*) malloc(sizeof(struct memoryList));

	// check if previous node is free and reduce to one contigous node
	if((trav->last != NULL) && (trav->last->alloc == 0)){
		temp = trav; // temporary equal trav
		trav = trav->last; // trav is set to previous node
		trav->size += temp->size; // add current and last node together and save in last node
		trav->next = temp->next; // current node is set to next node
		if(temp->next != NULL){ // check if temp == tail
			temp->next->last = trav; // set next nodes last pointer equal to trav
		}
		if (temp == next){ // if the current node is equal to next
			next = trav; // set next to the previous block
		}
		free(temp);

	}
	// check if next node is free and reduce to one contigous node
	if((trav->next != NULL) && (trav->next->alloc == 0)){
		
		temp = trav->next; // set temp to next node
		trav->size += temp->size; // add two nodes together and save in trav
		trav->next = temp->next; // set trav->last next pointer to temp next
		if (temp->next != NULL){ // if next nodes next pointer is != NULL
			temp->next->last = trav; // set next nodes next pointer to trav
		}
		if (temp == next){ // next node is equal to next
			next = trav; //set next
		}
		free(temp);


	}
	
}

/****** Memory status/property functions ******
 * Implement these functions.
 * Note that when refered to "memory" here, it is meant that the 
 * memory pool this module manages via initmem/mymalloc/myfree. 
 */

/* Get the number of contiguous areas of free space in memory. */
int mem_holes()
{
	int free_node = 0;
	for(struct memoryList* trav = head; trav != NULL; trav = trav->next){
		if(trav->alloc == 0){ // as long as the node is unallocated add the size of the block to free_mem and increment free_node
			free_node++;
		}
	}
	return free_node;
}

/* Get the number of bytes allocated */
int mem_allocated()
{
	int bytes = 0;
	for(struct memoryList* trav = head; trav != NULL; trav = trav->next){
		if(trav->alloc == 1){ // if the node is allocated add the size to bytes
			bytes += trav->size;
		}
	}
	return bytes;
}

/* Number of non-allocated bytes */
int mem_free()
{
	int free_mem = 0; // global variable for mem_free function
	for(struct memoryList* trav = head; trav != NULL; trav = trav->next){
		if(trav->alloc == 0){ // as long as the node is unallocated add the size of the block to free_mem and increment free_node
			free_mem += trav->size;
		}
	}
	return free_mem;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free()
{
	int largest = 0;
	for(struct memoryList* trav = head; trav != NULL; trav = trav->next){
		if((trav->alloc == 0) && trav->size > largest){ // finds the largest unallocated node
			largest = trav->size;
		}		
	}
	return largest;
}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size)
{
	int smallest = 0;	
	for(struct memoryList* trav = head; trav != NULL; trav = trav->next){
		if(trav->alloc == 0 && trav->size <= size){ // search for an unallocated block that is smaller or equal to size and increment smallest
			smallest++;	
		}
		
	}
	return smallest;
}       

/* Is the block allocated or not */
char mem_is_alloc(void *ptr)
{	
	struct memoryList* trav;
	for(trav = head; trav != NULL; trav = trav->next){
		if(trav->ptr == ptr){
			return trav->alloc;
		}
	}

	return trav->alloc;
} 

/* 
 * Feel free to use these functions, but do not modify them.  
 * The test code uses them, but you may find them useful.
 */


//Returns a pointer to the memory pool.
void *mem_pool()
{
	return myMemory;
}

// Returns the total number of bytes in the memory pool. */
int mem_total()
{
	return mySize;
}


// Get string name for a strategy. 
char *strategy_name(strategies strategy)
{
	switch (strategy)
	{
		case Best:
			return "best";
		case Worst:
			return "worst";
		case First:
			return "first";
		case Next:
			return "next";
		default:
			return "unknown";
	}
}

// Get strategy from name.
strategies strategyFromString(char * strategy)
{
	if (!strcmp(strategy,"best"))
	{
		return Best;
	}
	else if (!strcmp(strategy,"worst"))
	{
		return Worst;
	}
	else if (!strcmp(strategy,"first"))
	{
		return First;
	}
	else if (!strcmp(strategy,"next"))
	{
		return Next;
	}
	else
	{
		return 0;
	}
}


/* 
 * These functions are for you to modify however you see fit.  These will not
 * be used in tests, but you may find them useful for debugging.
 */

/* Use this function to print out the current contents of memory. */
void print_memory()
{
	printf("List of memory:\n");
	for(struct memoryList* trav = head; trav != NULL; trav = trav->next){
		printf("\tNode: %p \tsize: %d", trav->ptr, trav->size);
		if(trav->alloc == 0){
			printf("\tFREE\n");
		}
		else{
			printf("\tALLOCATED\n");
		}

	}
	printf("\n");
}


/* Use this function to track memory allocation performance.  
 * This function does not depend on your implementation, 
 * but on the functions you wrote above.
 */ 
void print_memory_status()
{
	printf("%d out of %d bytes allocated.\n",mem_allocated(),mem_total());
	printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
	printf("Average hole size is %f.\n\n",((float)mem_free())/mem_holes());

}

/* Use this function to see what happens when your malloc and free
 * implementations are called.  Run "mem -try <args>" to call this function.
 * We have given you a simple example to start.
 */
void try_mymem(int argc, char **argv) {
        strategies strat;
	void *a, *b, *c, *d, *e;
	if(argc > 1)
	  strat = strategyFromString(argv[1]);
	else
	  strat = First;
	
	
	/* A simple example.  
	   Each algorithm should produce a different layout. */
	
	initmem(strat,500);
	
	a = mymalloc(100);
	b = mymalloc(100);
	c = mymalloc(100);
	myfree(b);
	d = mymalloc(50);
	myfree(a);
	e = mymalloc(25);
	
	print_memory();
	print_memory_status();	
}