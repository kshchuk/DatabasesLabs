#include <stdbool.h>

typedef struct 
{
	int key;
	// Record number in master file
	int address;
	bool isValid;
} IndexFileRecord;