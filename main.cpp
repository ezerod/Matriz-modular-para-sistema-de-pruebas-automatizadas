#include "mbed.h"
#include "switching_matrix_system.h"
int main()
{
    switchingMatrixSystemInit();
    while (true) {
        switchingMatrixSystemUpdate();
    }
}

