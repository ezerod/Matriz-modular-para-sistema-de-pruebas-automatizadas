//=====[Libraries]=============================================================
#include "mbed.h"
#include "switching_matrix_system.h"
#include "pc_serial_com.h"
#include "i2c_com.h"
#include "non_blocking_delay.h"
#include "relay_matrix.h"
#include "arm_book_lib.h"
#include "state_machine.h"
#include "event_log.h"
#include "date_and_time.h"
#include <string>

//=====[Declaration of private defines]========================================
#define SYSTEM_TIME_INCREMENT_IN_MS 100

//=====[Declaration of private data types]=====================================
//=====[Declaration and initialization of public global objects]===============
//=====[Declaration of external public global variables]=======================
//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============
static systemStateMachine_t systemStateMachine;
static int submoduleQty = 0;
static int idOfSubmodule = 0;
static int submoduleCounter = 0;
static int rowPosition = 0;
static int colPosition = 0;
static bool reconfigureSubmodules = false;
static nonBlockingDelay_t systemDelay;
static int year;
static int month;
static int day;
static int hour;
static int minute;
static int second;

//=====[Declarations (prototypes) of private functions]========================
static void updateModules();
static void startSetUpModuleUpdate();
static void setRowPositionOfModuleUpdate();
static void setColPositionOfModuleUpdate();
static void endSetUpModuleUpdate();
static void principalMenuUpdate();
static void sequenceModeUpdate(){};
static void manualModeUpdate();
static void setDateUpdate();
static void setTimeUpdate();
static bool checkChangeOfSubmodules();
static void handleChangeOfSubmodules();
static void stateTransition(systemStateMachine_t newState);
static void parseDate(const char* date, char* day, char* month, char* year);
static void parseTime(const char* timeString, char* hour, char* minute, char* second);
//=====[Implementations of public functions]===================================
void switchingMatrixSystemInit() {
    i2cComInit();
    pcSerialComInit();
    relayMatrixInit();
    eventLogInit();
    submoduleQty = i2cComGetSubmoduleQty();
    idOfSubmodule = i2cComGetIdOfFirstSubmodule();
    nonBlockingDelayInit(&systemDelay, SYSTEM_TIME_INCREMENT_IN_MS);
    stateTransition(SET_DATE);
}

void switchingMatrixSystemUpdate() {
    if(nonBlockingDelayRead(&systemDelay)) {
        updateModules();
        if(checkChangeOfSubmodules() == true || reconfigureSubmodules)
            handleChangeOfSubmodules();
        switch(systemStateMachine) {
            case SET_DATE: setDateUpdate(); break;
            case SET_TIME: setTimeUpdate(); break;
            case START_SET_UP_MODULE: startSetUpModuleUpdate();  break;
            case SET_ROW_POSITION_OF_MODULE: setRowPositionOfModuleUpdate(); break;
            case SET_COL_POSITION_OF_MODULE: setColPositionOfModuleUpdate(); break;
            case END_SET_UP_MODULE: endSetUpModuleUpdate(); break;
            case PRINCIPAL_MENU: principalMenuUpdate(); break;
            case SEQUENCE_MODE: sequenceModeUpdate(); break;
            case MANUAL_MODE: manualModeUpdate(); break;
            default: break; 
        }   
    }
}

//=====[Implementations of private functions]==================================
static bool checkChangeOfSubmodules() {
    return i2cComChangeInSubmodulesRecognized();
}

static void handleChangeOfSubmodules() {
    pcSerialComStringWrite("---------------------------------------------------------------\n");
    pcSerialComStringWrite("There was a change in the submodules connected!!\n");
    pcSerialComStringWrite("Please, reconfigure the submodules\n");
    pcSerialComStringWrite("---------------------------------------------------------------\n");
    stateTransition(START_SET_UP_MODULE);
    idOfSubmodule = i2cComGetIdOfFirstSubmodule();
    submoduleQty = i2cComGetSubmoduleQty();
    submoduleCounter = 0;
    i2cComChangeInSubmodulesHandled();
    relayMatrixClearMatrixes();

    eventLogEventRecord(I2C_CHANGE_IN_SUBMODULES);
}

static void updateModules() {
    pcSerialComUpdate();
    i2cComUpdate();
    eventLogUpdate();
}

static void startSetUpModuleUpdate() {
    i2cComStartIdentificationOfSubmodule(idOfSubmodule);
    eventLogEventRecord(I2C_SUBMODULE_IDENTIFYING, idOfSubmodule);
    stateTransition(SET_ROW_POSITION_OF_MODULE);
}

static void setRowPositionOfModuleUpdate() {
    char buffer[10];
    char* aux;
    if(pcSerialComGetRow(buffer)) {
        rowPosition = strtoul(buffer, &aux, 10);
        stateTransition(SET_COL_POSITION_OF_MODULE);
    }
}

static void setColPositionOfModuleUpdate() {
    char buffer[10];
    char* aux;
    if( pcSerialComGetCol(buffer)) {
        colPosition = strtoul(buffer, &aux, 10);
        stateTransition(END_SET_UP_MODULE);
    }
}

static void endSetUpModuleUpdate() {
    if(relayMatrixInsertModule(idOfSubmodule, rowPosition, colPosition) == false) {

        pcSerialComStringWrite("---------------------------------------------------------------\n");
        pcSerialComStringWrite("Module not Added\nInvalid position entered! Please enter the row and column again\n");
        pcSerialComStringWrite("---------------------------------------------------------------\n");

        eventLogEventRecord(MODULE_MATRIX_INVALID_POSITION, idOfSubmodule);
        stateTransition(START_SET_UP_MODULE);
    }
    else if(submoduleCounter == submoduleQty - 1) {
        pcSerialComStringWrite("---------------------------------------------------------------\n");
        pcSerialComStringWrite("Module added.\n");
        pcSerialComStringWrite("---------------------------------------------------------------\n");
        eventLogEventRecord(I2C_SUBMODULE_IDENTIFIED, idOfSubmodule);

        relayMatrixGenerate();
        eventLogEventRecord(RELAY_MATRIX_SUCCESSFULLY_CONFIGURED);
        relayMatrixPrintMatrix();

        stateTransition(PRINCIPAL_MENU);
    }
    else {
        pcSerialComStringWrite("---------------------------------------------------------------\n");
        pcSerialComStringWrite("Module added.\n");
        pcSerialComStringWrite("---------------------------------------------------------------\n");

        eventLogEventRecord(I2C_SUBMODULE_IDENTIFIED, idOfSubmodule);
        submoduleCounter++;
        idOfSubmodule++;
        stateTransition(START_SET_UP_MODULE);
    }
    i2cComStopIdentificationOfSubmodule(idOfSubmodule);
}

static void principalMenuUpdate() {
    char buffer[10];
    int option;
    char* aux;
    if( pcSerialComGetPrincipalMenuOptions(buffer)) {
        option = strtoul(buffer, &aux, 10);
        switch(option) {
            case 1:
                stateTransition(MANUAL_MODE); break;
            case 2:
                stateTransition(SEQUENCE_MODE); break;

            default: pcSerialComStringWrite("Invalid option!!\n"); stateTransition(PRINCIPAL_MENU); break;
        }
    }
}


static void manualModeUpdate() {
    char buffer[10];
    int relayToToggle = 0;
    char* aux;

    if(pcSerialComGetRelayIdToToggle(buffer)) {

        if(buffer[0] == 'x') {
            stateTransition(PRINCIPAL_MENU);
        }
        else {
            relayToToggle = strtoul(buffer, &aux, 10);
            if(! relayMatrixToggleRelay(relayToToggle)) {
                pcSerialComStringWrite("---------------------------------------------------------------\n");
                pcSerialComStringWrite("Relay not found. Operation ignored\n");
                pcSerialComStringWrite("---------------------------------------------------------------\n");
            }
            else {
                if(relayMatrixReadStateOfRelay(relayToToggle) == ON) {
                    eventLogEventRecord(RELAY_CLOSED, relayToToggle);
                }
                else {
                    eventLogEventRecord(RELAY_OPENED, relayToToggle);
                }
            }
            relayMatrixPrintMatrix();
            stateTransition(MANUAL_MODE);
        }
    }
}

static void stateTransition(systemStateMachine_t nextState) {
    systemStateMachine = nextState;
    pcSerialComSetSystemState(nextState);
}

static void setDateUpdate() {
    char buffer[10];
    char* aux;
    char _year[5] = "";
    char _month[3] = "";
    char _day[3] = "";

    if(pcSerialComGetDate(buffer)) {
        parseDate(buffer, _day, _month, _year);
        year = strtoul(_year, &aux, 10);
        month = strtoul(_month, &aux, 10);
        day = strtoul(_day, &aux, 10);
        if(year < 2023) {
            stateTransition(SET_DATE);
        }
        else if(month < 1 && month > 12) {
            stateTransition(SET_DATE);
        }
        else if(day < 1 && day > 31) {
            stateTransition(SET_DATE);
        }
        else {
            stateTransition(SET_TIME);
        }
    }
}

static void setTimeUpdate() {
    char buffer[10];
    char* aux;
    char _hour[3] = "";
    char _minute[3] = "";
    char _second[3] = "";

    if(pcSerialComGetDate(buffer)) {
        parseTime(buffer, _hour, _minute, _second);
        hour = strtoul(_hour, &aux, 10);
        minute = strtoul(_minute, &aux, 10);
        second = strtoul(_second, &aux, 10);
        if(hour < 0 && hour > 23) {
            stateTransition(SET_TIME);
        }
        else if(minute < 0 && minute > 59) {
            stateTransition(SET_TIME);
        }
        else if(second < 0 && second > 59) {
            stateTransition(SET_TIME);
        }
        else {
            dateAndTimeWrite(year, month, day, hour, minute, second);
            eventLogSetFilename();
            stateTransition(START_SET_UP_MODULE);
        }
    }
}

static void parseDate(const char* date, char* day, char* month, char* year) {
    std::strncpy(day, date, 2);
    day[2] = '\0';

    std::strncpy(month, date + 2, 2);
    month[2] = '\0';

    std::strncpy(year, date + 4, 4);
    year[4] = '\0';
}

static void parseTime(const char* timeString, char* hour, char* minute, char* second) {
    std::strncpy(hour, timeString, 2);
    hour[2] = '\0';

    std::strncpy(minute, timeString + 2, 2);
    minute[2] = '\0';

    std::strncpy(second, timeString + 4, 2);
    second[2] = '\0';
}
