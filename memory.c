#include <errno.h>
#include "memory.h"
#include "bitset.h"

#include <stdint.h>
#include <unistd.h>


#define ORDER_MAX 16
#define ORDER_MIN 6
#define PAGE_SIZE 64

#define BUCKET_COUNT (ORDER_MAX - ORDER_MIN + 1)
#define HEAP_SIZE ((size_t)1 << ORDER_MAX) //65536


typedef struct list_t {
  size_t size;
  struct list_t *prev, *next;
} list_t;

static list_t buckets[BUCKET_COUNT];

static list_t *free_list;

//########Listen Funktionen#####################
static void list_init(list_t *list) {
  list->prev = list;
  list->next = list;
}

static void list_push(list_t *list, list_t *entry) {
  list_t *prev = list->prev;
  entry->prev = prev;
  entry->next = list;
  prev->next = entry;
  list->prev = entry;
}

static void list_remove(list_t *entry) {
  list_t *prev = entry->prev;
  list_t *next = entry->next;
  prev->next = next;
  next->prev = prev;
}

static list_t *list_pop(list_t *list) {
  list_t *back = list->prev;
  if (back == list) return NULL;
  list_remove(back);
  return back;
}

static void list_split(list_t *entry, size_t bucket, size_t chosenSize) {
list_remove(entry);
//printf("size after = %li \n", chosenSize >> 1);
//printf("entry    = %p\n", (void *)entry);
list_t *newEntry = (list_t *)((char *)entry + (chosenSize >> 1));//(char*)entry + (chosenSize >> 1);
//printf("newEntry = %p\n", (void *)newEntry);

list_push(&buckets[bucket+1], newEntry);
entry->size = (chosenSize >> 1);
newEntry->size = (chosenSize >> 1);
}

static int is_left_child(list_t *entry) {
size_t number = (HEAP_SIZE / (entry->size))/2;

for(int i =0 ; i < number; i++){
	if (entry == (free_list+ 2*i*(entry->size))){
		return 1;
	}
}
return 0;
}
// |..|..|..|..|..|..->|..|..|..|..|..|..|..|..|..|..|..|..|

static void list_merge(list_t *entry, size_t bucket, size_t size) {
if(bucket == 0){

}else{
	if(is_left_child(entry)){
		//printf("Linkes Kind erkannt\n");
		list_t *searched = (list_t *)((char *)entry + size);//(char*)entry + size;
		//printf("entry    = %p\n", (void *)entry);
		//printf("searched = %p\n", (void *)searched);
		list_t *temp = entry->next;
		while(temp!=entry){
     		if(searched == temp){
				list_remove(entry);
				list_remove(searched);
				entry->size= size << 1;
				list_push(&buckets[bucket-1], entry);
				list_merge(entry ,bucket-1, size << 1);
				break;
			}
     		temp= temp->next;
		}
	}else{
		//printf("Rechtes Kind erkannt\n");
		list_t *searched = (list_t *)((char *)entry - size);//(char*)entry - size;
		list_t *temp = entry->next;
		while(temp!=entry){
     		if(searched == temp){
				list_remove(entry);
				list_remove(searched);
				searched->size= size << 1;
				list_push(&buckets[bucket-1], searched);
				list_merge(searched, bucket-1, size << 1);
				break;
			}
     		temp= temp->next;
		}
	}
}
}
//########Listen Funktionen#####################

static size_t bucket_for_request(size_t request) {
  size_t bucket = BUCKET_COUNT - 1;
  size_t size = PAGE_SIZE;

  while (size < request) {
    bucket--;
    size *= 2;
  }

  return bucket;
}


static char heap[HEAP_SIZE];


void mem_init() {
	list_t *node = (list_t*) heap;
	node->size = HEAP_SIZE;
	node->prev = NULL;
	node->next = NULL;
	free_list = node;

	for (int i = 0; i <= (ORDER_MAX - ORDER_MIN); i++){
		list_init(&buckets[i]);
	}

	list_push(&buckets[0], free_list);
	//printf("Init abgeschlossen \n");
}

//     .....->..........................
void* mem_alloc(size_t size) {

size += sizeof(size_t);
if (size < PAGE_SIZE) {size = PAGE_SIZE;}

size_t power = 32;
while(power < size) {power*=2;}
size = power;

if(size > HEAP_SIZE) {goto fail;}

size_t newBucket = bucket_for_request(size);
list_t *result = NULL;
size_t chosenSize = size;

while(result == NULL){
	if(newBucket == -1){ perror("FAILURE TO ALLOCATE NEW MEMORY"), exit(-1);}
	result= list_pop(&buckets[newBucket]);
	newBucket--;
	chosenSize = chosenSize << 1;
}
chosenSize = chosenSize >> 1;
newBucket++;

while(chosenSize > size){
list_split(result, newBucket, chosenSize);
chosenSize = chosenSize >> 1;
newBucket ++;
}
return ((size_t*) result) + 1;

fail:
	errno = ENOMEM;
	return NULL;
}


void* mem_realloc(void *oldptr, size_t new_size) {

//alten Size aufbereiten
oldptr = (uint8_t *)oldptr - sizeof(size_t);
list_t *list = oldptr;
size_t size =  list->size;

//neue Size aufbereiten
new_size += sizeof(size_t);
if (new_size < PAGE_SIZE) {new_size = PAGE_SIZE;}
size_t power = 32;
while(power < new_size) {power*=2;}
new_size = power;
if(new_size > HEAP_SIZE) {goto fail;}

//1.Fall: Speicher ausreichend, nichts machen
if(size >= new_size){return ((size_t*) oldptr) + 1;}

//2.Fall:
size_t neededSize = new_size - sizeof(size_t);
list_t *newList = mem_alloc(neededSize);
oldptr = (uint8_t *)oldptr + sizeof(size_t);
memcpy(newList, oldptr, size);
oldptr = (uint8_t *)oldptr - sizeof(size_t);
//newList->size = new_size;
//printf("Größe des realloc Blocks %li:\n",newList->size);
oldptr = ((size_t*) oldptr) + 1;
mem_free(oldptr);
return ((size_t*) newList);

fail:
	errno = ENOMEM;
	return NULL;
}


void mem_free(void *ptr) {
if (!ptr) {return;}
ptr = (uint8_t *)ptr - sizeof(size_t);
list_t *list = ptr;
size_t size =  list->size;
size_t bucket = bucket_for_request(size);

list_push(&buckets[bucket], list);
list_merge(ptr, bucket, size);
}


void mem_dump(FILE *file) {
	size_t total_mem = 0;
	
	fputs("heap: {\n", file);
	fputs("\tlists: [\n", file);
	for(int i = 0; i < 11; i++){
		printf("Blöcke der Ordnung %d:\n",ORDER_MAX-i);
		list_t * start = (&buckets[i])->prev;
		fprintf(file, "\t\t{len: %lu },\n",start->size);
		total_mem += start->size;
		list_t * temp = start->next;

		while (temp != start) {
			fprintf(file, "\t\t{len: %lu },\n",temp->size);
			total_mem += temp->size;
			temp = temp->next;
		}
	}
	fputs("\t],\n", file);
	fprintf(
		file,
		"\tfree: %lu,\n"
		"\tcont: %lu\n",
		total_mem,
		HEAP_SIZE-total_mem
	);
	fputs("}\n", file);

}
