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
static bool modeSelected = false;
static modesOfOperation_t modeSelectUser;
static char* validOptions = nullptr;
static bool optionSelected = false;
static char _optionFromUser;
static int optionQty = 0;

static void printWelcomeMessage();
static void startSetUpModuleUpdate();
static void getPositionOfModule();
static void endSetUpModuleUpdate();
static void principalMenuUpdate();


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
        case SET_POSITION_OF_MODULE: getPositionOfModule(); break;
        case END_SET_UP_MODULE: endSetUpModuleUpdate(); break;
        case PRINCIPAL_MENU: principalMenuUpdate(); break;
        //case SEQUENCE_MODE:
        //case MANUAL_MODE
        default: break;
    }
}

bool pcSerialComGetPositionFromUser(char &optionFromUser) {
    if(optionSelected) {
        optionFromUser = _optionFromUser; 
    }

    return optionSelected;
}

void pcSerialComSetSystemState(systemStateMachine_t systemState) {
    pcSerialSystemStateMachine = systemState;
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

static void getPositionOfModule() {
    char receivedChar;
    switch(userInteraction) {
        case TALK_TO_USER:
            pcSerialComStringWrite(validOptions);
            userInteraction = WAIT_USER_RESPONSE;
            break;
        case WAIT_USER_RESPONSE:
            receivedChar = pcSerialComCharRead();
            if(isdigit(receivedChar)) {
                if(atoi(&receivedChar) < optionQty) {
                    _optionFromUser = receivedChar;
                    userInteraction = TALK_TO_USER;
                    optionSelected = true;
                }
            }
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