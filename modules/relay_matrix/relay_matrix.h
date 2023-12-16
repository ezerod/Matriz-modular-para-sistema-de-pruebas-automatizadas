#ifndef RELAY_MATRIX__H
#define RELAY_MATRIX__H

void relayMatrixInit();
void relayMatrixUpdate(bool changeInSubmodules);
void relayMatrixGenerate();
bool relayMatrixInsertModule(int submoduleId, int row, int col);
void relayMatrixPrintMatrix();
bool relayMatrixToggleRelay(int relayId);
int relayMatrixReadStateOfRelay(int relayId);
bool relayMatrixWriteStateOfRelay(int relayId, int state);
void relayMatrixClearMatrixes();
#endif