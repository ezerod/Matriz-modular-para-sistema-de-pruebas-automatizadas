#include "relay_matrix.h"
#include "i2c_com.h"
#include "pc_serial_com.h"
#include "arm_book_lib.h"
#include <string>
#include <vector>
#include "mbed.h"


#define MANAGER_MODULE_ID 1
#define MANAGER_RELAY11_PIN D7
#define MANAGER_RELAY12_PIN D6
#define MANAGER_RELAY21_PIN D5
#define MANAGER_RELAY22_PIN D4


typedef struct {
    int relay11Id;
    int relay12Id;
    int relay21Id;
    int relay22Id;    
} managerRelaysId_t;

typedef vector<vector<int>> Matrix;

DigitalOut ManagerRelay11(MANAGER_RELAY11_PIN);
DigitalOut ManagerRelay12(MANAGER_RELAY12_PIN);
DigitalOut ManagerRelay21(MANAGER_RELAY21_PIN);
DigitalOut ManagerRelay22(MANAGER_RELAY22_PIN);


static bool hasModule(int row, int column);
static bool addWithNeighbor(int row, int column, int submoduleId);
static void managerModuleSetRelay11Id(int idToAssign);
static void managerModuleSetRelay12Id(int idToAssign);
static void managerModuleSetRelay21Id(int idToAssign);
static void managerModuleSetRelay22Id(int idToAssign);
static void managerWriteStateOfRelay(int relayId, int state);

static std::vector< std::vector<int> > matrixOfModules;
static std::vector< std::vector<int> > relayMatrix;
static vector<Matrix> predefinedSequence;
static managerRelaysId_t managerRelaysId;



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
    aux.push_back(MANAGER_MODULE_ID);
    matrixOfModules.push_back(aux);
}

static bool addWithNeighbor(int row, int column, int submoduleId) {
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
    vector<int> auxRow1;  // Cambiado a vector<int>
    vector<int> auxRow2;  // Cambiado a vector<int>
    int idToAssign = 1;

    for (int i = 0; i < matrixOfModules.size(); i++) {
        for (int j = 0; j < matrixOfModules[i].size(); j++) {
            if (matrixOfModules[i][j] != 0) {
                auxRow1.push_back(idToAssign);
                auxRow1.push_back(idToAssign + 1);
                auxRow2.push_back(idToAssign + 2);
                auxRow2.push_back(idToAssign + 3);
                
                if (matrixOfModules[i][j] != MANAGER_MODULE_ID) {
                    i2cComSetRelay11Id(matrixOfModules[i][j], idToAssign);
                    i2cComSetRelay12Id(matrixOfModules[i][j], idToAssign + 1);
                    i2cComSetRelay21Id(matrixOfModules[i][j], idToAssign + 2);
                    i2cComSetRelay22Id(matrixOfModules[i][j], idToAssign + 3);
                } else {
                    managerModuleSetRelay11Id(idToAssign);
                    managerModuleSetRelay12Id(idToAssign + 1);
                    managerModuleSetRelay21Id(idToAssign + 2);
                    managerModuleSetRelay22Id(idToAssign + 3);
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
            if (i2cComReadStateOfRelay(relayMatrix[i][j]) == ON) {
                
                pcSerialComStringWrite("CLOSED");
            } else {
                pcSerialComStringWrite(" OPEN  ");
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

void relayMatrixWriteStateOfRelay(int relayId, int state) {
    if(relayId != managerRelaysId.relay11Id && relayId != managerRelaysId.relay12Id &&
       relayId != managerRelaysId.relay21Id && relayId != managerRelaysId.relay22Id)
        i2cComWriteStateOfRelay(relayId, state);
    else {
        managerWriteStateOfRelay(relayId, state);
    }
}

bool relayMatrixInsertModule(int submoduleId, int row, int col) {
    bool moduleAdded = addWithNeighbor(row, col, submoduleId);
    if(moduleAdded) {
        printf("%s\n", "se añadió el modulo");
    } else {
        printf("%s\n", "no se añadió el modulo");
    }
    return moduleAdded;
}

static void managerModuleSetRelay11Id(int idToAssign) {
    managerRelaysId.relay11Id = idToAssign;
}

static void managerModuleSetRelay12Id(int idToAssign) {
    managerRelaysId.relay12Id = idToAssign;
}

static void managerModuleSetRelay21Id(int idToAssign) {
    managerRelaysId.relay21Id = idToAssign;
}

static void managerModuleSetRelay22Id(int idToAssign) {
    managerRelaysId.relay22Id = idToAssign;
}

static void managerWriteStateOfRelay(int relayId, int state) {
    if(relayId == managerRelaysId.relay11Id)
        ManagerRelay11.write(state);
    else if(relayId == managerRelaysId.relay12Id)
        ManagerRelay12.write(state);
    else if(relayId == managerRelaysId.relay21Id)
        ManagerRelay21.write(state);
    else if(relayId == managerRelaysId.relay22Id)
        ManagerRelay22.write(state);
}