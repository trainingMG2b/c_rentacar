#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <stdbool.h>
#include <time.h>
#include <getopt.h>
#include <stdarg.h>
#include <signal.h>

#include "car.h"
#include "data.h"
#include "server.h"


// Function prototypes
static int _executeRequest( const int, const int );
static bool _parseRequest( const char *, const int );
static void _populateRequest( const int, const char *, const int );
static bool _appendJson( const char *, const int );
static void _resetRequest( const int );
static char * _prettyRequestName( const int );
static bool _appendHeader( char *, const int  );
static int _parsePathForCarId( const char * );
static void _sendErrorResponse( const int, const char *, const char *, const int, const char *, const char *, const char * );
static void _sendJsonResponse( const int, const char *, const char * );
static void _sendCarResponse( const int, const struct Car *, int, char * );
static int _processFullRequest( char *, const int, int *, const int, const int );
static void _signalHandler( int );
static void _serverUsage();


// Global variables
struct Request request[ MAX_CLIENTS ];
bool requestInProgress[ MAX_CLIENTS ];
bool debugRequest = false;


// Program entry point
// By default start in standard mode (curl, postman HTTP requests compatibility mode)
int main( int argc, char *argv[] ) {

	int serverSocket, addressLength, newSocket, clientSockets[ MAX_CLIENTS ];
    int activity, i, j, bytesRead, socketDescriptor;
	int maxSockectDescriptor;
	struct sockaddr_in address;
	char dataBuffer[ MAX_DATA_LENGTH + 1 ];
	char debug[ MAX_DATA_LENGTH + 1 ];
	fd_set readFdSet;
		
	char *telnetWelcomeMessage = SERVER_UP_MESSAGE;
	bool telnetMode = false;

	// Process expected program arguments
	int longIndex = 0;
	int opt = 0;
	static struct option longOptions[] = {
		{ "telnet",   no_argument,       0,  't' },
		{ "debug",    no_argument,       0,  'd' },
		{ "help",     no_argument,       0,  'h' },
		{ 0,          0,                 0,  0   }
	};

	// TODO : unhardcode options values
	while ( ( opt = getopt_long( argc, argv, "tdh",
				   longOptions, &longIndex ) ) != -1 ) {
		switch ( opt ) {
			case 't':
				telnetMode = true;
				break;
			case 'd':
				debugRequest = true;
				break;
			case 'h':
				_serverUsage();
				exit( 0 );
			default:
				_serverUsage();
				exit( EXIT_WRONG_ARGS );
		}
	}

	// Catch SIGHUP signal to stop processing and exit
	signal( SIGHUP, _signalHandler );

	// Initialise all clientSockets[] to 0 (and so, not checked)
	for ( i = 0; i < MAX_CLIENTS; i++ ) {

		clientSockets[ i ] = 0;
		_resetRequest( i );
	}
		
	// Create a master socket
	if ( ( serverSocket = socket( AF_INET, SOCK_STREAM, 0 ) ) == 0 ) {

		perror( OPEN_SERVER_SOCKET_ERROR );
		exit( EXIT_SERVER_SOCKET_ERROR );
	}
	
	// Set master socket to allow multiple connections
	if (  setsockopt( serverSocket, SOL_SOCKET, SO_REUSEADDR, &( int ){ 1 }, sizeof( int ) ) < 0 ) {

		perror( SERVER_SOCKET_CONFIG_ERROR );
		exit( EXIT_SERVER_SOCKET_CONFIG_ERROR );
	}
	
	// Master socket is of type INET and listens on relevant port
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( TCP_IP_LISTEN_PORT );
		
	// Bind the socket to predefined (localhost at least) ip addresses
	if ( bind( serverSocket, ( struct sockaddr * ) &address, sizeof( address ) ) < 0 ) {

		perror( SERVER_SOCKET_BIND_ERROR );
		exit( EXIT_SERVER_SOCKET_BIND_ERROR );
	}

	printf( LISTENING_ON_PORT_MSG, TCP_IP_LISTEN_PORT );
		
	// Specify maximum number of pending connections for the master socket
	if ( listen( serverSocket, MAX_PENDING_CONNECTIONS ) < 0 ) {

		perror( SERVER_SOCKET_LISTEN_ERROR );
		exit( EXIT_SERVER_SOCKET_LISTEN_ERROR );
	}
		
	// Accept incoming connections
	addressLength = sizeof( address );
	puts( SERVER_WAITING_MSG );

	// Main loop	
	while ( true ) {

		// Clear the socket set
		FD_ZERO( &readFdSet );
	
		// Add master socket to set
		FD_SET( serverSocket, &readFdSet );
		maxSockectDescriptor = serverSocket;
			
		// Add child sockets to set
		for ( i = 0 ; i < MAX_CLIENTS ; i++ ) {

			// Set socket descriptor
			socketDescriptor = clientSockets[ i ];
				
			// If valid socket descriptor then add to read list
			if ( socketDescriptor > 0 )
				FD_SET( socketDescriptor, &readFdSet );
				
			// Highest file descriptor number, needed for the select function
			if ( socketDescriptor > maxSockectDescriptor )
				maxSockectDescriptor = socketDescriptor;
		}
	
		// Wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
		activity = select( maxSockectDescriptor + 1, &readFdSet, NULL, NULL, NULL);
	
		if ( ( activity < 0 ) && ( errno != EINTR ) )
			printf( SOCKET_SELECT_ERROR );
			
		// If something happened on the master socket, then its an incoming connection
		if ( FD_ISSET( serverSocket, &readFdSet ) ) {

			if ( ( newSocket = accept( serverSocket, ( struct sockaddr * ) &address, ( socklen_t * ) &addressLength ) ) < 0 ) {

				perror( SERVER_SOCKET_ACCEPT_ERROR );
				exit( EXIT_SERVER_SOCKET_ACCEPT_ERROR );
			}
			
			// Tell user socket number - used in send and receive requests
			printf( NEW_CLIENT_CONNECTION_INFO, newSocket, inet_ntoa( address.sin_addr ), ntohs( address.sin_port ) );
		
			// In telnet mode, send new connection greeting message
			if ( telnetMode ) {

				if ( send( newSocket, telnetWelcomeMessage, strlen( telnetWelcomeMessage ), 0 ) != strlen( telnetWelcomeMessage ) )
					perror( SEND_GREETING_ERROR );

				puts( SERVER_WELCOME_MSG_SEND );
			}
				
			// Add new socket to array of sockets
			for ( i = 0; i < MAX_CLIENTS; i++ ) {

				// If position is empty
				if( clientSockets[ i ] == 0 ) {

					clientSockets[ i ] = newSocket;
					printf( CLIENT_ADDED_TO_LIST_OF_SOCKETS_MSG, i );
					break;
				}
			}
		}
			
		// Process incoming IO operation on relevant socket
		for ( i = 0; i < MAX_CLIENTS; i++ ) {

			socketDescriptor = clientSockets[ i ];
				
			if ( FD_ISSET( socketDescriptor, &readFdSet ) ) {

				// Check if it was for closing, and also read the incoming message
				if ( ( bytesRead = read( socketDescriptor, dataBuffer, MAX_DATA_READ_LEN ) ) == 0 ) {

					// Somebody disconnected, get his details and print
					getpeername( socketDescriptor, ( struct sockaddr * ) &address, ( socklen_t * ) &addressLength );
					printf( CLIENT_DISCONNECTED_MSG , inet_ntoa( address.sin_addr ), ntohs( address.sin_port ) );
						
					// Close the socket and mark as 0 in list for reuse
					close( socketDescriptor );
					clientSockets[ i ] = 0;
				}
				else {

					if ( telnetMode ) {

						// TODO : put some code in separate source file (eg: telnet.c with all telnet only related functions)
						// to lighten source code

						// Process one input line at a time
						// TODO - FIX : A bit "cavalier" actually, bytesRead should also be passed along dataBuffer everywhere dataBuffer is used.....
						dataBuffer[ bytesRead ] = 0x0;

						// Process input
						if ( bytesRead == 2 && dataBuffer[ 0 ] == 13 && dataBuffer[ 1 ] == 10 ) {

							// No more processing (JSON) is required for GET and DELETE
							if ( requestInProgress[ i ] && ( request[ i ].verb == GET_VERB || request[ i ].verb == DELETE_VERB ) ) {

								request[ i ].ready = true;
							}

							// Got a CRLF, check if request is to be executed
							if ( requestInProgress[ i ] && request[ i ].ready ) {

								// Execute request
								_executeRequest( i, newSocket );
							}
							else if ( requestInProgress[ i ] ) {

								// Trigger POST PUT PATCH verbs second stage procesing (JSON needed)
								if ( request[ i ].verb == POST_VERB ||
									request[ i ].verb == PUT_VERB ||
									request[ i ].verb == PATCH_VERB ) {

									request[ i ].ready = true;
								}
							}
						}
						else {

							// Try to parse input
							if ( requestInProgress[ i ] == false ) {

								if ( _parseRequest( dataBuffer, i ) ) {

									// Got a verb followeb by path followed by "HTTP/1.1"
									requestInProgress[ i ] = true;								
								}
								else {

									// Unknown verb, close connexion
									close( socketDescriptor );
									clientSockets[ i ] = 0;
									requestInProgress[ i ] = false;
									_resetRequest( i );

									printf( CNX_ABORTED_ON_UNKNOWN_VERB_ERROR );
								}

							}
							else if ( request[ i ].ready ) {

								// Try to read one JSON line at a time, blank line triggers VERB execution
								if ( ! _appendJson( dataBuffer, i ) ) {

									// Json buffer overflow, close connection
									close( socketDescriptor );
									clientSockets[ i ] = 0;
									requestInProgress[ i ] = false;
									_resetRequest( i );

									printf( CNX_ABORTED_ON_GARBAGED_INPUT_ERROR );
								}
							}
							else {

								// Append optional headers, ignore return code
								_appendHeader( dataBuffer, i );
							}
						}
					}
					else {

						// Postman or curl made HTTP request processing
						// TODO : before processing the requet, read all bytes of incoming data on socket
						//        and not only MAX_DATA_READ_LEN
						_processFullRequest( dataBuffer, bytesRead, clientSockets, i, socketDescriptor );
					}
				}
			}
		}
	}
		
	return 0;
}

// Print program usage on standard output
static void _serverUsage() {

	printf(
		"Usage:\n\
server [-t|--telnet] [-d|--debug] [-h|--help]\n\
        Whith the -t or --telnet argument, start in telnet compliant mode.\n\
        Whith the -d or --debug argument, print request debug info while processing the request.\n\
        Whith the -h or --help argument, print usage and quit.\n\
		Without -t or --telnet argument, start in standard mode, use curl or postman to make requests.\n"
	);
}

// Signal handler, quits on SIGHUP (and SIGKILL of course...)
//
// signalNum is the signal number passed to the handler
static void _signalHandler( int signalNum ) {

	switch ( signalNum ) {

		case SIGHUP:
		case SIGKILL:
			puts( BYE_MSG );
			exit( 0 );
		default:
			;
	}
}

// Process full request in standard mode (not telnet)
//
// dataBuffer contains the whole request
// bytesRead is the length of the request
// clientSockets is a pointer to the array of client sockets
// client is the client id for which the request is processed
// socket is the client socket
//
// Returns the HTTP integer status code (200, 201, 400, ...) of the executed request
static int _processFullRequest( char * dataBuffer, const int bytesRead, int * clientSockets, const int client, const int socket ) {

	// In this mode, the whole request is delivered at once and not line by line as in telnet mode.
	// Over simplifcation: we assume that the whole request fits in dataBuffer.
	// TODO : bufferize input in dynamically allocated char array, and append
	// databuffer in this array until bytesRead is strictly less than MAX_DATA_LENGTH
	// and then process the input. Do some sanity checks to avoid oversized requests.

	// 1 - Get Header block and search for lines delimited by LINE_SEPARATOR
	//     First line should be a valid HTTP request (GET, POST, ...) followed by header params
	// 2 - Get boby block
	// 3 - Parse request
	// 4 - If valid request + body, parse header params and get JSON from body
	// 5 - Check consistency
	// 6 - Execute request

	int rc = HTTP_RC_500;
	char * separatorPtr = NULL;
	char * headerBlock = NULL;
	char * bodyBlock = NULL;
	bool hasBody = false;

	// Cavalier (isnt't it?)!
	dataBuffer[ bytesRead ] = 0x0;

	if ( debugRequest ) {

		printf( DEBUG_BYTES_READ, dataBuffer );
	}

	separatorPtr = strstr( dataBuffer, HEADER_BODY_SEPARATOR );

	if ( separatorPtr == NULL ) {

		// Invalid Request, Bye....
		close( socket );
		clientSockets[ client ] = 0;
		requestInProgress[ client ] = false;
		_resetRequest( client );
	}
	else {

		// Get header block and body block, if any

		if ( separatorPtr - dataBuffer == bytesRead - strlen( HEADER_BODY_SEPARATOR ) )
			hasBody = false;
		else
			hasBody = true;

		if ( hasBody )
			bodyBlock = separatorPtr + strlen( HEADER_BODY_SEPARATOR );

		// Parse header block
		*separatorPtr = 0x00;
		headerBlock = dataBuffer;

		int line = 1;
		bool rcBool = false;
		char * token = strtok( headerBlock, LINE_SEPARATOR);

		while ( token ) {

			if ( line == 1 ) {

				if ( ! _parseRequest( token, client ) ) {

					_sendErrorResponse( 
						socket,
						ERROR_TYPE_BAD_REQUEST_STR,
						ERROR_TITLE_BAD_REQUEST_STR,
						rc = HTTP_RC_400,
						HTTP_RC_400_STR,
						ERROR_DETAIL_BAD_REQUEST_STR,
						request[ client ].path == NULL ? PATH_UNDEFINED : request[ client ].path );
					_resetRequest( client );
					return rc;
				}
			}
			else
				_appendHeader( token, client );

			if ( line > MAX_HEADERS ) {

				_sendErrorResponse( 
					socket,
					ERROR_TYPE_BAD_REQUEST_STR,
					ERROR_TITLE_BAD_REQUEST_STR,
					rc = HTTP_RC_400,
					HTTP_RC_400_STR,
					ERROR_DETAIL_BAD_REQUEST_STR,
					request[ client ].path == NULL ? PATH_UNDEFINED : request[ client ].path );
				_resetRequest( client );
				return rc;
			}

			line++;
			// Warning : strtok is NOT thread safe.....
    		token = strtok( NULL, LINE_SEPARATOR );
		}

		if ( hasBody ) {

			if ( strlen( bodyBlock ) < MAX_JSON_LEN ) {

				strcpy( request[ client ].json, bodyBlock );
			}
			else {

				_sendErrorResponse( 
					socket,
					ERROR_TYPE_BAD_REQUEST_STR,
					ERROR_TITLE_BAD_REQUEST_STR,
					rc = HTTP_RC_400,
					HTTP_RC_400_STR,
					ERROR_DETAIL_BAD_REQUEST_STR,
					request[ client ].path == NULL ? EMPTY_STRING : request[ client ].path );
				_resetRequest( client );
				return rc;
			}
		}

		return _executeRequest( client, socket );
	}
}

// Execute client request in standard mode and telnet mode
//
// client is the client id
// socket is the client socket 
//
// Returns the relevant HTTP status code (200, 201, 400, ...)
static int _executeRequest( const int client, const int socket ) {

	// Be verbose on debug
	if ( debugRequest ) {

		switch ( request[ client ].verb ) {

			case GET_VERB:
			case DELETE_VERB:
			case PUT_VERB:
			case POST_VERB:
			case PATCH_VERB:
				printf( DEBUG_EXECUTING_REQUEST_MSG, _prettyRequestName( request[ client ].verb ) );
				break;
			
			default:
				printf( DEBUG_UNKNOWN_REQUEST_MSG );
				break;
		}

		if ( strlen( request[ client ].json ) > 0 ) {

			printf( DEBUG_WITH_JSON_MSG, request[ client ].json );
		}
	}

	int rc = HTTP_RC_500;
	int carId = 0;
	struct Car * car = NULL;
	struct Car * retrievedCar = NULL;
	int updaterCarRc = 0;
	int mask = 0;
	bool valid = false;

	// TODO : actuallaly ONLY car requests are processed, extend this to persons and rentals
	//        based on the path prefix (e.g.: /cars/... /persons/.... /rentals/...  )
	switch ( request[ client ].verb ) {

		case DELETE_VERB:
			_sendErrorResponse( 
				socket,
				ERROR_TYPE_NOT_ALLOWED_STR,
				ERROR_TITLE_NOT_ALLOWED_STR,
				rc = HTTP_RC_403,
				HTTP_RC_403_STR,
				ERROR_DETAIL_CAR_DELETE_NOT_ALLOWED_STR,
				request[ client ].path );
			break;

		case GET_VERB:
			carId = _parsePathForCarId( request[ client ].path );

			if ( carId == 0 ) {

				_sendErrorResponse( 
					socket,
					ERROR_TYPE_BAD_REQUEST_STR,
					ERROR_TITLE_BAD_REQUEST_STR,
					rc = HTTP_RC_400,
					HTTP_RC_400_STR,
					ERROR_DETAIL_BAD_REQUEST_STR,
					request[ client ].path );
				break;
			}
			else if ( carId > 0 )
				car = retrieveCar( carId );

			if ( car == NULL )
				_sendErrorResponse( 
					socket,
					ERROR_TYPE_NOT_FOUND_STR,
					ERROR_TITLE_NOT_FOUND_STR,
					rc = HTTP_RC_404,
					HTTP_RC_404_STR,
					ERROR_DETAIL_CAR_NOT_FOUND_STR,
					request[ client ].path );
			else {

				_sendCarResponse( socket, car, rc = HTTP_RC_200, HTTP_RC_200_STR );
				free( car );
			}
			break;

		case POST_VERB:
			car = parseJsonAsCar( request[ client ].json, &mask );
			valid = validateCar( CAR_VALIDATE_CREATE_OPERATION, mask, car, false );

			if ( mask & CAR_ID_FIELD )
				_sendErrorResponse( 
					socket,
					ERROR_TYPE_ID_ATTR_NOT_ALLOWED_STR,
					ERROR_TITLE_ID_ATTR_NOT_ALLOWED_STR,
					rc = HTTP_RC_400,
					HTTP_RC_400_STR,
					ERROR_DETAIL_CAR_ID_NOT_ALLOWED_STR,
					request[ client ].path );
			else if ( ! valid )
				_sendErrorResponse( 
					socket,
					ERROR_TYPE_BAD_REQUEST_STR,
					ERROR_TITLE_BAD_REQUEST_STR,
					rc = HTTP_RC_400,
					HTTP_RC_400_STR,
					ERROR_DETAIL_BAD_REQUEST_STR,
					request[ client ].path );
			else {

				carId = createCar( car );

				if ( carId > 0 ) {

					car->carId = carId;
					_sendCarResponse( socket, car, rc = HTTP_RC_201, HTTP_RC_201_STR );
				}
				else
					_sendErrorResponse( 
						socket,
						ERROR_TYPE_INTERNAL_ERROR_STR,
						ERROR_TITLE_INTERNAL_ERROR_STR,
						rc = HTTP_RC_500,
						HTTP_RC_500_STR,
						ERROR_DETAIL_INTERNAL_ERROR_STR,
						request[ client ].path );
			}

			free( car );
			break;

		case PUT_VERB:
		case PATCH_VERB:
			carId = _parsePathForCarId( request[ client ].path );

			if ( carId == 0 ) {

				_sendErrorResponse( 
					socket,
					ERROR_TYPE_BAD_REQUEST_STR,
					ERROR_TITLE_BAD_REQUEST_STR,
					rc = HTTP_RC_400,
					HTTP_RC_400_STR,
					ERROR_DETAIL_BAD_REQUEST_STR,
					request[ client ].path );
				break;
			}

			car = parseJsonAsCar( request[ client ].json, &mask );

			if ( carId != car->carId )
				_sendErrorResponse( 
					socket,
					ERROR_TYPE_BAD_REQUEST_STR,
					ERROR_TITLE_BAD_REQUEST_STR,
					rc = HTTP_RC_400,
					HTTP_RC_400_STR,
					ERROR_DETAIL_BAD_REQUEST_STR,
					request[ client ].path );
			else {

				valid = validateCar( CAR_VALIDATE_UPDATE_OPERATION, mask, car, request[ client ].verb == PATCH_VERB ? false : true );

				if ( valid ) {

					retrievedCar = retrieveCar( car->carId );

					if ( retrievedCar != NULL ) {

						carCopyToConditional( car, retrievedCar, mask );
						if ( updateCar( retrievedCar ) > 0 ) {

							// success
							_sendCarResponse( socket, retrievedCar, rc = HTTP_RC_200, HTTP_RC_200_STR );
						}
						else
							_sendErrorResponse( 
								socket,
								ERROR_TYPE_INTERNAL_ERROR_STR,
								ERROR_TITLE_INTERNAL_ERROR_STR,
								rc = HTTP_RC_500,
								HTTP_RC_500_STR,
								ERROR_DETAIL_INTERNAL_ERROR_STR,
								request[ client ].path );

						free( retrievedCar );
					}
					else
						_sendErrorResponse( 
							socket,
							ERROR_TYPE_NOT_FOUND_STR,
							ERROR_TITLE_NOT_FOUND_STR,
							rc = HTTP_RC_404,
							HTTP_RC_404_STR,
							ERROR_DETAIL_CAR_NOT_FOUND_STR,
							request[ client ].path );
				}
				else
					_sendErrorResponse( 
						socket,
						ERROR_TYPE_BAD_REQUEST_STR,
						ERROR_TITLE_BAD_REQUEST_STR,
						rc = HTTP_RC_400,
						HTTP_RC_400_STR,
						ERROR_DETAIL_BAD_REQUEST_STR,
						request[ client ].path );
			}

			free( car );
			break;

		default:
			break;
	}

	// Request executed, reset everything in order to be ready to process next request
	_resetRequest( client );

	return rc;
}

// Send car request response
//
// socket is the client socket
// car is a pointer to a car structure which should'nt be NULL
// code is HTTP integer return code
// codeMeaning is clear text HTTP return code
static void _sendCarResponse( const int socket, const struct Car *car, int code, char *codeMeaning ) {

	char * carAsJson;
	char header[ 512 ] = { 0 };
	char currentTime[ 64 ] = { 0 };
	int contentLength = 0;

	// TODO - FIX : fix all possible overflows....

	if ( car == NULL )
		return;

	carAsJson = carToJson( car );
	int carAsJsonLen = strlen( carAsJson );

	// append \r\n + 0x0 
	char * json = calloc( carAsJsonLen + 3, sizeof( char ) );
	strncpy( json, carAsJson, carAsJsonLen );
	json[ carAsJsonLen ] = '\r';
	json[ carAsJsonLen + 1 ] = '\n';
	json[ carAsJsonLen + 2 ] = 0x0;

	// body content length
	contentLength = strlen( json );

	// Get current time as http time string
	time_t now = time( 0 );
	struct tm tm = *gmtime( &now );
	strftime( currentTime, sizeof( currentTime ), HTTP_TIME_FORMAT_STR, &tm );

	// Construct header part
	sprintf( header, HTTP_HEADER_STR, code, codeMeaning, currentTime, contentLength );

	_sendJsonResponse( socket, header, json );

	free( json );
	free( carAsJson );
}

// Standard way to report an error to the client
// Check RFC 7807
//
// socket is client socket
// type is the error type
// title is the error title
// status is the HTTP integer error code
// statusStr is the HTTP error code description
// detail is the detailed error
// instance is the path that triggered the error
static void _sendErrorResponse( const int socket, const char *type, const char *title, const int status, const char *statusStr, const char *detail, const char *instance ) {

	char header[ 512 ] = { 0 };
	char json[ 1024 ] = { 0 };
	char currentTime[ 64 ] = { 0 };
	int contentLength = 0;

	// TODO - FIX : fix all possible overflows....

	// Compute json error as of RFC 7807
	sprintf( json, REQUEST_ERROR_RFC7807_JSON_STR, type, title, status, detail, instance );
	contentLength = strlen( json );

	// Get current time as http time string
	time_t now = time( 0 );
	struct tm tm = *gmtime( &now );
	strftime( currentTime, sizeof( currentTime ), HTTP_TIME_FORMAT_STR, &tm );

	// Construct header part
	sprintf( header, HTTP_HEADER_STR, status, statusStr, currentTime, contentLength );

	_sendJsonResponse( socket, header, json );
}

// Send JSON response to client
//
// socket is the client socket
// header is the header params returned to the client
// json is the json body of the response
static void _sendJsonResponse( const int socket, const char *header, const char *json ) {

	if ( header == NULL || json == NULL ) {

		return;
	}

	// TODO - check header and json sizes (if not NULL) to stay in acceptable ranges...
	int headerLen = strlen( header );
	int jsonLen = strlen( json );
	char * response = calloc( headerLen + jsonLen + 3, sizeof( char ) );
	memset( response, 0, headerLen + jsonLen + 3 );

	// Construct response
	sprintf( response, "%s\r\n%s", header, json );

	// Send response
	if( send( socket, response, strlen( response ), 0 ) != strlen( response ) )
		perror( "send response error" );

	// Cleanup
	free( response );
}

// Retrive car id from path
//
// path is the path of the request
//
// Returns 0 if unable to parse path which should in turn trigger a BAD_REQUEST error
static int _parsePathForCarId( const char * path ) {

	int carId = -1;
	int rc = 0;

	// path should be in form /cars/:id where :id is the id of the car to retrieve  e.g.: /cars/2
	int count = sscanf( path, API_PATH_GET_CAR, &carId );

	if ( count != 1 )
		return rc;
	else if ( carId <= 0 )
		return rc;
	else
		rc = carId;

	return rc;
}

// Parse client request
//
// buffer contains the request
// client is the clien id
//
// Return true if all went well, else return false
static bool _parseRequest( const char *buffer, const int client ) {

	bool rc = false;

	char verb[ MAX_VERB_LEN ] = { 0 };
	char path[ MAX_URI_PATH_LEN ] = { 0 };
	char protocol[ MAX_PROTOCOL_LEN ] = { 0 };
	
	int count = sscanf( buffer, "%15s %127s %15s", verb, path, protocol );

	if ( debugRequest ) {

		printf( DEBUG_VERB_MSG, verb );
		printf( DEBUG_PATH_MSG, path );
		printf( DEBUG_PROTOCOL_MSG , protocol );
	}

	if ( strcmp( protocol, HTTP_1_DOT_1 ) != 0 ) {

		rc = false;
		return rc;
	}

	if ( strcmp( verb, GET_VERB_STR ) == 0 ) {

		_populateRequest( GET_VERB, path, client );		
		rc = true;
	}
	else if ( strcmp( verb, DELETE_VERB_STR ) == 0 ) {

		_populateRequest( DELETE_VERB, path, client );		
		rc = true;
	}
	else if ( strcmp( verb, POST_VERB_STR ) == 0 ) {

		_populateRequest( POST_VERB, path, client );		
		rc = true;
	}
	else if ( strcmp( verb, PUT_VERB_STR ) == 0 ) {

		_populateRequest( PUT_VERB, path, client );		
		rc = true;
	}
	else if ( strcmp( verb, PATCH_VERB_STR ) == 0 ) {

		_populateRequest( PATCH_VERB, path, client );		
		rc = true;
	}

	return rc;
}

// Append header param for this client request, used only in telnet mode
//
// buffer contains a single header line
// client is the clien id
//
// Return true if all went well, else return false
static bool _appendHeader( char *buffer, const int client ) {

	bool rc = false;

	char name[ MAX_HEADER_NAME_LEN ] = { 0 };
	char value[ MAX_HEADER_VALUE_LEN ] = { 0 };

	// Strip \r\n from end of buffer
	if ( buffer[ strlen( buffer ) - 2 ] == '\r' && buffer[ strlen( buffer ) - 1 ] == '\n' )
		buffer[ strlen( buffer) - 2 ] = 0x0;

	int bufferLen = strlen( buffer );

	if ( bufferLen < 4 )
		return rc;

	// Search for first ": " string as name / value separator
	char * ptr = strstr( buffer, HEADER_NAME_VALUE_SEPARATOR );

	if ( ptr != NULL ) {

		// Found !
		strncpy( value, ptr + 2, MAX_HEADER_VALUE_LEN - 1 );
		strncpy( name, buffer, ( ptr - buffer ) >= MAX_HEADER_NAME_LEN ? MAX_HEADER_NAME_LEN - 1 : ( ptr - buffer ) );
	}
	else
		return rc;

	// Check if headers array is already full
	if ( request[ client ].headersCount < MAX_HEADERS  ) {

		if ( debugRequest ) {

			printf( DEBUG_HEADER_MSG, name, value );
		}

		strncpy( request[ client ].headers[ request[ client ].headersCount ].name, name, MAX_HEADER_NAME_LEN );
		strncpy( request[ client ].headers[ request[ client ].headersCount ].value, value, MAX_HEADER_VALUE_LEN );

		request[ client ].headersCount++;

		rc = true;
	}

	return rc;
}

// Populate client request with info retrieved from socket
//
// verb is the HTTP request type (GET, PUT, ...)
// path is the client request path
// client is the client id
static void _populateRequest( const int verb, const char *path, const int client ) {
	
	request[ client ].verb = verb;
	strncpy( request[ client ].path, path, MAX_URI_PATH_LEN );
	request[ client ].path[ MAX_URI_PATH_LEN - 1 ] = 0;
}

// Append json input line to the client json body
//
// buffer contains the json line
// client is the client id
//
// Return true if all went well, else return false
static bool _appendJson( const char *buffer, const int client ) {

	bool rc = false;

	if ( strlen( buffer ) <= MAX_JSON_LEN - strlen( request[ client ].json ) - 1 ) {

		strncat( request[ client ].json, buffer, strlen( buffer ) );
		rc = true;
	}

	return rc;
}

// Reset client request, either on success or error while processing the request
//
// client is the client id
static void _resetRequest( const int client ) {

	requestInProgress[ client ] = false;
	request[ client ].ready = false;
	request[ client ].verb = UNDEFINED_VERB;
	memset( request[ client ].json, 0, MAX_JSON_LEN );
	memset( request[ client ].path, 0, MAX_URI_PATH_LEN );

	for ( int i = 0; i < MAX_HEADERS; i++ ) {

		request[ client ].headersCount = 0;
		memset( request[ client ].headers[ i ].name, 0, MAX_HEADER_NAME_LEN );
		memset( request[ client ].headers[ i ].value, 0, MAX_HEADER_VALUE_LEN );
	}
}

// String representation of the request verb
//
// verb is the int value or the request verb
//
// Returns the string representation of the request verb
static char * _prettyRequestName( const int verb ) {

	switch ( verb ) {

		case GET_VERB:
			return GET_VERB_STR;
		case DELETE_VERB:
			return DELETE_VERB_STR;
		case PUT_VERB:
			return PUT_VERB_STR;
		case POST_VERB:
			return POST_VERB_STR;
		case PATCH_VERB:
			return PATCH_VERB_STR;

		default:
			return UNDEFINED_VERB;
	}
}
