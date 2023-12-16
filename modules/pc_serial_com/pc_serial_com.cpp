//=====[Libraries]=============================================================
#include "pc_serial_com.h"
#include "mbed.h"
#include <string>

//=====[Declaration of private defines]========================================
#define MAX_LENGTH_USER_RESPONSE 10

//=====[Declaration of private data types]=====================================
typedef enum {
    IDLE,
    TALK_TO_USER,
    WAIT_USER_RESPONSE
}   pcSerialComUserInteraction_t;

//=====[Declaration and initialization of public global objects]===============
UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============
static systemStateMachine_t pcSerialSystemStateMachine;
static pcSerialComUserInteraction_t userInteraction = TALK_TO_USER;
static char userResponse[MAX_LENGTH_USER_RESPONSE];
static bool dataAvailableToRead = false;

//=====[Declarations (prototypes) of private functions]========================
static void printWelcomeMessage();
static bool getUserResponse();
static void startSetUpModuleUpdate();
static void getRowPositionOfModule();
static void getColPositionOfModule();
static void endSetUpModuleUpdate();
static void principalMenuUpdate();
static void manualModeUpdate();
static void sequenceModeUpdate();
static void getTimeUpdate();
static void getDateUpdate();
//=====[Implementations of public functions]===================================
void pcSerialComInit() {
    printWelcomeMessage();
}

char pcSerialComCharRead() {
    char receivedChar = '\0';
    if( uartUsb.readable() ) {
        uartUsb.read( &receivedChar, 1 );
    }
    return receivedChar;
}

void pcSerialComStringWrite( const char* str ) {
    uartUsb.write( str, strlen(str) );
}

void pcSerialComUpdate() {
    switch(pcSerialSystemStateMachine) {
        case SET_DATE: getDateUpdate(); break;
        case SET_TIME: getTimeUpdate(); break;
        case START_SET_UP_MODULE: startSetUpModuleUpdate(); break;
        case SET_ROW_POSITION_OF_MODULE: getRowPositionOfModule(); break;
        case SET_COL_POSITION_OF_MODULE: getColPositionOfModule(); break;
        case END_SET_UP_MODULE: endSetUpModuleUpdate(); break;
        case PRINCIPAL_MENU: principalMenuUpdate(); break;
        case SEQUENCE_MODE: sequenceModeUpdate(); break;
        case MANUAL_MODE: manualModeUpdate();
        default: break;
    }
}

void pcSerialComSetSystemState(systemStateMachine_t systemState) {
    pcSerialSystemStateMachine = systemState;
    switch(pcSerialSystemStateMachine) {
        case SET_DATE: userInteraction = TALK_TO_USER; break;
        case SET_TIME: userInteraction = TALK_TO_USER; break;
        case START_SET_UP_MODULE: break;
        case SET_ROW_POSITION_OF_MODULE: userInteraction = TALK_TO_USER; break;
        case SET_COL_POSITION_OF_MODULE: userInteraction = TALK_TO_USER; break;
        case END_SET_UP_MODULE: break;
        case PRINCIPAL_MENU: userInteraction = TALK_TO_USER; break;
        case SEQUENCE_MODE:  break;
        case MANUAL_MODE: userInteraction = TALK_TO_USER;
    }
    dataAvailableToRead = false;
}

bool pcSerialComGetRow(char* rowPosition) {
    bool rowReadCopy = dataAvailableToRead;
    strcpy(rowPosition, userResponse);
    return rowReadCopy;
}

bool pcSerialComGetCol(char* colPosition) {
    bool colReadCopy = dataAvailableToRead;
    strcpy(colPosition, userResponse);
    return colReadCopy;
}

bool pcSerialComGetPrincipalMenuOptions(char* option) {
    bool optionReadCopy = dataAvailableToRead;
    strcpy(option, userResponse);
    return optionReadCopy;
}

bool pcSerialComGetRelayIdToToggle(char* relayToToggle) {
    bool relayReadCopy = dataAvailableToRead;
    strcpy(relayToToggle, userResponse);
    return relayReadCopy;
}

bool pcSerialComGetDate(char* date) {
    bool dateRead = dataAvailableToRead;
    strcpy(date, userResponse);
    return dateRead;
}

bool pcSerialComGetTime(char* _time) {
    bool timeRead = dataAvailableToRead;
    strcpy(_time, userResponse);
    return timeRead;
}

//=====[Implementations of private functions]==================================
static bool getUserResponse() {
    char receivedChar = pcSerialComCharRead();
    bool taskCompleted = false;
    static int len = 0;
    if(receivedChar != '\0') {
        if(receivedChar != 13) {
            userResponse[len] = receivedChar;
            len ++;
        }
        else {
            userResponse[len] = '\0';
            len = 0;
            taskCompleted = true;
        }
    }

    return taskCompleted;
}

static void printWelcomeMessage() {
    pcSerialComStringWrite("---------------------------------------------------------------\n");
    pcSerialComStringWrite("            WELCOME             \n");
    pcSerialComStringWrite("---------------------------------------------------------------\n");
    pcSerialComStringWrite("    SWITCHING MATRIX SYSTEM     \n");
    pcSerialComStringWrite("Authored by: Ezequiel Rodriguez \n");
    pcSerialComStringWrite("    Embedded Systems - FIUBA    \n");
    pcSerialComStringWrite("---------------------------------------------------------------\n");

}

static void startSetUpModuleUpdate() {
    pcSerialComStringWrite("---------------------------------------------------------------\n");
    pcSerialComStringWrite("Identifying Submodule...\n");
    pcSerialComStringWrite("---------------------------------------------------------------\n");
    
}

static void getRowPositionOfModule() {
    switch(userInteraction) {
        case TALK_TO_USER:
            pcSerialComStringWrite("Insert a row to place the module:\t");
            userInteraction = WAIT_USER_RESPONSE;
            break;
        case WAIT_USER_RESPONSE:
            if(getUserResponse()) {
                userInteraction = IDLE;
                dataAvailableToRead = true;
                pcSerialComStringWrite("\n");
                pcSerialComStringWrite("---------------------------------------------------------------\n");
            }
            break;
        default: break;
    }
}

static void getColPositionOfModule() {
    switch(userInteraction) {
        case TALK_TO_USER:
            pcSerialComStringWrite("Insert a column to place the module:\t");
            userInteraction = WAIT_USER_RESPONSE;
            break;
        case WAIT_USER_RESPONSE:
            if(getUserResponse()) {
                userInteraction = IDLE;
                dataAvailableToRead = true;
                pcSerialComStringWrite("\n");
                pcSerialComStringWrite("---------------------------------------------------------------\n");
            }
            break;
        default: break;
    }
}

static void endSetUpModuleUpdate() {
    pcSerialComStringWrite("The module is configured\n");
    pcSerialComStringWrite("---------------------------------------------------------------\n");
}

static void principalMenuUpdate() {
    switch(userInteraction) {
        case TALK_TO_USER:
            pcSerialComStringWrite("---------------------------------------------------------------\n");
            pcSerialComStringWrite("           PRINCIPAL MENU       \n");
            pcSerialComStringWrite("---------------------------------------------------------------\n");
            pcSerialComStringWrite("Select an option:\n");
            pcSerialComStringWrite("1. Manual mode operation\n");
            pcSerialComStringWrite("2. Sequence mode operation\n");
            userInteraction = WAIT_USER_RESPONSE;
            break;
         case WAIT_USER_RESPONSE:
            if(getUserResponse()) {
                userInteraction = IDLE;
                dataAvailableToRead = true;
                pcSerialComStringWrite("\n");
                pcSerialComStringWrite("---------------------------------------------------------------\n");
            }
            break;
        default: break;
    }
}

static void manualModeUpdate() {
    switch(userInteraction) {
        case TALK_TO_USER:
            pcSerialComStringWrite("---------------------------------------------------------------\n");
            pcSerialComStringWrite("         MANUAL OPERATION       \n");
            pcSerialComStringWrite("---------------------------------------------------------------\n");
            pcSerialComStringWrite("To go to PRINCIPAL MENU press 'x'\n\n");
            pcSerialComStringWrite("Enter a relay ID to toggle state:\t");
            userInteraction = WAIT_USER_RESPONSE;
            break;
        case WAIT_USER_RESPONSE:
            if(getUserResponse()) {
                userInteraction = IDLE;
                dataAvailableToRead = true;
                pcSerialComStringWrite("\n");
                pcSerialComStringWrite("---------------------------------------------------------------\n");
            }
            break;
        default: break;
    }
}

static void sequenceModeUpdate() {
}

static void getDateUpdate() {
    switch(userInteraction) {
        case TALK_TO_USER:
            pcSerialComStringWrite("Set the date in ddmmyyyy format: \t");
            userInteraction = WAIT_USER_RESPONSE;
            break;
        case WAIT_USER_RESPONSE:
            if(getUserResponse()) {
                userInteraction = IDLE;
                dataAvailableToRead = true;
                pcSerialComStringWrite("\n");
                pcSerialComStringWrite("---------------------------------------------------------------\n");
            }
            break;
        default: break;
    }
}

static void getTimeUpdate() {
    switch(userInteraction) {
        case TALK_TO_USER:
            pcSerialComStringWrite("Set the time in hhmmss format: \t");
            userInteraction = WAIT_USER_RESPONSE;
            break;
        case WAIT_USER_RESPONSE:
            if(getUserResponse()) {
                userInteraction = IDLE;
                dataAvailableToRead = true;
                pcSerialComStringWrite("\n");
                pcSerialComStringWrite("---------------------------------------------------------------\n");
            }
            break;
        default: break;
    }
}