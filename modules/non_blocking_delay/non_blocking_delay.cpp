//=====[Libraries]=============================================================

#include "non_blocking_delay.h"

//=====[Declaration of private defines]========================================

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============

static tick_t tickCounter = 0;

//=====[Declarations (prototypes) of private functions]========================

void tickerCallback( );
tick_t tickRead( );

//=====[Implementations of private methods]====================================

void nonBlockingDelay::tickInit() {
    this->ticker.attach( tickerCallback, 1ms );
}

//=====[Implementations of public methods]=====================================

nonBlockingDelay::nonBlockingDelay( tick_t durationValue ) {
    this->tickInit( );
    this->duration = durationValue;
    this->isRunning = false;
}

bool nonBlockingDelay::read( ) {
    bool timeArrived = false;
    tick_t elapsedTime;

    if( !this->isRunning ) {
        this->startTime = tickCounter;
        this->isRunning = true;
    }    
    else {
        elapsedTime = tickCounter - this->startTime;
        if ( elapsedTime >= this->duration ) {
            timeArrived = true;
            this->isRunning = false;
        }
    }

    return timeArrived;
}

void nonBlockingDelay::write( tick_t durationValue ) {
    this->duration = durationValue;
}

//=====[Implementations of private functions]==================================

void tickerCallback( ) 
{
    tickCounter++;
}

tick_t tickRead( ) {
    return tickCounter;
}