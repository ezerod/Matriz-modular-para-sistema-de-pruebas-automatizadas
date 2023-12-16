//=====[Libraries]=============================================================
#include "mbed.h"
#include "arm_book_lib.h"
#include "event_log.h"
#include "state_machine.h"
#include "date_and_time.h"
#include "pc_serial_com.h"
#include "sd_card.h"
#include "non_blocking_delay.h"
#include <string>

//=====[Declaration of private defines]========================================
#define LOGGING_TIME_IN_MS 1000
//=====[Declaration of private data types]=====================================
typedef struct systemEvent {
    time_t seconds;
    char typeOfEvent[EVENT_LOG_NAME_MAX_LENGTH];
} systemEvent_t;

//=====[Declaration and initialization of public global objects]===============

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============
static char fileName[SD_CARD_FILENAME_MAX_LENGTH];
static int eventsIndex     = 0;
static systemEvent_t arrayOfStoredEvents[EVENT_LOG_MAX_STORAGE];
static bool newEventsToLog = false;
static bool fileNameSet = false;
static nonBlockingDelay_t loggingTimer;
bool sdInitialized = false;

//=====[Declarations (prototypes) of private functions]========================
static void clearArrayOfStoredEvents();
static int eventLogNumberOfStoredEvents();
static void eventLogRead( int index, char* str );
static void eventLogWrite(const char* event);
static bool eventLogSaveToSdCard();

//=====[Implementations of public functions]===================================
void eventLogInit() {
    sdInitialized = sdCardInit();
    nonBlockingDelayInit(&loggingTimer, LOGGING_TIME_IN_MS);
}

void eventLogSetFilename() {
    time_t seconds;
    seconds = time(NULL);
    fileName[0] = '\0';

    strftime( fileName, SD_CARD_FILENAME_MAX_LENGTH, 
              "%Y_%m_%d_%H_%M_%S", localtime(&seconds) );
    strcat( fileName, ".txt" );
    fileNameSet = true;
}

void eventLogUpdate() {
    if(nonBlockingDelayRead(&loggingTimer)) {
        if(newEventsToLog && fileNameSet && sdInitialized) {
        eventLogSaveToSdCard();
        clearArrayOfStoredEvents();
        newEventsToLog = false;
        }
    }
}

void eventLogEventRecord(systemEvents_t eventIndex, int id) {
    char buffer[MAX_LEN_OF_EVENT_STRING];
    if(id == -1) {
        sprintf(buffer,"%s", eventStrings[eventIndex]);
    }
    else
        sprintf(buffer, eventStrings[eventIndex], id);

    eventLogWrite(buffer);
    newEventsToLog = true;
}

//=====[Implementations of private functions]==================================
static void clearArrayOfStoredEvents() {
    for(int i = 0; i < eventsIndex; i++) {
        strcpy(arrayOfStoredEvents[i].typeOfEvent, "");
    }
    eventsIndex = 0;
}

static bool eventLogSaveToSdCard() {
    char fileNameSD[SD_CARD_FILENAME_MAX_LENGTH+4] = "";
    fileNameSD[0] = '\0';
    char eventStr[EVENT_STR_LENGTH] = "";
    bool eventsStored = false;
    FILE* sdCardFilePointer;

    strcat( fileNameSD, "/sd/" );
    strcat( fileNameSD, fileName );

    sdCardFilePointer = fopen( fileNameSD, "a" );
    for (int i = 0; i < eventLogNumberOfStoredEvents(); i++) {
            eventLogRead( i, eventStr );
            if ( sdCardWriteFile( sdCardFilePointer, eventStr ) == true ) {
                eventsStored = true;
        }
    }
    fclose( sdCardFilePointer );
    
    return eventsStored;
}

static void eventLogWrite(const char* event) {
    arrayOfStoredEvents[eventsIndex].seconds = time(NULL);
    strcpy( arrayOfStoredEvents[eventsIndex].typeOfEvent, event);
    if ( eventsIndex < EVENT_LOG_MAX_STORAGE - 1 ) {
        eventsIndex++;
    } else {
        eventsIndex = 0;
    }
}

static void eventLogRead( int index, char* str ) {
    str[0] = '\0';
    strcat( str, "Event = " );
    strcat( str, arrayOfStoredEvents[index].typeOfEvent );
    strcat( str, "\r\nDate and Time = " );
    strcat( str, ctime(&arrayOfStoredEvents[index].seconds) );
    strcat( str, "\r\n" );
}

static int eventLogNumberOfStoredEvents() {
    return eventsIndex;
}