#ifndef RELAY_MATRIX__H
#define RELAY_MATRIX__H

void relayMatrixInit();
void relayMatrixUpdate();
void relayMatrixGenerate();
bool relayMatrixInsertModule(int submoduleId, int row, int col);
void relayMatrixWriteStateOfRelay(int relayId, int state);
void relayMatrixPrintMatrix();

#endif