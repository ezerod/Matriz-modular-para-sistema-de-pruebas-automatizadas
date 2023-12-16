//=====[#include guards - begin]===============================================
#ifndef RELAY__H
#define RELAY__H

//=====[Declaration of public defines]=========================================

//=====[Declaration of public data types]======================================

//=====[Declarations (prototypes) of public functions]=========================
void relayAddManagerIdOfRelay(int relayId);
void relayAddSubmoduleIdOfRelay(int submoduleId, int relayId);
void relayClearIdOfRelays();

bool relayClose(int relayId);
bool relayOpen(int relayId);
bool relayToggle(int relayId);
int relayReadState(int relayId);
void openRelaysOfManager();
#endif