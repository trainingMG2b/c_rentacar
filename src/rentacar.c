#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdarg.h>

#include "rentacar.h"
#include "car.h"


// We use the C json parging library found at
//   https://github.com/DaveGamble/cJSON 


// Table person.db
// A person has a
//   lastname       char[ 64 ]
//   firstname      char[ 64 ]
//   civility       enum { MR = 0, MRS = 1, MISS = 2 }
//   gender	        enum { MALE = 0, FEMALE = 1 }
//   email          char[ 64 ]
//   driver_license char[ 64 ]
//   person_id      int > 0, unique
//
// Table car.db
// A car has a
//   brand          char[ 32 ]
//   model          char[ 64 ]
//   year           int
//   active         bool
//   number_plate   char[ 16 ]
//   car_id         int > 0, unique
//
// Table rental.db
// A person can have 0 or more cars between two dates, a car belongs to only one person
//   startdate      long
//   enddate        long -1 if not defined
//   active         enum { NO = 0, YES = 1 }
//   person_id      int, foreign key from person
//   car_id         int, foreign key from car
//   person_car_id  int > 0, unique
//
//
// Persons can not be deleted
// Cars can not be deleted
// Rentals records can be deleted


// Program entry point
int _main( int argc, char *argv[] ) {

	int rc = 0;
	int opt = 0;
	struct Arguments arguments;
	char * jsonText;

	/* Default values. */
	arguments.object = 0x00;
	arguments.operation = 0x00;
	arguments.jsonFileName = UNDEFINED;

	static struct option longOptions[] = {
		{ "car",      no_argument,       0,  'C' },
		{ "person",   no_argument,       0,  'P' },
		{ "rental",   no_argument,       0,  'R' },
		{ "create",   no_argument,       0,  'c' },
		{ "retrieve", no_argument,       0,  'r' },
		{ "update",   no_argument,       0,  'u' },
		{ "delete",   no_argument,       0,  'd' },
		{ 0,          0,                 0,  0   }
	};

	// pricess program arguments
	int longIndex = 0;
	while ( ( opt = getopt_long( argc, argv, "CPRcrud",
				   longOptions, &longIndex ) ) != -1 ) {
		switch ( opt ) {
			case 'C':
				arguments.object = 'C';
				break;
			case 'P':
				arguments.object = 'P';
				break;
			case 'R':
				arguments.object = 'R';
				break;
			case 'c':
				arguments.operation = 'c';
				break;
			case 'r':
				arguments.operation = 'r';
				break;
			case 'u':
				arguments.operation = 'u';
				break;
			case 'd':
				arguments.operation = 'd';
				break;
			default:
			 	_wrongArgumentsAndQuit();
		}
	}

	// get json file name, should be last (non option indicator) argument, and is mandatory
	if ( optind == argc -1 ) {

		char *fileName = argv[ optind ];
		if ( !_checkFileExistence( fileName ) )
			_wrongArgumentsAndQuit();

		arguments.jsonFileName = fileName;
	}
	else
		_wrongArgumentsAndQuit();

	// args Ok and json file exist and is readable, do some more sanity checks (a bit paranoiac... ;-)
	_doArgumentsSanityCheck( &arguments );

	// load json text
	jsonText = _loadJsonTextFromFile( arguments.jsonFileName );

	// just do it....
	rc = _processObjectOperation( &arguments, jsonText );

	// clean up....
	_cleanup( jsonText );

	return rc;
}

// Print program usage on standrd output
void usage() {

	printf(
		"Usage:\n\
rentacar\n\
        Without arguments : display usage\n\
\n\
rentacar -C -c json_file\n\
        Creates new car_id\n\
\n\
rentacar -C -r json_file\n\
        Retrieve all cars as specified in the json file (eg: { \"active\": true, \"car_id\": 123 })\n\
\n\
rentacar -C -u json_file\n\
        Update a car ientified by its car_id defined in the json file\n\
\n\
rentacar -C -d json_file\n\
        Delete a car by its car_id defined in the json file\n"
	);
}

// An error occured, print messge on standard error and exit with returnCode
void error( char * msg, int returnCode ) {

	fprintf( stderr, msg );
	exit( returnCode );
}

// Print message on standard error
void messageErr( const char *msg ) {
	
	fprintf( stderr, msg );
}

// Print message on standard output
void messageOut( const char *msg ) {
	
	fprintf( stdout, msg );
}

//
// private functions (should not be called outside of this file)
//

// Bad arguments, print usage and exit with related error code 
void _wrongArgumentsAndQuit() {
	
	usage();
	exit( EXIT_WRONG_ARGS );
}

// Check if a fileName exists
// fileName parameter is the full path to file
// Returns true if it existe, false otherwise
bool _checkFileExistence( const char *fileName ) {

	FILE *fp;

	fp = fopen ( fileName , "rb" );
	if ( !fp )
		return false;

	fclose( fp );

	return true;
}

// Do sanity checks on awaited arguments paramater
// arguments are the program arguments packed in a Arguments struccture
// Print message on standard error and quit if something went wrong
void _doArgumentsSanityCheck( const struct Arguments *arguments ) {

	if ( arguments->object == 0x00 ||
		arguments->operation == 0x00 ||
		strcmp( arguments->jsonFileName, UNDEFINED ) == 0 )
		_wrongArgumentsAndQuit();

	if ( strchr( OBJECTS, arguments->object ) == 0 && strchr( OPERATIONS, arguments->operation ) != 0 )
		_wrongArgumentsAndQuit();
}

// Load json string from file
// fileName is the name of the file containing the json text
// Returns json string from fileName parameter
char * _loadJsonTextFromFile( char *fileName ) {

	FILE *fp;
	long fileSize;
	char *buffer;

	fp = fopen( fileName , "rb" );
	if ( !fp )
		return 0x00;

	fseek( fp , 0L , SEEK_END );
	fileSize = ftell( fp );
	rewind( fp );

	// allocate memory for entire content
	buffer = calloc( 1, fileSize + 1 );

	// error on memory allocation
	if ( !buffer ) {

		fclose( fp );
		error( "Unable to allocate memory for Json text from file (too big?)", EXIT_OUT_OF_MEMORY );
	}

	// copy entire file into the buffer
	if ( fread( buffer , fileSize, 1 , fp) != 1 ) {

		fclose( fp );
		free( buffer );
		error( "Unable to load Json text from file", EXIT_LOAD_JSON_CONTENT );
	}

	// close file
	fclose( fp );

	// return buffer
	return buffer;
}

// Do object operation 
// based on arguments and json text parameters
// Returns id of processed object (>=1) or 0 on error
int _processObjectOperation( const struct Arguments *args, char *jsonText ) {

	int rc = 0;

	switch ( args->object ) {

		case 'C':
			rc = carOperation( args->operation, jsonText );
			break;
		
		default:
			break;
	}

	return rc;
}

// Free previously allocated buffer argument
void _cleanup( char * buffer ) {

	free( buffer );
}
