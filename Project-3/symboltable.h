/*
* Compiler Design Project 3 : Semantic Analyser
*
* File        : symboltable.h
*
* Authors     : Karthik M - 15CO22, Kaushik S Kalmady - 15CO222
* Date        : 11-3-2018
*/



#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#define HASH_TABLE_SIZE 100
#define NUM_TABLES 10

int table_index = 0;
int current_scope = 0;

/* struct to hold each entry */
struct entry_s
{
	char* lexeme;
	double value;
	int data_type;
	int* parameter_list; // for functions
	int array_dimension;
	struct entry_s* successor;
};

typedef struct entry_s entry_t;

/* Wrapper for symbol table with pointer to symbol table of parent scope */
struct table_s
{
	entry_t** symbol_table;
	int parent;
};

typedef struct table_s table_t;

extern table_t symbol_table_list[NUM_TABLES];

// Initialise all symbol tables to NULL
// Add in YACC
// int i;
// for(i=0; i<NUM_TABLES;i++)
// {
// 	symbol_table_list[i].symbol_table = NULL;
// 	symbol_table_list[i].parent = -1;
// }

/* Create a new hash_table. */
entry_t** create_table()
{
	entry_t** hash_table_ptr = NULL; // declare a pointer

	/* Allocate memory for a hashtable array of size HASH_TABLE_SIZE */
	if( ( hash_table_ptr = malloc( sizeof( entry_t* ) * HASH_TABLE_SIZE ) ) == NULL )
    	return NULL;

	int i;

	// Intitialise all entries as NUscopeLL
    for( i = 0; i < HASH_TABLE_SIZE; i++ )
	{
		hash_table_ptr[i] = NULL;
	}

	return hash_table_ptr;
}

/*
"{" current_scope = create_new_scope()


"}" current_scope = symbol_table_list[current_scope].parent;
*/

int create_new_scope()
{
	table_index++;

	symbol_table_list[table_index].symbol_table = create_table();
	symbol_table_list[table_index].parent = current_scope;

	return table_index;
}

int exit_scope()
{
	return symbol_table_list[current_scope].parent;
}
/* Generate hash from a string. Then generate an index in [0, HASH_TABLE_SIZE) */
uint32_t hash( char *lexeme )
{
	size_t i;
	uint32_t hash;

	/* Apply jenkin's hash function
	* https://en.wikipedia.org/wiki/Jenkins_hash_function#one-at-a-time
	*/
	for ( hash = i = 0; i < strlen(lexeme); ++i ) {
        hash += lexeme[i];
        hash += ( hash << 10 );
        hash ^= ( hash >> 6 );
    }
	hash += ( hash << 3 );
	hash ^= ( hash >> 11 );
    hash += ( hash << 15 );

	return hash % HASH_TABLE_SIZE; // return an index in [0, HASH_TABLE_SIZE)
}

/* Create an entry for a lexeme, token pair. This will be called from the insert function */
entry_t *create_entry( char *lexeme, int value )
{
	entry_t *newentry;

	/* Allocate space for newentry */
	if( ( newentry = malloc( sizeof( entry_t ) ) ) == NULL ) {
		return NULL;
	}
	/* Copy lexeme to newentry location using strdup (string-duplicate). Return NULL if it fails */
	if( ( newentry->lexeme = strdup( lexeme ) ) == NULL ) {
		return NULL;
	}

	newentry->value = value;
	newentry->successor = NULL;
	newentry->parameter_list = NULL;
	newentry->array_dimension = -1;
	return newentry;
}

/* Search for an entry given a lexeme. Return a pointer to the entry of the lexeme exists, else return NULL */
entry_t* search(entry_t** hash_table_ptr, char* lexeme)
{
	uint32_t idx = 0;
	entry_t* myentry;

    // get the index of this lexeme as per the hash function
	idx = hash( lexeme );

	/* Traverse the linked list at this idx and see if lexeme exists */
	myentry = hash_table_ptr[idx];

	while( myentry != NULL && strcmp( lexeme, myentry->lexeme ) != 0 )
	{
		myentry = myentry->successor;
	}

	if(myentry == NULL) // lexeme is not found
		return NULL;

	else // lexeme found
		return myentry;

}

// Search recursively in every parent scope for lexeme
entry_t* search_recursive(char* lexeme)
{
	int idx = current_scope;
	entry_t* finder = NULL;

	while(idx != -1)
	{
		finder = search(symbol_table_list[idx].symbol_table, lexeme);

		if(finder != NULL)
			return finder;

		idx = symbol_table_list[idx].parent;
	}

	return finder;
}
/* Insert an entry into a hash table. */
entry_t* insert( entry_t** hash_table_ptr, char* lexeme, int value)
{
	// Make sure you pass the current scope symbol table here
	entry_t* finder = search( hash_table_ptr, lexeme );
	if( finder != NULL) // If lexeme already exists, don't insert, return NULL
	{
		return NULL; //capture this is callee code and do necessary error handling
	}

	uint32_t idx;
	entry_t* newentry = NULL;
	entry_t* head = NULL;

	idx = hash( lexeme ); // Get the index for this lexeme based on the hash function
	newentry = create_entry( lexeme, value ); // Create an entry using the <lexeme, token> pair

	if(newentry == NULL) // In case there was some error while executing create_entry()
	{
		printf("Insert failed. New entry could not be created.");
		exit(1);
	}

	head = hash_table_ptr[idx]; // get the head entry at this index

	if(head == NULL) // This is the first lexeme that matches this hash index
	{
		hash_table_ptr[idx] = newentry;
	}
	else // if not, add this entry to the head
	{
		newentry->successor = hash_table_ptr[idx];
		hash_table_ptr[idx] = newentry;
	}
	return hash_table_ptr[idx];
}

// This should be called after function definition/declaration
void fill_array_dimension(entry_t* entry, int dimension)
{
	entry->array_dimension = dimension;
}

void fill_parameter_list(entry_t* entry, int* list, int n)
{
	entry->parameter_list = (int *)malloc(n*sizeof(int));

	int i;
	for(i=0; i<n; i++)
		entry->parameter_list[i] = list[i];
}

// This is called after a function call to check if param list match
int check_parameter_list(entry_t* entry, int* list)
{
	int* parameter_list = entry->parameter_list;
	int n = sizeof(parameter_list) / sizeof(int);
	int m = sizeof(list) / sizeof(int);
	if(m != n)
	{
		return -1;
	}
	int i;
	for(i=0; i<n; i++)
	{
		if(list[i] != parameter_list[i])
			return 0;
	}
	return 1;
}


// Traverse the hash table and print all the entries
void display(int idx)
{
	int i;
	entry_t* traverser;
    printf("\n====================================================\n");
    printf(" %-20s %-20s %-20s\n","lexeme","value","data-type");
    printf("====================================================\n");
	entry_t** hash_table_ptr = symbol_table_list[idx].symbol_table;
	for( i=0; i < HASH_TABLE_SIZE; i++)
	{

		traverser = hash_table_ptr[i];
		while( traverser != NULL)
		{
			printf(" %-20s %-20d %-20d \n", traverser->lexeme, (int)traverser->value, traverser->data_type);
			traverser = traverser->successor;
		}
	}
    printf("====================================================\n");

}


void display_all()
{
		int i;
		for(i=0; i<table_index+1; i++){
			printf("Scope: %d\n",i);
			display(i);
			printf("\n\n");
		}

}
