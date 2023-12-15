//=====[#include guards - begin]===============================================

#ifndef _EVENT_LOG_H_
#define _EVENT_LOG_H_

//=====[Declaration of public defines]=========================================

#define EVENT_LOG_MAX_STORAGE        20
#define EVENT_HEAD_STR_LENGTH         8
#define EVENT_LOG_NAME_MAX_LENGTH    50
#define DATE_AND_TIME_STR_LENGTH     18
#define CTIME_STR_LENGTH             25
#define NEW_LINE_STR_LENGTH           3
#define EVENT_STR_LENGTH             (EVENT_HEAD_STR_LENGTH + \
                                      EVENT_LOG_NAME_MAX_LENGTH + \
                                      DATE_AND_TIME_STR_LENGTH  + \
                                      CTIME_STR_LENGTH + \
                                      NEW_LINE_STR_LENGTH)
#define MAX_QTY_OF_EVENTS 10
#define MAX_LEN_OF_EVENT_STRING 50

//=====[Declaration of public data types]======================================
typedef enum {
    //RELAY MATRIX RELATED EVENTS
    RELAY_CLOSED = 0,
    RELAY_OPENED = 1,
    RELAY_MATRIX_SUCCESSFULLY_CONFIGURED = 2,
    MODULE_MATRIX_INVALID_POSITION = 3,
    //SUBMODULE HANDLING RELATED EVENTS
    I2C_SUBMODULE_IDENTIFYING = 4,
    I2C_SUBMODULE_IDENTIFIED = 5,
    I2C_CHANGE_IN_SUBMODULES = 6,
    
}systemEvents_t;

const char eventStrings[MAX_QTY_OF_EVENTS][MAX_LEN_OF_EVENT_STRING] = { "RELAY %d CLOSED",
                                                                        "RELAY %d OPENED",
                                                                        "RELAY MATRIX CONFIGURED",
                                                                        "INVALID COORDINATES FOR SUBMODULE %d",
                                                                        "SUBMODULE %d START ID PROCESS",
                                                                        "SUBMODULE %d ID PROCESS COMPLETED",
                                                                        "CHANGE IN SUBMODULE CONFIGURATION",
                                                                      };

//=====[Declarations (prototypes) of public functions]=========================
void eventLogInit();
void eventLogUpdate();
void eventLogEventRecord(systemEvents_t eventIndex, int id = -1);
void eventLogSetFilename();
//=====[#include guards - end]=================================================

#endif // _EVENT_LOG_H_