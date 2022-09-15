#ifndef _RENTACAR_H
#define _RENTACAR_H

#include <stdbool.h>

#define EXIT_WRONG_ARGS					-100
#define EXIT_OUT_OF_MEMORY				-101
#define EXIT_LOAD_JSON_CONTENT			-102

#define EXIT_SERVER_SOCKET_ERROR		-103
#define EXIT_SERVER_SOCKET_CONFIG_ERROR	-104
#define EXIT_SERVER_SOCKET_BIND_ERROR	-105
#define EXIT_SERVER_SOCKET_LISTEN_ERROR	-106

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

void messageErr( const char * );
void messageOut( const char * );

#endif
