#include <stdio.h>
#include "memory.h"

static int compare_array_content(int *array1, int *array2, int size)
{
    int result = 1;

    for (int i = 0; i < size; i++)
    {

        if (array1[i] != array2[i])
        {
            result = 0;
            break;
        }
    }

    return result;
}

int main()
{
    mem_init();

    // Test mem_alloc and free
    int *array;
    int init_size = 10;
    array = mem_alloc(sizeof(int) * init_size);

    if (array == NULL)
        perror("mem_alloc()"), exit(-1);

    for (int i = 0; i < init_size; i++)
        array[i] = i;


    mem_dump(stdout);
    mem_free(array);
    mem_dump(stdout);
    printf("Test 1 abgeschlossen \n \n");

    // Test 2 realloc
    init_size = 50;
    int new_size = 130;

    array = mem_alloc(sizeof(int) * init_size); //200 Bytes -> 256 Bytes (Ordnung 8)
    int *test_array = mem_alloc(sizeof(int) * init_size); //200 Bytes -> 256 Bytes (Ordnung 8)

    
    
	if (array == NULL)
        perror("mem_alloc()"), exit(-1);

    for (int i = 0; i < init_size; i++)
    {
        array[i] = rand() % 50;
        test_array[i] = array[i];
    }

    array = mem_realloc(array, sizeof(int) * new_size); //520 Bytes -> 1024 Bytes (Ordnung 10)

    if (compare_array_content(array, test_array, init_size))
    {
        printf("Werte erfolgreich kopiert bei mem_realloc \n");
    }
    else
    {
        printf("Kopieren der Werte bei realloc fehlgeschlagen \n");
    }

    for (int i = init_size; i < new_size; i++)
    {
        array[i] = i;
    }

    mem_dump(stdout);
    mem_free(array);
    mem_dump(stdout);
    mem_free(test_array);
    mem_dump(stdout);
    printf("Test 2 abgeschlossen \n\n");

	printf("Test 3:\n");
    // Test3 Max fragmentation
	int *array_max[1024];
	for (int j = 0; j< 1024; j++){
		array_max[j]= mem_alloc(sizeof(int));
	}

	for (int j = 0; j< 1024; j+=2){
		mem_free(array_max[j]);
	}

	mem_dump(stdout);
	int *frag_array = mem_alloc(sizeof(int) * 16); //64 Bytes -> 128 Bytes (Ordnung 7)
	mem_free(frag_array);
}