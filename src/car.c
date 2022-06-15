#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "rentacar.h"
#include "car.h"
#include "data.h"
#include "cJSON.h"

//
// TODOs (as exercices):
//    Factorize the json parsing (brand, model, ..., carId) before the operation call in a private utility finction
//    Add comments above each function, using the WAR model : What the function does, Arguments description, Return value
//    Add some commnents at relevant places in the code
//    Factorize the sprintf message calls with defines and variable arguments
//


int carOperation( char operation, const char * jsonText ) {

    int rc = 0;
    char * msg = calloc( MAX_MESSAGE_LEN, sizeof( char ) );
    
    const cJSON *brand = NULL;
    const cJSON *model = NULL;
    const cJSON *year = NULL;
    const cJSON *active = NULL;
    const cJSON *numberPlate = NULL;
    const cJSON *carId = NULL;

    // BEGIN - Factorize
    int mask = 0;
    cJSON *carJson = cJSON_Parse( jsonText );

    if ( carJson == NULL ) {

        cJSON_Delete( carJson );

        sprintf( msg, "{ \"message\": \"Unable to parse json text\", \"json\": %s, \"status\": %d }", jsonText, SC_INTERNAL_ERROR );
        messageErr( msg );
        free( msg );

        return rc;
    }

    struct Car car;
    car.carId = 0;

    brand = cJSON_GetObjectItemCaseSensitive( carJson, "brand" );

    if ( brand != NULL && cJSON_IsString( brand ) && ( brand->valuestring != NULL ) ) {

        strncpy( car.brand, brand->valuestring, BRAND_LEN );
        car.brand[ BRAND_LEN - 1 ] = 0x0;
        mask |= BRAND_FIELD;
    }

    model = cJSON_GetObjectItemCaseSensitive( carJson, "model" );

    if ( model != NULL && cJSON_IsString( model ) && ( model->valuestring != NULL ) ) {

        strncpy( car.model, model->valuestring, MODEL_LEN );
        car.model[ MODEL_LEN - 1 ] = 0x0;
        mask |= MODEL_FIELD;
    }

    year = cJSON_GetObjectItemCaseSensitive( carJson, "year" );

    if ( year != NULL && cJSON_IsNumber( year ) ) {

        car.year = year->valueint;
        mask |= YEAR_FIELD;
    }

    numberPlate = cJSON_GetObjectItemCaseSensitive( carJson, "numberPlate" );

    if ( numberPlate != NULL && cJSON_IsString( numberPlate ) && ( numberPlate->valuestring != NULL ) ) {

        strncpy( car.numberPlate, numberPlate->valuestring, NUMBER_PLATE_LEN );
        car.numberPlate[ NUMBER_PLATE_LEN - 1 ] = 0x0;
        mask |= NUMBER_PLATE_FIELD;
    }

    active = cJSON_GetObjectItemCaseSensitive( carJson, "active" );

    if ( active != NULL && cJSON_IsBool( active ) ) {

        car.active = active->valueint;
        mask |= ACTIVE_FIELD;
    }

    carId = cJSON_GetObjectItemCaseSensitive( carJson, "carId" );

    if ( carId != NULL && cJSON_IsNumber( carId ) ) {

        car.carId = carId->valueint;
        mask |= CAR_ID_FIELD;
    }
    // END - Factorize

    int id = 0;
    struct Car * retrievedCar;
    char * carAsJson;
    char * carDesc;
 
    // Do operation
    switch ( operation ) {

        case CREATE_OPER:
            if ( ! ( mask & BRAND_FIELD ) ||
                ! ( mask & MODEL_FIELD ) ||
                ! ( mask & YEAR_FIELD ) ||
                ! ( mask & ACTIVE_FIELD ) ||
                ! ( mask &  NUMBER_PLATE_FIELD ) ) {

                sprintf( msg, "{ \"message\": \"Car definition incomplete in input json\", \"status\": %d }", SC_BAD_REQUEST );
                messageErr( msg );
                break;
            }

            id = createCar( &car );

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

                retrievedCar = retrieveCar( car.carId );
                if ( retrievedCar != NULL ) {

                    rc = retrievedCar->carId;
                    carAsJson = carToJson( retrievedCar );
                    sprintf( msg, "{ \"car\": %s, \"status\": %d }", carAsJson, SC_OK );
                    messageOut( msg );
                    free( carAsJson );
                    free( retrievedCar );
                }
                else {

                    sprintf( msg, "{ \"message\": \"Car with carId = %d not found\", \"status\": %d }", car.carId, SC_NOT_FOUND );
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
                
                retrievedCar = retrieveCar( car.carId );
                if ( retrievedCar != NULL ) {

                    carCopyToConditional( &car, retrievedCar, mask );
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

                    sprintf( msg, "{ \"message\": \"Car with carId = %d not found\", \"status\": %d }", car.carId, SC_NOT_FOUND );
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

                id = deleteCar( car.carId );
                rc = id;
            }
            break;

        default:
            sprintf( msg, "{ \"message\": \"Operation unknown: %c\", \"status\": %d }", operation, SC_BAD_REQUEST );
            messageErr( msg );
    }

    free( msg );
    cJSON_Delete( carJson );

    return rc;
}

char * carToString( const struct Car * car ) {

    char * carJson = calloc( MAX_MESSAGE_LEN, sizeof( char ) );

    if ( car != NULL ) {

        sprintf( carJson, "brand: %s, model: %s, year: %d, numberPlate: %s, active: %s, carId: %d", car->brand, car->model, car->year, car->numberPlate, car->active ? "true" : "false", car->carId );
        return carJson;
    }
    else
        return "NULL";
}

char * carToJson( const struct Car * car ) {

    char * carJson = calloc( MAX_MESSAGE_LEN, sizeof( char ) );

    if ( car != NULL ) {

        sprintf( carJson, "{ \"brand\": \"%s\", \"model\": \"%s\", \"year\": %d, \"numberPlate\": \"%s\", \"active\": %s, \"carId\": %d }", car->brand, car->model, car->year, car->numberPlate, car->active ? "true" : "false", car->carId );
        return carJson;
    }
    else
        return "{ \"error\": \"NULL\" }";
}

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

void carCopyToConditional( const struct Car * source, struct Car * dest, int mask ) {

    if ( source != NULL && dest != NULL ) {

        if ( mask & BRAND_FIELD ) strcpy( dest->brand, source->brand );
        if ( mask & MODEL_FIELD ) strcpy( dest->model, source->model );
        if ( mask & NUMBER_PLATE_FIELD ) strcpy( dest->numberPlate, source->numberPlate );
        if ( mask & YEAR_FIELD ) dest->year = source->year;
        if ( mask & ACTIVE_FIELD ) dest->active = source->active;
        if ( mask & CAR_ID_FIELD ) dest->carId = source->carId;
    }
}