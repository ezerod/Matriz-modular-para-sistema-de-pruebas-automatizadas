#include "relay_matrix.h"
#include "i2c_com.h"
#include "pc_serial_com.h"
#include "arm_book_lib.h"
#include <string>
#include <vector>
#include "mbed.h"

#define MAX_QTY_OF_MODULES 8
#define MAX_ROWS MAX_QTY_OF_MODULES
#define MAX_COLUMNS MAX_QTY_OF_MODULES
#define MAX_ROWS_OF_RELAY_MATRIX        2 * MAX_QTY_OF_MODULES
#define MAX_COLUMNS_OF_RELAY_MATRIX     2 * MAX_QTY_OF_MODULES



static bool hasModule(int row, int column);
bool addWithNeighbor(int row, int column, int submoduleId);

static vector< vector<int> > matrixOfModules;
static vector< vector<int> > relayMatrix;


static bool hasModule(int row, int column) {
    // Check if the given position is within the bounds of the matrix
    if (row >= 0 && row < matrixOfModules.size() && column >= 0 && column < matrixOfModules[row].size()) {
        // Check if the module ID at the specified position is not zero
        return matrixOfModules[row][column] != 0;
    }
    return false;
}

void relayMatrixInit() {
    vector<int> aux;
    aux.push_back(1);
    matrixOfModules.push_back(aux);
}

bool addWithNeighbor(int row, int column, int submoduleId) {
    // Ensure that the row is within the current matrix size
    if (row < 0) {
        return false;
    }

    // Ensure that the column is within the current row size
    if (column < 0) {
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

    // Check left
    if (column > 0 && hasModule(row, column - 1)) {
        matrixOfModules[row][column] = submoduleId;
        return true;
    }

    // Check below
    if (row < matrixOfModules.size() - 1 && hasModule(row + 1, column)) {
        matrixOfModules[row][column] = submoduleId;
        return true;
    }

    // Check right
    if (column < matrixOfModules[row].size() - 1 && hasModule(row, column + 1)) {
        matrixOfModules[row][column + 1] = submoduleId;
        return true;
    }

    // Check above
    if (row > 0 && hasModule(row - 1, column)) {
        matrixOfModules[row][column] = submoduleId;
        return true;
    }

    // No neighboring module found
    return false;
}

// Function to generate the relay matrix from the module matrix
void relayMatrixGenerate() {
    relayMatrix.clear();
    vector<int> auxRow1;
    vector<int> auxRow2;
    int idToAssign = 1;

    for(int i = 0; i < matrixOfModules.size(); i++) {
        for(int j = 0; j < matrixOfModules[i].size(); j++) {
            if(matrixOfModules[i][j] != 0) {
                
                auxRow1.push_back({idToAssign, idToAssign + 1});
                auxRow2.push_back({idToAssign + 2, idToAssign + 3});
                if(matrixOfModules[i][j] != MANAGER_MODULE_ID) {
                    i2cComSetRelay11Id(matrixOfModules[i][j], idToAssign);
                    i2cComSetRelay12Id(matrixOfModules[i][j], idToAssign + 1);
                    i2cComSetRelay21Id(matrixOfModules[i][j], idToAssign + 2);
                    i2cComSetRelay22Id(matrixOfModules[i][j], idToAssign + 3);
                }
                else {
                    managerModuleSetRelay11Id(idToAssign);
                    managerModuleSetRelay12Id(idToAssign);
                    managerModuleSetRelay11Id(idToAssign);
                    managerModuleSetRelay11Id(idToAssign);
                }
                idToAssign += 4;
            }
            else {
                auxRow1.push_back({0, 0});
                auxRow2.push_back({0, 0});
            }
        }

        relayMatrix.push_back(auxRow1);
        relayMatrix.push_back(auxRow2);
        auxRow1.clear();
        auxRow2.clear();
    }
    
    }
}
void relayMatrixPrintMatrix() {
    char actualRow[MAX_COLUMNS_OF_RELAY_MATRIX + 1];
    for(int i = 0; i < relayMatrix.size(); i++) {
        for(int j = 0; j < relayMatrix[i].size(); j++) {
            if(relayMatrix[i][j] != 0) {
                if(i2cComReadStateOfRelay(relayMatrix[i][j]) == ON) {
                    actualRow[j] = 'X';
                }
                else {
                    actualRow[j] = 'O';
                }
            }
            else {
                actualRow[j] = '\t';
            }
        }
        
        actualRow[MAX_COLUMNS_OF_RELAY_MATRIX] = '\0';
        pcSerialComStringWrite(actualRow);
        pcSerialComStringWrite("\n");
    }
}

void relayMatrixCloseRelay(int row, int col) {
    if(relayMatrix.size() > row && relayMatrix[row].size() > col) {
        i2cComWriteStateOfRelay(relayMatrix[row][col], ON);
    }
}

bool relayMatrixInsertModule(int submoduleId, int row, int col) {
    bool moduleAdded = addWithNeighbor(row, col, submoduleId);
    if(moduleAdded) {
        printf("%s\n", "se añadió el modulo");
    } else {
        printf("%s\n", "se añadió el modulo");
    }
    return moduleAdded;
}