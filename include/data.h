#ifndef _DATA_H
#define _DATA_H

#include "car.h"

#define CREATE_OPER       'c'
#define RETRIEVE_OPER     'r'
#define UPDATE_OPER       'u'
#define DELETE_OPER       'd'

#define PATH_SEP          "/"
// #define PATH_SEP          "\\"
#define MAX_PATH_LEN      256

// Status codes
#define SC_OK             200
#define SC_BAD_REQUEST    400
#define SC_UNAUTHORIZED   401
#define SC_FORBIDDEN      403
#define SC_NOT_FOUND      404
#define SC_INTERNAL_ERROR 500

// #define DATA_HOME_DIR     "/home/guest1/training/workspaces/c/structs/data"
// #define DATA_HOME_DIR     "C:\\Users\\guest1\\training\\workspaces\\c\\structs\\data"
#define DATA_HOME_DIR     "./data"
#define ID_FILE           "id.db"
#define CARS_FILE         "car.db"


int createCar( struct Car * );
struct Car * retrieveCar( const int );
int updateCar( const struct Car * );
int deleteCar( int );

int _getNextId();

#endif