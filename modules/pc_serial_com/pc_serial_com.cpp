#include "pc_serial_com.h"
#include "mbed.h"
#include <string>

#define MATRIX_MODULE_TOP   " ---- "
#define MATRIX_MODULE_BOTTOM " ---- "
#define MATRIX_MODULE_RIGHT "|"
#define MATRIX_MODULE_LEFT  "|"
#define MAX_LEN_OF_STRING_TO_PRINT 100


typedef enum {
    IDLE,
    TALK_TO_USER,
    WAIT_USER_RESPONSE
}   pcSerialComUserInteraction_t;

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);
static systemStateMachine_t pcSerialSystemStateMachine;
static pcSerialComUserInteraction_t userInteraction = TALK_TO_USER;
static modesOfOperation_t modeSelectUser;
static bool modeSelected = false;
static int row = 0;
static int col = 0;
static char userResponse[10];
static bool dataAvailableToRead = false;

static void printWelcomeMessage();
static void startSetUpModuleUpdate();
static void getRowPositionOfModule();
static void getColPositionOfModule();
static void endSetUpModuleUpdate();
static void principalMenuUpdate();
static void manualModeUpdate();
static bool getUserResponse();

void pcSerialComInit() {
    printWelcomeMessage();
};

char pcSerialComCharRead()
{
    char receivedChar = '\0';
    if( uartUsb.readable() ) {
        uartUsb.read( &receivedChar, 1 );
    }
    return receivedChar;
}


void pcSerialComStringWrite( const char* str )
{
    uartUsb.write( str, strlen(str) );
}

void pcSerialComUpdate() {
    switch(pcSerialSystemStateMachine) {
        case START_SET_UP_MODULE: startSetUpModuleUpdate(); break;
        case SET_ROW_POSITION_OF_MODULE: getRowPositionOfModule(); break;
        case SET_COL_POSITION_OF_MODULE: getColPositionOfModule(); break;
        case END_SET_UP_MODULE: endSetUpModuleUpdate(); break;
        case PRINCIPAL_MENU: principalMenuUpdate(); break;
        //case SEQUENCE_MODE:
        case MANUAL_MODE: manualModeUpdate();
        default: break;
    }
}

void pcSerialComSetSystemState(systemStateMachine_t systemState) {
    pcSerialSystemStateMachine = systemState;
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

bool getUserResponse() {
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
    pcSerialComStringWrite("--------------------------------\n");
    pcSerialComStringWrite("            WELCOME             \n");
    pcSerialComStringWrite("--------------------------------\n");
    pcSerialComStringWrite("    SWITCHING MATRIX SYSTEM     \n");
    pcSerialComStringWrite("Authored by: Ezequiel Rodriguez \n");
    pcSerialComStringWrite("    Embedded Systems - FIUBA    \n");
    pcSerialComStringWrite("--------------------------------\n");

}

static void startSetUpModuleUpdate() {
    pcSerialComStringWrite("--------------------------------\n");
    pcSerialComStringWrite("Identifying Submodule...\n");
    
}

static void getRowPositionOfModule() {
    switch(userInteraction) {
        case TALK_TO_USER:
            pcSerialComStringWrite("Insert a row to place the module:\t");
            userInteraction = WAIT_USER_RESPONSE;
            break;
        case WAIT_USER_RESPONSE:
            if(getUserResponse()) {
                userInteraction = TALK_TO_USER;
                dataAvailableToRead = true;
            }
            break;
        default: break;
    }
}

static void getColPositionOfModule() {
    char receivedChar;
    switch(userInteraction) {
        case TALK_TO_USER:
            pcSerialComStringWrite("Insert a column to place the module:\t");
            userInteraction = WAIT_USER_RESPONSE;
            break;
        case WAIT_USER_RESPONSE:
            if(getUserResponse()) {
                userInteraction = TALK_TO_USER;
                dataAvailableToRead = true;
            }
            break;
        default: break;
    }
}

static void endSetUpModuleUpdate() {
    pcSerialComStringWrite("The module is configured\n");
    pcSerialComStringWrite("--------------------------------\n");
}

static void principalMenuUpdate() {
    char receivedChar;
    switch(userInteraction) {
        case TALK_TO_USER:
            pcSerialComStringWrite("--------------------------------\n");
            pcSerialComStringWrite("           PRINCIPAL MENU       \n");
            pcSerialComStringWrite("--------------------------------\n");
            pcSerialComStringWrite("Select an option:\n");
            pcSerialComStringWrite("1. Manual mode operation\n");
            pcSerialComStringWrite("2. Sequence mode operation\n");
            userInteraction = WAIT_USER_RESPONSE;
            break;
        case WAIT_USER_RESPONSE:
            receivedChar = pcSerialComCharRead();
            switch(receivedChar) {
                case '1':
                    modeSelectUser = MANUAL_OPERATION;
                    modeSelected = true;
                    break;
                case '2':
                    modeSelectUser = SEQUENCE_OPERATION;
                    modeSelected = true;
                break;
                default: break;
            }
            break;
        default: break;
    }
}

bool pcSerialComPrincipalMenuUserSelection(modesOfOperation_t &modeSelect) {
    modeSelect = modeSelectUser;
    return modeSelected;
}

static void manualModeUpdate() {
    switch(userInteraction) {
        case TALK_TO_USER:
            pcSerialComStringWrite("--------------------------------\n");
            pcSerialComStringWrite("         MANUAL OPERATION       \n");
            pcSerialComStringWrite("--------------------------------\n");
            pcSerialComStringWrite("Enter a relay ID to toggle state:\t");

            userInteraction = WAIT_USER_RESPONSE;
            break;
        case WAIT_USER_RESPONSE:
            if(getUserResponse()) {
                userInteraction = TALK_TO_USER;
                dataAvailableToRead = true;
            }
            break;
        default: break;
    }
}

bool pcSerialComGetRelayIdToToggle(char* relayToToggle) {
    bool relayReadCopy = dataAvailableToRead;
    strcpy(relayToToggle, userResponse);
    return relayReadCopy;
}