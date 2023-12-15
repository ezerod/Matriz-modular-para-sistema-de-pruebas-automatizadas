//=====[Libraries]=============================================================
#include "relay_matrix.h"
#include "pc_serial_com.h"
#include "arm_book_lib.h"
#include <string>
#include <vector>
#include "mbed.h"
#include "relay.h"

//=====[Declaration of private defines]========================================
#define MANAGER_MODULE_ID 1

//=====[Declaration of private data types]=====================================
typedef vector<vector<int>> Matrix;

//=====[Declaration and initialization of public global objects]===============
//=====[Declaration of external public global variables]=======================
//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============
static Matrix matrixOfModules;
static Matrix relayMatrix;

//=====[Declarations (prototypes) of private functions]========================
static bool hasModule(int row, int column);
static bool addWithNeighbor(int row, int column, int submoduleId);
static void initializeManagerModule();


//=====[Implementations of public functions]===================================
void relayMatrixInit() {
    initializeManagerModule();
}

void relayMatrixUpdate(bool changeInSubmodules) {
    if(changeInSubmodules) {
        relayMatrixClearMatrixes();
    }
}

void relayMatrixClearMatrixes() {
    matrixOfModules.clear();
    relayMatrix.clear();
    
    relayClearIdOfRelays();
    initializeManagerModule();
}

bool relayMatrixInsertModule(int submoduleId, int row, int col) {
    bool moduleAdded = addWithNeighbor(row, col, submoduleId);
    return moduleAdded;
}

void relayMatrixGenerate() {
    relayMatrix.clear();
    vector<int> auxRow1;
    vector<int> auxRow2; 
    int idToAssign = 1;

    for (int i = 0; i < matrixOfModules.size(); i++) {
        for (int j = 0; j < matrixOfModules[i].size(); j++) {
            if (matrixOfModules[i][j] != 0) {
                auxRow1.push_back(idToAssign);
                auxRow1.push_back(idToAssign + 1);
                auxRow2.push_back(idToAssign + 2);
                auxRow2.push_back(idToAssign + 3);
                
                if (matrixOfModules[i][j] != MANAGER_MODULE_ID) {
                    relayAddSubmoduleIdOfRelay(matrixOfModules[i][j], idToAssign);
                    relayAddSubmoduleIdOfRelay(matrixOfModules[i][j], idToAssign + 1);
                    relayAddSubmoduleIdOfRelay(matrixOfModules[i][j], idToAssign + 2);
                    relayAddSubmoduleIdOfRelay(matrixOfModules[i][j], idToAssign + 3);
                } else {
                    relayAddManagerIdOfRelay(idToAssign);
                    relayAddManagerIdOfRelay(idToAssign + 1);
                    relayAddManagerIdOfRelay(idToAssign + 2);
                    relayAddManagerIdOfRelay(idToAssign + 3);
                }
                idToAssign += 4;
            } else {
                auxRow1.push_back(0);
                auxRow1.push_back(0);
                auxRow2.push_back(0);
                auxRow2.push_back(0);
            }
        }

        relayMatrix.push_back(auxRow1);
        relayMatrix.push_back(auxRow2);
        auxRow1.clear();
        auxRow2.clear();
    }
}

void relayMatrixPrintMatrix() {
    char buffer[100];
    int stateOfRelay;

    for (int i = 0; i < relayMatrix.size(); i++) {
        for (int j = 0; j < relayMatrix[i].size(); j++) {
            pcSerialComStringWrite("---------    ");
        }
        pcSerialComStringWrite("\n");

        for (int j = 0; j < relayMatrix[i].size(); j++) {
            pcSerialComStringWrite(" Relay ");
            sprintf(buffer, "(%d)   ", relayMatrix[i][j]);
            pcSerialComStringWrite(buffer);
        }
        pcSerialComStringWrite("\n");

        for (int j = 0; j < relayMatrix[i].size(); j++) {
            pcSerialComStringWrite("    ");
            stateOfRelay = relayReadState(relayMatrix[i][j]);

            if (stateOfRelay == ON) {
                pcSerialComStringWrite("CLOSED");
            } else if(stateOfRelay == OFF) {
                pcSerialComStringWrite(" OPEN  ");
            }
            else {
                pcSerialComStringWrite("UNKNOWN");
            }

            pcSerialComStringWrite("  ");
        }
        pcSerialComStringWrite("\n");
        for (int j = 0; j < relayMatrix[i].size(); j++) {
            pcSerialComStringWrite("---------    ");
        }
        pcSerialComStringWrite("\n");
    }
}

bool relayMatrixToggleRelay(int relayId) {
    return relayToggle(relayId);
}

bool relayMatrixWriteStateOfRelay(int relayId, int state) {
    bool relayStateWritten;
    if(state == ON) {
        relayStateWritten = relayClose(relayId);
    }
    else {
        relayStateWritten = relayOpen(relayId);
    }

    return relayStateWritten;
}

int relayMatrixReadStateOfRelay(int relayId) {
    return relayReadState(relayId);
}

//=====[Implementations of private functions]===================================
static bool addWithNeighbor(int row, int column, int submoduleId) {
    if (row < 0 || column < 0) {
        return false;
    }

    // Adjust the row size if needed
    if (row >= matrixOfModules.size()) {
        matrixOfModules.resize(row + 1, std::vector<int>(0, 0));
    }

    // Adjust the column size if needed for the specified row
    if (column >= matrixOfModules[row].size()) {
        matrixOfModules[row].resize(column + 1, 0);
    }

    // Check if the position is already occupied
    if (matrixOfModules[row][column] != 0) {
        return false;
    }

    // Check directions (left, below, right and top)
    if (column > 0 && hasModule(row, column - 1)) {
        matrixOfModules[row][column] = submoduleId;
        return true;
    }

    if (row < matrixOfModules.size() - 1 && hasModule(row + 1, column)) {
        matrixOfModules[row][column] = submoduleId;
        return true;
    }

    if (column < matrixOfModules[row].size() - 1 && hasModule(row, column + 1)) {
        matrixOfModules[row][column + 1] = submoduleId;
        return true;
    }

    if (row > 0 && hasModule(row - 1, column)) {
        matrixOfModules[row][column] = submoduleId;
        return true;
    }

    // No neighboring module found
    return false;
}

static void initializeManagerModule() {
    vector<int> aux;
    aux.push_back(MANAGER_MODULE_ID);
    matrixOfModules.push_back(aux);

    openRelaysOfManager();
}

static bool hasModule(int row, int column) {
    return row >= 0 && row < matrixOfModules.size() && column >= 0 && column < matrixOfModules[row].size() && matrixOfModules[row][column] != 0;
}

