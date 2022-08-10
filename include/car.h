#ifndef _CAR_H
#define _CAR_H

#include <stdbool.h>

#define CAR_BRAND_FIELD          0x01
#define CAR_MODEL_FIELD          0x02
#define CAR_YEAR_FIELD           0x04
#define CAR_ACTIVE_FIELD         0x08
#define CAR_NUMBER_PLATE_FIELD   0x10
#define CAR_ID_FIELD             0x20

#define ALL_FIELDS              CAR_BRAND_FIELD + CAR_MODEL_FIELD + CAR_YEAR_FIELD + CAR_ACTIVE_FIELD + CAR_NUMBER_PLATE_FIELD + CAR_ID_FIELD

#define CAR_VALIDATE_CREATE_OPERATION   0x01
#define CAR_VALIDATE_UPDATE_OPERATION   0x02


// Table car.db
// A car has a
//   brand          char[ 32 ]
//   model          char[ 64 ]
//   year           int
//   active         bool
//   numberPlate    char[ 16 ]
//   carId          int > 0, unique

#define BRAND_LEN        32
#define MODEL_LEN        64
#define NUMBER_PLATE_LEN 16

#define MIN_CAR_YEAR        2000


struct Car {
    char brand[ BRAND_LEN ];
    char model[ MODEL_LEN ];
    int year;
    bool active;
    char numberPlate[ NUMBER_PLATE_LEN ];
    int carId;
};

struct Car * parseJsonAsCar( const char *, int * );
int carOperation( char, const char * );
void carCopyTo( const struct Car *, struct Car * );
void carCopyToConditional( const struct Car *, struct Car *, int );
char * carToString( const struct Car * );
char * carToJson( const struct Car * );
bool validateCar( int, int, const struct Car *, bool );

#endif
