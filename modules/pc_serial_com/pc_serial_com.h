#ifndef PC_SERIAL_COM__H
#define PC_SERIAL_COM__H
#include "state_machine.h"
#include "modes_of_operation.h"

void pcSerialComInit();
char pcSerialComCharRead();
void pcSerialComStringWrite( const char* str );
void pcSerialComUpdate();
void pcSerialComSetSystemState(systemStateMachine_t systemState);
bool pcSerialComPrincipalMenuUserSelection(modesOfOperation_t &modeSelect);
bool pcSerialComGetRow(char* rowPosition);
bool pcSerialComGetCol(char* colPosition);
bool pcSerialComGetRelayIdToToggle(char* relayToToggle);

#endif