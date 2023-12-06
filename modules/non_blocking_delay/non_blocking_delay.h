//=====[#include guards - begin]===============================================

#ifndef _nonBlockingDelay_H_
#define _nonBlockingDelay_H_

//=====[Libraries]=============================================================

#include "mbed.h"

//=====[Declaration of public defines]=========================================

//=====[Declaration of public data types]======================================

typedef uint64_t tick_t;

//=====[Declaration of public classes]=========================================

class nonBlockingDelay {
    public:
//=====[Declaration of public methods]=========================================
    nonBlockingDelay( tick_t durationValue );
    void write( tick_t durationValue );
    bool read( );

    private:
//=====[Declaration of privates atributes]=========================================
    tick_t startTime;
    tick_t duration;
    bool isRunning;
    Ticker ticker;
    
//=====[Declaration of privates methods]=========================================
    void tickInit( );
};

//=====[Declarations (prototypes) of public functions]=========================

//=====[#include guards - end]=================================================
#endif