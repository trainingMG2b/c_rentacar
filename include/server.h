#ifndef _SERVER_H
#define _SERVER_H

#define EXIT_WRONG_ARGS					-100
#define EXIT_SERVER_SOCKET_ERROR		-103
#define EXIT_SERVER_SOCKET_CONFIG_ERROR	-104
#define EXIT_SERVER_SOCKET_BIND_ERROR	-105
#define EXIT_SERVER_SOCKET_LISTEN_ERROR	-106
#define EXIT_SERVER_SOCKET_ACCEPT_ERROR	-106

#define TCP_IP_LISTEN_PORT			13722
#define MAX_PENDING_CONNECTIONS		3
#define MAX_CLIENTS					30
#define MAX_DATA_LENGTH				1024

#define OPEN_SERVER_SOCKET_ERROR            "Error opening server socket"
#define SERVER_SOCKET_CONFIG_ERROR			"Error seting server socket confixog"
#define SERVER_SOCKET_BIND_ERROR			"Error binding server socket to port"
#define SERVER_SOCKET_LISTEN_ERROR			"Error while listening on server socket port"
#define SOCKET_SELECT_ERROR					"Error on socket select errorr"
#define SERVER_SOCKET_ACCEPT_ERROR			"Error on socket accept"

#define CNX_ABORTED_ON_GARBAGED_INPUT_ERROR "Garbaged input error, connexion aborted"
#define CNX_ABORTED_ON_UNKNOWN_VERB_ERROR   "Unknown verb error, connexion aborted"

#define SERVER_UP_MESSAGE					"Rentacar Server v0.1 \r\n"
#define LISTENING_ON_PORT_MSG				"Listener on port %d \n"
#define SERVER_WAITING_MSG					"Waiting for connections ..." 
#define SERVER_WELCOME_MSG_SEND				"Welcome message sent successfully"
#define CLIENT_ADDED_TO_LIST_OF_SOCKETS_MSG	"Adding to list of sockets as %d\n"
#define CLIENT_DISCONNECTED_MSG				"Host disconnected, ip %s, port %d \n"
#define NEW_CLIENT_CONNECTION_INFO          "New connection, socket fd is %d, ip is : %s, port : %d\n"
#define SEND_GREETING_ERROR                 "send greeting message error"
#define BYE_MSG								"Bye..."

#define HTTP_1_DOT_0                "HTTP/1.0"
#define HTTP_1_DOT_1                "HTTP/1.1"

#define EMPTY_STRING	""

#define UNDEFINED_VERB	0
#define GET_VERB		1
#define POST_VERB		2
#define PUT_VERB		3
#define PATCH_VERB		4
#define DELETE_VERB		5

#define MAX_VERB_LEN			16
#define MAX_URI_PATH_LEN		128
#define MAX_PROTOCOL_LEN		16
#define MAX_JSON_LEN			2048
#define MAX_DATA_READ_LEN		2048
#define MAX_HEADERS				16
#define MAX_HEADER_NAME_LEN		64
#define MAX_HEADER_VALUE_LEN	128

#define DEBUG_VERB_MSG                      "verb = %s\n"
#define DEBUG_PATH_MSG                      "path = %s\n"
#define DEBUG_PROTOCOL_MSG                  "protocol = %s\n"
#define DEBUG_HEADER_MSG                    "got header : name / value = %s / %s\n"
#define DEBUG_BYTES_READ					"Bytes read = %s\n"
#define DEBUG_EXECUTING_REQUEST_MSG         "executing request %s\n"
#define DEBUG_WITH_JSON_MSG                 "with json = %s\n"
#define DEBUG_UNKNOWN_REQUEST_MSG			"Unknown request encountered.\n"

#define HEADER_NAME_VALUE_SEPARATOR         ": "
#define HEADER_BODY_SEPARATOR				"\r\n\r\n"
#define LINE_SEPARATOR						"\r\n"

#define GET_VERB_STR        "GET"
#define DELETE_VERB_STR     "DELETE"
#define POST_VERB_STR       "POST"
#define PUT_VERB_STR        "PUT"
#define PATCH_VERB_STR      "PATCH"

#define PATH_UNDEFINED		"PATH_UNDEFINED"

#define HTTP_RC_200 	200
#define HTTP_RC_201 	201
#define HTTP_RC_400 	400
#define HTTP_RC_401 	401
#define HTTP_RC_403 	403
#define HTTP_RC_404 	404
#define HTTP_RC_405 	405
#define HTTP_RC_406 	406
#define HTTP_RC_500 	500
#define HTTP_RC_503 	503

#define HTTP_RC_200_STR 	"OK"
#define HTTP_RC_201_STR 	"Created"
#define HTTP_RC_400_STR 	"Bad Request"
#define HTTP_RC_401_STR 	"Unauthorized"
#define HTTP_RC_403_STR 	"Forbidden"
#define HTTP_RC_404_STR 	"Not Found"
#define HTTP_RC_405_STR 	"Method Not Allowed"
#define HTTP_RC_406_STR 	"Not Acceptable"
#define HTTP_RC_500_STR 	"Internal Server Error"
#define HTTP_RC_503_STR 	"Service Unavailable"

#define HTTP_TIME_FORMAT_STR "%a, %d %b %Y %H:%M:%S %Z"

#define API_PATH_GET_CAR    "/cars/%d"

#define ERROR_TYPE_NOT_ALLOWED_STR                  "/errors/not-allowed"
#define ERROR_TYPE_NOT_FOUND_STR                    "/errors/not-found"
#define ERROR_TYPE_ID_ATTR_NOT_ALLOWED_STR          "/errors/id-attr-not-allowed"
#define ERROR_TYPE_INTERNAL_ERROR_STR               "/errors/internal-error"
#define ERROR_TYPE_BAD_REQUEST_STR                  "/errors/bad-request"

#define ERROR_TITLE_NOT_ALLOWED_STR                 "Operation not allowed."
#define ERROR_TITLE_NOT_FOUND_STR                   "Not found."
#define ERROR_TITLE_ID_ATTR_NOT_ALLOWED_STR         "Id attribute not allowed in this operation."
#define ERROR_TITLE_INTERNAL_ERROR_STR              "Internal error."
#define ERROR_TITLE_BAD_REQUEST_STR                 "Bad request."

#define ERROR_DETAIL_CAR_DELETE_NOT_ALLOWED_STR     "It's not allowed to delete a car."
#define ERROR_DETAIL_CAR_NOT_FOUND_STR              "Requested car not found."
#define ERROR_DETAIL_CAR_ID_NOT_ALLOWED_STR         "Car Id attribute not allowed in this operation."
#define ERROR_DETAIL_INTERNAL_ERROR_STR             "Internal server error."
#define ERROR_DETAIL_BAD_REQUEST_STR                "Bad request error."

#define REQUEST_ERROR_RFC7807_JSON_STR "{ \"type\": \"%s\", \"title\": \"%s\", \"status\": %d, \"detail\": \"%s\", \"instance\": \"%s\" }\r\n"

#define HTTP_HEADER_STR "HTTP/1.0 %d %s\r\nDate: %s\r\nServer: Rentacar Server v0.1\r\nContent-Length: %d\r\nContent-Type: application/json; charset=utf-8\r\n"


struct Header {

	char name[ MAX_HEADER_NAME_LEN ];
	char value[ MAX_HEADER_VALUE_LEN ];
};

// TODO : Request headers should be allocated dynamically and not hard limited
struct Request {

	int verb;
	char path[ MAX_URI_PATH_LEN ];
	char json[ MAX_JSON_LEN ];
	bool ready;
	struct Header headers[ MAX_HEADERS ];
	int headersCount;
};

#endif
