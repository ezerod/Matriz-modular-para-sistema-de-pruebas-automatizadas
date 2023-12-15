#include "relay.h"
#include "mbed.h"
#include "i2c_com.h"
#include "arm_book_lib.h"
#include <vector>

#define MANAGER_RELAY11_PIN D7
#define MANAGER_RELAY12_PIN D6
#define MANAGER_RELAY21_PIN D5
#define MANAGER_RELAY22_PIN D4


static vector<int> managerIds;
static vector<int> submoduleIds;

DigitalOut managerRelay11(MANAGER_RELAY11_PIN);
DigitalOut managerRelay12(MANAGER_RELAY12_PIN);
DigitalOut managerRelay21(MANAGER_RELAY21_PIN);
DigitalOut managerRelay22(MANAGER_RELAY22_PIN);

static int readStateOfRelayOfManager(int index);
static void writeStateOfRelayOfManager(int index, int state);

void relayAddManagerIdOfRelay(int relayId) {
    managerIds.push_back(relayId);
}

void relayAddSubmoduleIdOfRelay(int submoduleId, int relayId) {
    static int currentRelay = 1;
    static int previousSubmoduleId = -1;
    if(submoduleId != previousSubmoduleId) {
        currentRelay = 1;
        previousSubmoduleId = submoduleId;
    }
    switch(currentRelay) {
            case 1: i2cComSetRelay11Id(submoduleId, relayId); currentRelay++; break;
            case 2: i2cComSetRelay12Id(submoduleId, relayId); currentRelay++; break;
            case 3: i2cComSetRelay21Id(submoduleId, relayId); currentRelay++; break;
            case 4: i2cComSetRelay22Id(submoduleId, relayId); currentRelay++; break;
            default: break;
        }
    submoduleIds.push_back(relayId);
}

void relayClearIdOfRelays() {
    managerIds.clear();
    submoduleIds.clear();
}

bool relayClose(int relayId) {
    bool relayClosed = false;
    
    for(int i = 0; i < managerIds.size(); i++) {
        if(relayId == managerIds[i]) {
            writeStateOfRelayOfManager(i, ON);
            relayClosed = true;
        }
    }

    if(! relayClosed) {
        for(int i = 0; i < submoduleIds.size(); i++) {
            if(relayId == submoduleIds[i]) {
                i2cComWriteStateOfRelay(relayId, ON);
                relayClosed = true;
            }
        }   
    }

    return relayClosed;
}

bool relayOpen(int relayId) {
    bool relayOpened = false;

    for(int i = 0; i < managerIds.size(); i++) {
        if(relayId == managerIds[i]) {
            writeStateOfRelayOfManager(i, OFF);
            relayOpened = true;
        }
    }
    if(! relayOpened) {
        for(int i = 0; i < submoduleIds.size(); i++) {
            if(relayId == submoduleIds[i]) {
                i2cComWriteStateOfRelay(relayId, OFF);
                relayOpened = true;
            }
        }   
    }

    return relayOpened;
}

int relayReadState(int relayId) {
    bool relayStateRead = false;
    int stateOfRelay = -1;

    for(int i = 0; i < managerIds.size(); i++) {
        if(relayId == managerIds[i]) {
            stateOfRelay = readStateOfRelayOfManager(i);
            relayStateRead = true;
        }
    }

    if(! relayStateRead) {
        for(int i = 0; i < submoduleIds.size(); i++) {
            if(relayId == submoduleIds[i]) {
                stateOfRelay = i2cComReadStateOfRelay(relayId);
                relayStateRead = true;
            }
        }   
    }

    return stateOfRelay;
}

bool relayToggle(int relayId) {
    bool relayToggled = false;
    int state = relayReadState(relayId);

    if(state == OFF)
        relayToggled = relayClose(relayId);
    else if(state == ON) 
        relayToggled = relayOpen(relayId);
    
    return relayToggled;
}

static int readStateOfRelayOfManager(int index) {
    int stateRead = -1;
    switch(index) {
        case 0: stateRead = managerRelay11.read(); break;
        case 1: stateRead = managerRelay12.read(); break;
        case 2: stateRead = managerRelay21.read(); break;
        case 3: stateRead = managerRelay22.read(); break;
        default: stateRead = -1; break;
    }

    return stateRead;
}

static void writeStateOfRelayOfManager(int index, int state) {
    switch(index) {
        case 0: managerRelay11.write(state); break;
        case 1: managerRelay12.write(state); break;
        case 2: managerRelay21.write(state); break;
        case 3: managerRelay22.write(state); break;
        default: break;
    }
}

void openRelaysOfManager() {
    managerRelay11.write(OFF);
    managerRelay12.write(OFF);
    managerRelay21.write(OFF);
    managerRelay22.write(OFF);
}