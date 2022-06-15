#ifndef _CAR_H
#define _CAR_H

#include <stdbool.h>

#define BRAND_FIELD          0x01
#define MODEL_FIELD          0x02
#define YEAR_FIELD           0x04
#define ACTIVE_FIELD         0x08
#define NUMBER_PLATE_FIELD   0x10
#define CAR_ID_FIELD         0x20


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

struct Car {
    char brand[ BRAND_LEN ];
    char model[ MODEL_LEN ];
    int year;
    bool active;
    char numberPlate[ NUMBER_PLATE_LEN ];
    int carId;
};

int carOperation( char, const char * );
void carCopyTo( const struct Car *, struct Car * );
void carCopyToConditional( const struct Car *, struct Car *, int );
char * carToString( const struct Car * );
char * carToJson( const struct Car * );

#endif
