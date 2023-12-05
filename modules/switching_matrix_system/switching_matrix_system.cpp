#include "switching_matrix_system.h"
#include "pc_serial_com.h"
#include "i2c_com.h"
#include "state_machine.h"
#include "non_blocking_delay.h"
#include "mbed.h"
#include "modes_of_operation.h"
#include "relay_matrix.h"
#include <string>

systemStateMachine_t systemStateMachine;
static int submoduleQty = 0;
static int idOfSubmodule = 0;
static int submoduleCounter = 0;
static int rowPosition = 0;
static int colPosition = 0;
static bool changeOfSubmodules = false;

static void updateModules();
static void startSetUpModuleUpdate();
static void setPositionOfModuleUpdate();
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
    printf("%d\n", submoduleQty);
    idOfSubmodule = i2cComGetIdOfFirstSubmodule();
    printf("%d\n", idOfSubmodule);
    stateTransition(START_SET_UP_MODULE);
    printf("%s\n", "arranco");
}

void switchingMatrixSystemUpdate() {
    if(checkChangeOfSubmodules() == true) {
        handleChangeOfSubmodules();
    }

    updateModules();

    switch(systemStateMachine) {
        case START_SET_UP_MODULE: startSetUpModuleUpdate();  break;
        case SET_POSITION_OF_MODULE: setPositionOfModuleUpdate(); break;
        case END_SET_UP_MODULE: endSetUpModuleUpdate(); break;
        case PRINCIPAL_MENU: principalMenuUpdate(); break;
        //case SEQUENCE_MODE: sequenceModeUpdate(); break;
        case MANUAL_MODE: manualModeUpdate(); break;
        default: break; 
    }
}


static void startSetUpModuleUpdate() {
    char** validOptions;
    int optionQty = 0;
    i2cComStartIdentificationOfSubmodule(idOfSubmodule);
    stateTransition(SET_POSITION_OF_MODULE);
}

static void setPositionOfModuleUpdate() {
    stateTransition(END_SET_UP_MODULE);
    
}

static void endSetUpModuleUpdate() {
    static int i = 0;
    relayMatrixInsertModule(idOfSubmodule, 1 , i);
    i++;
    if(submoduleCounter == submoduleQty - 1) {
        relayMatrixGenerate();
        relayMatrixPrintMatrix();
        stateTransition(PRINCIPAL_MENU);
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
