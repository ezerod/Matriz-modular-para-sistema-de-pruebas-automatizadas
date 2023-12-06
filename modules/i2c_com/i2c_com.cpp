#include "i2c_com.h"
#include "mbed.h"
#include "non_blocking_delay.h"
#include "arm_book_lib.h"

#define I2C1_SDA PB_9
#define I2C1_SCL PB_8

#define MAX_QTY_OF_SUBMODULES 7
#define INITIAL_ADDRESS_PCF8574 64
#define FINAL_ADDRESS_PCF8574 78

#define MODULE_2X2_CONFIG 0b01110000
#define RELAY_ROW1_COL1 0b00000001
#define RELAY_ROW1_COL2 0b00000010
#define RELAY_ROW2_COL1 0b00000100
#define RELAY_ROW2_COL2 0b00001000
#define RELAY_ROW1_COL1_IN_MASK 0b00010000
#define RELAY_ROW1_COL2_IN_MASK 0b00100000
#define RELAY_ROW2_COL1_IN_MASK 0b01000000
#define BLINK_TIME_MS 500
//#define RELAY_ROW2_COL2_IN_MASK 0b10000000; NOT USED BECAUSE PIN QTY IS NOT SUFFICIENT
#define IDENTIFICATION_LED 0b10000000

typedef enum {
    IDENTIFYING_MODULE,
    NORMAL_FUNCTIONING,
    I2C_CHANGE_IN_MODULES_RECOGNIZED
}i2cModes_t;



typedef struct{
    int submoduleId;

    int address;
    char data;
    bool matrixPinRelay11Out;
    bool matrixPinRelay12Out; 
    bool matrixPinRelay21Out;
    bool matrixPinRelay22Out;
    bool matrixPinRelay11In;
    bool matrixPinRelay12In;
    bool matrixPinRelay21In;
    bool identificationLedOut;

    int relay11Id;

    int relay12Id;

    int relay21Id;

    int relay22Id;
    //bool matrixPinRelay22In;
} pcf8574RelayMatrix_t;

i2cModes_t i2cMode = NORMAL_FUNCTIONING;

I2C managerComI2C(I2C1_SDA, I2C1_SCL);

static pcf8574RelayMatrix_t submodulesRecognized[MAX_QTY_OF_SUBMODULES];

static int submoduleQty = 0;

static bool changeInSubmodule = false;

static bool identifyingModule = false;
static int submoduleRecognizedIndexToIdentify = 0;
nonBlockingDelay identificationBlinkyDelay(500);


static void writeStateOfRelayOnSubmodule(int indexOfSubmodule, int relayId, int state);
static void writeLedPin(int address, int state);
static void identifyingModuleUpdate();
static void normalFunctioningUpdate();
static void changeOfModulesDetectedUpdate();
static void inicializeSubmodules();
static void scanRelayModules();
static int readStateOfRelayOnSubmodule(int indexOfSubmodule, int relay);
static bool checkChangesOfSubmodulesRecognized();


int  i2cComGetSubmoduleQty() {
    return submoduleQty;
}

void i2cComSetRelay11Id(int submoduleId, int relayId) {
    bool taskCompleted = false;
    for(int i = 0; i < submoduleQty && taskCompleted == false; i++) {
        if(submodulesRecognized[i].submoduleId == submoduleId) {
            submodulesRecognized[i].relay11Id = relayId;
            taskCompleted = true;
        }
    }
}

void i2cComSetRelay12Id(int submoduleId, int relayId) {
    bool taskCompleted = false;
    for(int i = 0; i < submoduleQty && taskCompleted == false; i++) {
        if(submodulesRecognized[i].submoduleId == submoduleId) {
            submodulesRecognized[i].relay12Id = relayId;
            taskCompleted = true;
        }
    }
}

void i2cComSetRelay21Id(int submoduleId, int relayId) {
    bool taskCompleted = false;
    for(int i = 0; i < submoduleQty && taskCompleted == false; i++) {
        if(submodulesRecognized[i].submoduleId == submoduleId) {
            submodulesRecognized[i].relay21Id = relayId;
            taskCompleted = true;
        }
    }
}

void i2cComSetRelay22Id(int submoduleId, int relayId) {
    bool taskCompleted = false;
    for(int i = 0; i < submoduleQty && taskCompleted == false; i++) {
        if(submodulesRecognized[i].submoduleId == submoduleId) {
            submodulesRecognized[i].relay22Id = relayId;
            taskCompleted = true;
        }
    }
}

bool i2cComChangeInSubmodulesRecognized() {
    return changeInSubmodule;
}

void i2cComWriteStateOfRelay(int relayId, int state) {
    bool taskCompleted = false;

    for(int i = 0; i < submoduleQty && taskCompleted == false; i++) {
        if(relayId == submodulesRecognized[i].relay11Id) {
            writeStateOfRelayOnSubmodule(i, RELAY_ROW1_COL1, state);
            taskCompleted = true;
        }
        else if(relayId == submodulesRecognized[i].relay12Id) {
            writeStateOfRelayOnSubmodule(i, RELAY_ROW1_COL2, state);
            taskCompleted = true;
        }
        else if(relayId == submodulesRecognized[i].relay21Id) {
            writeStateOfRelayOnSubmodule(i, RELAY_ROW2_COL1, state);
            taskCompleted = true;
        }
        else if(relayId == submodulesRecognized[i].relay22Id) {
            writeStateOfRelayOnSubmodule(i, RELAY_ROW2_COL2, state);
            taskCompleted = true;
        }
    }
}

int i2cComReadStateOfRelay(int relayId) {
    bool taskCompleted = false;
    int valueRead;
    for(int i = 0; i < submoduleQty && taskCompleted == false; i++) {
        if(relayId == submodulesRecognized[i].relay11Id){
            valueRead = readStateOfRelayOnSubmodule(i, RELAY_ROW1_COL1);
            taskCompleted = true;
        }
        else if(relayId == submodulesRecognized[i].relay12Id) {
            valueRead = readStateOfRelayOnSubmodule(i, RELAY_ROW1_COL2);
            taskCompleted = true;
        }
        else if(relayId == submodulesRecognized[i].relay21Id) {
            valueRead = readStateOfRelayOnSubmodule(i, RELAY_ROW2_COL1);
            taskCompleted = true;
        }
        else if(relayId == submodulesRecognized[i].relay22Id) {
            readStateOfRelayOnSubmodule(i, RELAY_ROW2_COL2);
            taskCompleted = true;
        }
    }

    return valueRead;
}

int i2cComGetIdOfFirstSubmodule() {
    return submodulesRecognized[0].submoduleId;
}


static void scanRelayModules() {
    int i = INITIAL_ADDRESS_PCF8574;
    char testMessage = 0x00;
    int submoduleId = 2;
    while(i <= FINAL_ADDRESS_PCF8574) {
        if(managerComI2C.write(i, &testMessage, 1) == 0) {
            submodulesRecognized[submoduleQty].address = i;
            submodulesRecognized[submoduleQty].submoduleId = submoduleId;
            submoduleQty++;
            submoduleId++;
        }
        i = i + 2;
    }
}

static void inicializeSubmodules() {
    char configurationOfModule = MODULE_2X2_CONFIG;
    for(int i = 0; i < submoduleQty; i++) {
        writeStateOfRelayOnSubmodule(i, RELAY_ROW1_COL1, (int)(MODULE_2X2_CONFIG & RELAY_ROW1_COL1));
        writeStateOfRelayOnSubmodule(i, RELAY_ROW1_COL2, (int)(MODULE_2X2_CONFIG & RELAY_ROW1_COL2));
        writeStateOfRelayOnSubmodule(i, RELAY_ROW2_COL1, (int)(MODULE_2X2_CONFIG & RELAY_ROW2_COL1));
        writeStateOfRelayOnSubmodule(i, RELAY_ROW2_COL2, (int)(MODULE_2X2_CONFIG & RELAY_ROW2_COL2));
        writeLedPin(i, (int)(MODULE_2X2_CONFIG & IDENTIFICATION_LED));
    }
}

static void writeStateOfRelayOnSubmodule(int indexOfSubmodule, int relay, int state) {
    switch(relay) {
        case RELAY_ROW1_COL1: submodulesRecognized[indexOfSubmodule].matrixPinRelay11Out = state; break;
        case RELAY_ROW1_COL2: submodulesRecognized[indexOfSubmodule].matrixPinRelay12Out = state; break;
        case RELAY_ROW2_COL1: submodulesRecognized[indexOfSubmodule].matrixPinRelay21Out = state; break;
        case RELAY_ROW2_COL2: submodulesRecognized[indexOfSubmodule].matrixPinRelay22Out = state; break;
    }

    submodulesRecognized[indexOfSubmodule].data = 0b00000000;
    if ( submodulesRecognized[indexOfSubmodule].matrixPinRelay11Out ) submodulesRecognized[indexOfSubmodule].data |= 0b00000001;
    if ( submodulesRecognized[indexOfSubmodule].matrixPinRelay12Out ) submodulesRecognized[indexOfSubmodule].data |= 0b00000010; 
    if ( submodulesRecognized[indexOfSubmodule].matrixPinRelay21Out ) submodulesRecognized[indexOfSubmodule].data |= 0b00000100; 
    if ( submodulesRecognized[indexOfSubmodule].matrixPinRelay22Out ) submodulesRecognized[indexOfSubmodule].data |= 0b00001000;

    managerComI2C.write(submodulesRecognized[indexOfSubmodule].address, &submodulesRecognized[indexOfSubmodule].data, 1); 
}


static int readStateOfRelayOnSubmodule(int indexOfSubmodule, int relay) {
    int pinValueRead = -1;
    managerComI2C.read(submodulesRecognized[indexOfSubmodule].address, &submodulesRecognized[indexOfSubmodule].data, 1);
    switch(relay) {
        case RELAY_ROW1_COL1:
            if( (submodulesRecognized[indexOfSubmodule].data & (RELAY_ROW1_COL1_IN_MASK)) && (submodulesRecognized[indexOfSubmodule].data & (RELAY_ROW1_COL1)))
                pinValueRead = ON;
            else if((submodulesRecognized[indexOfSubmodule].data & (RELAY_ROW1_COL1_IN_MASK)) && (submodulesRecognized[indexOfSubmodule].data & (RELAY_ROW1_COL1)))
                pinValueRead = OFF;
            break;
        case RELAY_ROW1_COL2:
            if( (submodulesRecognized[indexOfSubmodule].data & (RELAY_ROW1_COL2_IN_MASK)) && (submodulesRecognized[indexOfSubmodule].data & (RELAY_ROW1_COL2)))
                pinValueRead = ON;
            else if((submodulesRecognized[indexOfSubmodule].data & (RELAY_ROW1_COL2_IN_MASK)) && (submodulesRecognized[indexOfSubmodule].data & (RELAY_ROW1_COL2)))
                pinValueRead = OFF;
            break;
        case RELAY_ROW2_COL1:
            if( (submodulesRecognized[indexOfSubmodule].data & (RELAY_ROW2_COL1_IN_MASK)) && (submodulesRecognized[indexOfSubmodule].data & (RELAY_ROW2_COL1)))
                pinValueRead = ON;
            else if((submodulesRecognized[indexOfSubmodule].data & (RELAY_ROW2_COL1_IN_MASK)) && (submodulesRecognized[indexOfSubmodule].data & (RELAY_ROW2_COL1)))
                pinValueRead = OFF;
            break;
        case RELAY_ROW2_COL2:
            if( (submodulesRecognized[indexOfSubmodule].data & (RELAY_ROW2_COL1_IN_MASK)))
                pinValueRead = ON;
            else
                pinValueRead = OFF;
            break;
    }

    return pinValueRead;
}

void i2cComInit() {
    managerComI2C.frequency(100000);
    scanRelayModules();
    inicializeSubmodules();
}

void i2cComUpdate() {
    if(i2cMode != I2C_CHANGE_IN_MODULES_RECOGNIZED) {
        if(checkChangesOfSubmodulesRecognized() == true) {
            changeInSubmodule = true;
            i2cMode = I2C_CHANGE_IN_MODULES_RECOGNIZED;
        }
        if(i2cMode == IDENTIFYING_MODULE)
            identifyingModuleUpdate();
    }
}

static void identifyingModuleUpdate() {
    if(identificationBlinkyDelay.read()) {
        if(submodulesRecognized[submoduleRecognizedIndexToIdentify].identificationLedOut)
            writeLedPin(submoduleRecognizedIndexToIdentify, OFF);
        else
            writeLedPin(submoduleRecognizedIndexToIdentify, ON);
    }
}

void i2cComStartIdentificationOfSubmodule(int submoduleId) {
    for(int i = 0; i < submoduleQty; i++) {
        if(submodulesRecognized[i].submoduleId == submoduleId) {
            submoduleRecognizedIndexToIdentify = i;
            printf("%d\n", submoduleRecognizedIndexToIdentify);
        }
    }
    i2cMode = IDENTIFYING_MODULE;
}

void i2cComStopIdentificationOfSubmodule(int submoduleId) {
    writeLedPin(submoduleRecognizedIndexToIdentify, OFF);
    i2cMode = NORMAL_FUNCTIONING;
}

static void writeLedPin(int address, int state) {
    submodulesRecognized[address].data = submodulesRecognized[address].data & (~IDENTIFICATION_LED);
    submodulesRecognized[address].identificationLedOut = false;
    if(state == ON) {
        submodulesRecognized[address].data = submodulesRecognized[address].data | IDENTIFICATION_LED;
        submodulesRecognized[address].identificationLedOut = true;
    }
    managerComI2C.write(submodulesRecognized[address].address, &submodulesRecognized[address].data, 1);
}

static void changeOfModulesDetectedUpdate() {
    changeInSubmodule = true;
}

static bool checkChangesOfSubmodulesRecognized() {
    int i = INITIAL_ADDRESS_PCF8574;
    int index = 0;
    char testMessage = 0;
    bool changeDetected = false;
    
    while(i <= FINAL_ADDRESS_PCF8574 && changeDetected == false) {
        if(managerComI2C.write(i, &testMessage, 1) == 0) {
            if(index < submoduleQty)
                if(submodulesRecognized[index].address != i)
                    changeDetected = true;
            
            index ++;
        }
        i = i + 2;
    }

    return changeDetected;
}