//=====[#include guards - begin]===============================================
#ifndef PC_SERIAL_COM__H
#define PC_SERIAL_COM__H

//=====[Libraries]===============================================
#include "state_machine.h"

//=====[Declaration of public defines]=========================================

//=====[Declaration of public data types]======================================

//=====[Declarations (prototypes) of public functions]=========================
void pcSerialComInit();
char pcSerialComCharRead();
void pcSerialComStringWrite( const char* str );
void pcSerialComUpdate();

void pcSerialComSetSystemState(systemStateMachine_t systemState);
bool pcSerialComGetPrincipalMenuOptions(char* option);
bool pcSerialComGetRow(char* rowPosition);
bool pcSerialComGetCol(char* colPosition);
bool pcSerialComGetRelayIdToToggle(char* relayToToggle);
bool pcSerialComGetTime(char* _time);
bool pcSerialComGetDate(char* date);
#endif