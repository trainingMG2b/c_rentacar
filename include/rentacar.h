#ifndef _RENTACAR_H
#define _RENTACAR_H

#include <stdbool.h>

#define EXIT_WRONG_ARGS				-100
#define EXIT_OUT_OF_MEMORY			-101
#define EXIT_LOAD_JSON_CONTENT		-102

#define MAX_MESSAGE_LEN				256

#define UNDEFINED			"__undefined"

#define OBJECTS				"CPR"
#define OPERATIONS			"crud"

struct Arguments {

	// object that is to operate on ( Car, Person or Rental)
	char object;      // C P R

	// operation to apply on object ( create, retrieve, update, delete )
	char operation;   // c r u d

	// json file that contains the needed information
	char *jsonFileName;
};

void usage();
void messageErr( const char * );
void messageOut( const char * );

// private functions
void _wrongArgumentsAndQuit();
bool _checkFileExistence( const char * );
char * _loadJsonTextFromFile( char * );
void _doArgumentsSanityCheck( const struct Arguments * );
int _processObjectOperation( const struct Arguments *, char * );
void _cleanup( char * );

#endif
