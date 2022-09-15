#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "rentacar.h"
#include "data.h"
#include "car.h"

static int _getNextId();

// TODOs (as exercices):
//    factorize the strncat blocs in a private utility finction
//    implement more business control checks. e.g.: on create a new car, the numberPlate must not already exist

// Create a new Car
//
// car is the Car to create, must not be null.
//
// Return the newly created carId
int createCar( struct Car *car ) {

    int rc = 0;

    if ( car == NULL )
        return rc;

    int carId = _getNextId();

    car->carId = carId;

    char fileName[ MAX_PATH_LEN ] = { 0 };
    FILE *file;

    // BEGIN - Factorize
    strncat( fileName, DATA_HOME_DIR, MAX_PATH_LEN - 1 );
    strncat( fileName, PATH_SEP, MAX_PATH_LEN - sizeof( DATA_HOME_DIR ) - 1 );
    strncat( fileName, CARS_FILE, MAX_PATH_LEN - sizeof( DATA_HOME_DIR ) - sizeof( PATH_SEP ) - 1 );
    // END - Factorize

    file = fopen( fileName, "a+" );
    fwrite( car, sizeof( struct Car ), 1, file );
    fflush( file );
    fclose( file );

    rc = carId;
    return rc;
}

// Retrive a car
//
// carId is the Car id to retrieve
//
// Returns the retrieved car or NULL if not found
struct Car * retrieveCar( const int carId ) {

    char fileName[ MAX_PATH_LEN ] = { 0 };
    FILE *file;
    struct Car car;
    struct Car *rc = NULL;

    strncat( fileName, DATA_HOME_DIR, MAX_PATH_LEN - 1 );
    strncat( fileName, PATH_SEP, MAX_PATH_LEN - sizeof( DATA_HOME_DIR ) - 1 );
    strncat( fileName, CARS_FILE, MAX_PATH_LEN - sizeof( DATA_HOME_DIR ) - sizeof( PATH_SEP ) - 1 );

    file = fopen( fileName, "r" );

    while ( fread( &car, sizeof( struct Car ), 1, file ) ) {

        if ( car.carId == carId ) {

            rc = calloc( 1, sizeof( struct Car ) );
            carCopyTo( &car, rc );
            break;
        }
    }

    fclose( file );

    return rc;
}

// Update a car
//
// car is the Car to update, must not be null
//
// Returns the updated carId or zero if not found
int updateCar( const struct Car *car ) {

    char fileName[ MAX_PATH_LEN ] = { 0 };
    FILE *file;
    struct Car carBuffer;
    int count = 0;
    int rc = 0;

    if ( car == NULL )
        return rc;

    strncat( fileName, DATA_HOME_DIR, MAX_PATH_LEN - 1 );
    strncat( fileName, PATH_SEP, MAX_PATH_LEN - sizeof( DATA_HOME_DIR ) - 1 );
    strncat( fileName, CARS_FILE, MAX_PATH_LEN - sizeof( DATA_HOME_DIR ) - sizeof( PATH_SEP ) - 1 );

    file = fopen( fileName, "r+" );

    if ( file == NULL || car->carId == 0 )
        return rc;

    while ( fread( &carBuffer, sizeof( struct Car ), 1, file ) ) {

        if ( carBuffer.carId == car->carId ) {

            fseek( file, count * sizeof( struct Car ), SEEK_SET );
            fwrite( car, sizeof( struct Car ), 1, file );
            fflush( file );
            rc = car->carId;
            break;
        }

        count++;
    }

    fclose( file );

    return rc;
}

// Delete a car (WARNING: In the specs, a car cannot be deleted, see comments in the file containing the main function...)
//
// carId is the id of the Car to delete
//
// returns the deleted carId or zero if not found
int deleteCar( int carId ) {

    struct Car *retrievedCar;
    int rc = 0;
    int index = 0;

    retrievedCar = retrieveCar( carId );
    char * msg = calloc( MAX_MESSAGE_LEN, sizeof( char ) );

    if ( retrievedCar != NULL ) {

        rc = retrievedCar->carId;
        sprintf( msg, "{ \"message\": \"Not allowed to delete car with carId = %d\", \"status\": %d }", carId, SC_FORBIDDEN );
        messageErr( msg );
        free( msg );
        free( retrievedCar );
    }
    else {

        sprintf( msg, "{ \"message\": \"Car with carId = %d not found\", \"status\": %d }", carId, SC_NOT_FOUND );
        messageErr( msg );
        free( msg );        
    }

    return rc;
}

// Get the next available id
//
// Returns the next id
static int _getNextId() {

    int rc = 0;
    char fileName[ MAX_PATH_LEN ] = { 0 };
    int fd;

    strncat( fileName, DATA_HOME_DIR, MAX_PATH_LEN - 1 );
    strncat( fileName, PATH_SEP, MAX_PATH_LEN - sizeof( DATA_HOME_DIR ) - 1 );
    strncat( fileName, ID_FILE, MAX_PATH_LEN - sizeof( DATA_HOME_DIR ) - sizeof( PATH_SEP ) - 1 );

    fd = open( fileName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );
    lseek( fd, 0, SEEK_SET );
 
    int val;

    if ( read( fd, &val, sizeof( int ) ) == 0 ) {

        // first write to file (val = 1)
        val = 1;
        rc = val;
    }
    else {

        val++;
        rc = val;
    }

    lseek( fd, 0, SEEK_SET );
    write( fd, &val, sizeof( int ) );
    close( fd );

    return rc;
}
