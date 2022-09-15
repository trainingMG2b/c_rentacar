#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#include "rentacar.h"
#include "car.h"
#include "data.h"
#include "cJSON.h"


// TODOs (as exercices):
//    Factorize the json parsing (brand, model, ..., carId) before the operation call in a private utility function
//    Add comments above each function, using the WAR model : What the function does, Arguments description, Return value
//    Add some commnents at relevant places in the code
//    Factorize the sprintf message calls with defines and variable arguments

// Return a Car (object instance) constructed from json input text and while parsing sets mask to indicate
// which fields where present in the json input
//
// jsonText contains the json text to be parsed
// mask bits are filled while parsing the json to indicate which field was present in the json
//
// Return a pointer to a new Car structure, which should be freed later to avoid memory leak
struct Car * parseJsonAsCar( const char * jsonText, int * mask ) {

    const cJSON *brand = NULL;
    const cJSON *model = NULL;
    const cJSON *year = NULL;
    const cJSON *active = NULL;
    const cJSON *numberPlate = NULL;
    const cJSON *carId = NULL;

    *mask = 0;
    cJSON *carJson = cJSON_Parse( jsonText );

    if ( carJson == NULL ) {

        cJSON_Delete( carJson );
        return NULL;
    }

    struct Car * car = calloc( 1, sizeof( struct Car ) );

    car->carId = 0;

    brand = cJSON_GetObjectItemCaseSensitive( carJson, "brand" );

    if ( brand != NULL && cJSON_IsString( brand ) && ( brand->valuestring != NULL ) ) {

        strncpy( car->brand, brand->valuestring, BRAND_LEN );
        car->brand[ BRAND_LEN - 1 ] = 0x0;
        *mask |= CAR_BRAND_FIELD;
    }

    model = cJSON_GetObjectItemCaseSensitive( carJson, "model" );

    if ( model != NULL && cJSON_IsString( model ) && ( model->valuestring != NULL ) ) {

        strncpy( car->model, model->valuestring, MODEL_LEN );
        car->model[ MODEL_LEN - 1 ] = 0x0;
        *mask |= CAR_MODEL_FIELD;
    }

    year = cJSON_GetObjectItemCaseSensitive( carJson, "year" );

    if ( year != NULL && cJSON_IsNumber( year ) ) {

        car->year = year->valueint;
        *mask |= CAR_YEAR_FIELD;
    }

    numberPlate = cJSON_GetObjectItemCaseSensitive( carJson, "numberPlate" );

    if ( numberPlate != NULL && cJSON_IsString( numberPlate ) && ( numberPlate->valuestring != NULL ) ) {

        strncpy( car->numberPlate, numberPlate->valuestring, NUMBER_PLATE_LEN );
        car->numberPlate[ NUMBER_PLATE_LEN - 1 ] = 0x0;
        *mask |= CAR_NUMBER_PLATE_FIELD;
    }

    active = cJSON_GetObjectItemCaseSensitive( carJson, "active" );

    if ( active != NULL && cJSON_IsBool( active ) ) {

        car->active = active->valueint;
        *mask |= CAR_ACTIVE_FIELD;
    }

    carId = cJSON_GetObjectItemCaseSensitive( carJson, "carId" );

    if ( carId != NULL && cJSON_IsNumber( carId ) ) {

        car->carId = carId->valueint;
        *mask |= CAR_ID_FIELD;
    }

    cJSON_Delete( carJson );

    return car;
}

// Do a car opration (crud) based on the given json string
//
// operation is the one of crud
// jsonText contains the car data to be processed
//
// Returns the carId (>0) or 0 on error
int carOperation( char operation, const char * jsonText ) {

    int rc = 0;
    int mask = 0;
    int id = 0;
    struct Car * retrievedCar;
    char * carAsJson;
    char * carDesc;

    char * msg = calloc( MAX_MESSAGE_LEN, sizeof( char ) );

    struct Car * car = parseJsonAsCar( jsonText, &mask );

    // Do operation
    switch ( operation ) {

        case CREATE_OPER:
            if ( ! ( mask & CAR_BRAND_FIELD ) ||
                ! ( mask & CAR_MODEL_FIELD ) ||
                ! ( mask & CAR_YEAR_FIELD ) ||
                ! ( mask & CAR_ACTIVE_FIELD ) ||
                ! ( mask & CAR_NUMBER_PLATE_FIELD ) ) {

                sprintf( msg, "{ \"message\": \"Car definition incomplete in input json\", \"status\": %d }", SC_BAD_REQUEST );
                messageErr( msg );
                break;
            }

            id = createCar( car );

            if ( id <= 0 ) {

                sprintf( msg, "{ \"message\": \"Unable to create a new car\", \"car\": %s, \"status\": %d }", jsonText, SC_INTERNAL_ERROR );
                messageErr( msg );
            }
            else {

                rc = id;
                sprintf( msg, "{ \"message\": \"New car created\", \"carId\": %d, \"status\": %d }", rc, SC_OK );
                messageOut( msg );
            }
            break;

        case RETRIEVE_OPER:
            if ( ! mask & CAR_ID_FIELD ) {

                sprintf( msg, "{ \"message\": \"Car id not found in input json\", \"status\": %d }", SC_BAD_REQUEST );
                messageErr( msg );
            }
            else {

                retrievedCar = retrieveCar( car->carId );
                if ( retrievedCar != NULL ) {

                    rc = retrievedCar->carId;
                    carAsJson = carToJson( retrievedCar );
                    sprintf( msg, "{ \"car\": %s, \"status\": %d }", carAsJson, SC_OK );
                    messageOut( msg );
                    free( carAsJson );
                    free( retrievedCar );
                }
                else {

                    sprintf( msg, "{ \"message\": \"Car with carId = %d not found\", \"status\": %d }", car->carId, SC_NOT_FOUND );
                    messageErr( msg );
                }
            }
            break;

        case UPDATE_OPER:
            if ( ! mask & CAR_ID_FIELD ) {

                sprintf( msg, "{ \"message\": \"Car id not found in input json\", \"status\": %d }", SC_BAD_REQUEST );
                messageErr( msg );
            }
            else {
                
                retrievedCar = retrieveCar( car->carId );
                if ( retrievedCar != NULL ) {

                    carCopyToConditional( car, retrievedCar, mask );
                    rc = updateCar( retrievedCar );
                    if ( rc ) {
                        
                        sprintf( msg, "{ \"carId\": %d, \"status\": %d }", rc, SC_OK );
                        messageOut( msg );
                    }
                    else {

                        sprintf( msg, "{ \"message\": \"Unable to update car\": %s, \"status\": %d }", jsonText, SC_INTERNAL_ERROR );
                        messageErr( msg );
                    }
                }
                else {

                    sprintf( msg, "{ \"message\": \"Car with carId = %d not found\", \"status\": %d }", car->carId, SC_NOT_FOUND );
                    messageErr( msg );
                }
            }
            break;

        case DELETE_OPER:
            if ( ! mask & CAR_ID_FIELD ) {

                sprintf( msg, "{ \"message\": \"Car id not found in input json\", \"status\": %d }", SC_BAD_REQUEST );
                messageErr( msg );
            }
            else {

                id = deleteCar( car->carId );
                rc = id;
            }
            break;

        default:
            sprintf( msg, "{ \"message\": \"Operation unknown: %c\", \"status\": %d }", operation, SC_BAD_REQUEST );
            messageErr( msg );
    }

    free( msg );
    if ( car != NULL )
        free( car );

    return rc;
}

// Convert a Car to a human readable string
//
// car is the Car to convert
//
// Returns the string representation or the string "NULL" if car is null
char * carToString( const struct Car * car ) {

    char * carJson = calloc( MAX_MESSAGE_LEN, sizeof( char ) );

    if ( car != NULL ) {

        sprintf( carJson, "brand: %s, model: %s, year: %d, numberPlate: %s, active: %s, carId: %d", car->brand, car->model, car->year, car->numberPlate, car->active ? "true" : "false", car->carId );
        return carJson;
    }
    else
        return "NULL";
}

// Convert a Car to a json string
//
// car is the Car to convert
//
// Returns the json representation or the string "{ "error": 'NULL"}" if car is null
char * carToJson( const struct Car * car ) {

    char * carJson = calloc( MAX_MESSAGE_LEN, sizeof( char ) );

    if ( car != NULL ) {

        sprintf( carJson, "{ \"brand\": \"%s\", \"model\": \"%s\", \"year\": %d, \"numberPlate\": \"%s\", \"active\": %s, \"carId\": %d }", car->brand, car->model, car->year, car->numberPlate, car->active ? "true" : "false", car->carId );
        return carJson;
    }
    else
        return "{ \"error\": \"NULL\" }";
}

// Copy a Car to another one. Source and destination must not be null
//
// source is the source Car
// dest is the destination Car
void carCopyTo( const struct Car * source, struct Car * dest ) {

    if ( source != NULL && dest != NULL ) {

        strcpy( dest->brand, source->brand );
        strcpy( dest->model, source->model );
        strcpy( dest->numberPlate, source->numberPlate );
        dest->year = source->year;
        dest->active = source->active;
        dest->carId = source->carId;
    }
}

// Copy some fields from a source Car to a destination Car. Source and destination must not be null
//
// source is the source Car
// dest is the destination Car
// mask is an int containing a mask of the fields to be copied (see car.h)
void carCopyToConditional( const struct Car * source, struct Car * dest, int mask ) {

    if ( source != NULL && dest != NULL ) {

        if ( mask & CAR_BRAND_FIELD ) strcpy( dest->brand, source->brand );
        if ( mask & CAR_MODEL_FIELD ) strcpy( dest->model, source->model );
        if ( mask & CAR_NUMBER_PLATE_FIELD ) strcpy( dest->numberPlate, source->numberPlate );
        if ( mask & CAR_YEAR_FIELD ) dest->year = source->year;
        if ( mask & CAR_ACTIVE_FIELD ) dest->active = source->active;
        if ( mask & CAR_ID_FIELD ) dest->carId = source->carId;
    }
}

// Validate a Car attributes
// This is very >>>BASIC<<< car attributes checking. In real world scenarios, brand should 
// be checked against a list of known brands, model from brand known models, number plate
// shoud be checked against a regular expression (man 3 regcomp/regexec). 
//
// operation is either create or update
// mask indicates which fields have been initialized (usually with a value from JSON)
// car is the car to be validated
//
// Return true if the validation is OK, else return false
bool validateCar( int operation, int mask, const struct Car * car, bool allFieldsMandatory ) {

    switch ( operation ) {

        case CAR_VALIDATE_CREATE_OPERATION:
        case CAR_VALIDATE_UPDATE_OPERATION:
            break;

        default:
            return false;
    }

    if ( car == NULL )
        return false;

    if ( allFieldsMandatory && mask != ALL_FIELDS )
        return false;

	time_t now = time( 0 );
	struct tm tm = *gmtime( &now );
    int currentYear = tm.tm_year + 1900;

    switch ( operation ) {
        
        case CAR_VALIDATE_CREATE_OPERATION:
            if ( ( mask & CAR_ID_FIELD ) != 0 )
                return false;

            if ( ( mask & CAR_ACTIVE_FIELD ) == 0 )
                return false;

            if ( ( mask & CAR_BRAND_FIELD ) == 0 ||
                car->brand == NULL ||
                car->brand[ BRAND_LEN - 1 ] != 0x0 ||
                strlen( car->brand ) == 0 )
                return false;
                
            if ( ( mask & CAR_MODEL_FIELD ) == 0 ||
                car->model == NULL ||
                car->model[ MODEL_LEN - 1 ] != 0x0 ||
                strlen( car->model ) == 0 )
                return false;

            if ( ( mask & CAR_YEAR_FIELD ) == 0 ||
                car->year < MIN_CAR_YEAR ||
                car->year > currentYear + 1 )
                return false;

            if ( ( mask & CAR_NUMBER_PLATE_FIELD ) == 0 ||
                car->numberPlate == NULL ||
                car->numberPlate[ NUMBER_PLATE_LEN - 1 ] != 0x0 ||
                strlen( car->numberPlate ) == 0 )
                return false;

            break;

        case CAR_VALIDATE_UPDATE_OPERATION:
            if ( ( mask & CAR_ID_FIELD ) == 0 )    
                return false;

            if ( ( mask & CAR_BRAND_FIELD ) != 0 && (
                car->brand == NULL ||
                car->brand[ BRAND_LEN - 1 ] != 0x0 ||
                strlen( car->brand ) == 0 ) )
                return false;
                
            if ( ( mask & CAR_MODEL_FIELD ) != 0 && (
                car->model == NULL ||
                car->model[ MODEL_LEN - 1 ] != 0x0 ||
                strlen( car->model ) == 0 ) )
                return false;

            if ( ( mask & CAR_YEAR_FIELD ) != 0 && (
                car->year < MIN_CAR_YEAR ||
                car->year > currentYear + 1 ) )
                return false;

            if ( ( mask & CAR_NUMBER_PLATE_FIELD ) != 0 && (
                car->numberPlate == NULL ||
                car->numberPlate[ NUMBER_PLATE_LEN - 1 ] != 0x0 ||
                strlen( car->numberPlate ) == 0 ) )
                return false;

            break;
    
        default:
            return false;
            break;
    }
    
    return true;
}