#include "switching_matrix_system.h"
#include "pc_serial_com.h"
#include "i2c_com.h"
#include "state_machine.h"
#include "non_blocking_delay.h"
#include "mbed.h"
#include "modes_of_operation.h"
#include "relay_matrix.h"
#include "arm_book_lib.h"
#include <string>

systemStateMachine_t systemStateMachine;
static int submoduleQty = 0;
static int idOfSubmodule = 0;
static int submoduleCounter = 0;
static int rowPosition = 0;
static int colPosition = 1;
static bool changeOfSubmodules = false;

static void updateModules();
static void startSetUpModuleUpdate();
static void setRowPositionOfModuleUpdate();
static void setColPositionOfModuleUpdate();
static void endSetUpModuleUpdate();
static void principalMenuUpdate();
static void sequenceModeUpdate();
static void manualModeUpdate();
static bool checkChangeOfSubmodules();
static void handleChangeOfSubmodules();
static void stateTransition(systemStateMachine_t newState);

void switchingMatrixSystemInit() {
    i2cComInit();
    pcSerialComInit();
    relayMatrixInit();
    //eventLogInit();
    submoduleQty = i2cComGetSubmoduleQty();
    idOfSubmodule = i2cComGetIdOfFirstSubmodule();
    stateTransition(START_SET_UP_MODULE);
}

void switchingMatrixSystemUpdate() {
    if(checkChangeOfSubmodules() == true) {
        handleChangeOfSubmodules();
    }

    updateModules();

    switch(systemStateMachine) {
        case START_SET_UP_MODULE: startSetUpModuleUpdate();  break;
        case SET_ROW_POSITION_OF_MODULE: setRowPositionOfModuleUpdate(); break;
        case SET_COL_POSITION_OF_MODULE: setColPositionOfModuleUpdate(); break;
        case END_SET_UP_MODULE: endSetUpModuleUpdate(); break;
        case PRINCIPAL_MENU: principalMenuUpdate(); break;
        //case SEQUENCE_MODE: sequenceModeUpdate(); break;
        case MANUAL_MODE: manualModeUpdate(); break;
        default: break; 
    }
}


static void startSetUpModuleUpdate() {
    i2cComStartIdentificationOfSubmodule(idOfSubmodule);
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
    if(relayMatrixInsertModule(idOfSubmodule, rowPosition, colPosition++) == false) {
        pcSerialComStringWrite("La posición ingresada para ingresar el módulo es invalida!!");
        stateTransition(START_SET_UP_MODULE);
    }
    else if(submoduleCounter == submoduleQty - 1) {
        relayMatrixGenerate();
        relayMatrixPrintMatrix();
        stateTransition(MANUAL_MODE);
    }
    else {
        submoduleCounter++;
        idOfSubmodule++;
        stateTransition(START_SET_UP_MODULE);
    }
    i2cComStopIdentificationOfSubmodule(idOfSubmodule);
}

static void principalMenuUpdate() {
    modesOfOperation_t mode;
    if(pcSerialComPrincipalMenuUserSelection(mode) == true) {
        if(mode == MANUAL_OPERATION) {
            stateTransition(MANUAL_MODE);
        }
        else {
            stateTransition(SEQUENCE_MODE);
        }
    }
}

static void manualModeUpdate() {
    char id[2];
 //   if(getRelayIdToToggle(id)) {

   // }

   relayMatrixWriteStateOfRelay(5, ON);
   relayMatrixPrintMatrix();
   relayMatrixWriteStateOfRelay(9, ON);
   relayMatrixPrintMatrix();
   relayMatrixWriteStateOfRelay(5, OFF);
   relayMatrixPrintMatrix();
   relayMatrixWriteStateOfRelay(9, OFF);
}

static void stateTransition(systemStateMachine_t nextState) {
    systemStateMachine = nextState;
    pcSerialComSetSystemState(nextState);
}

static void updateModules() {
    pcSerialComUpdate();
    i2cComUpdate();
}

static bool checkChangeOfSubmodules() {
    return i2cComChangeInSubmodulesRecognized();
}

void handleChangeOfSubmodules() {

}
