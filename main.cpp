#include "mbed.h"
#include "switching_matrix_system.h"
#include "arm_book_lib.h"
// main() runs in its own thread in the OS
int main()
{
    switchingMatrixSystemInit();
    while (true) {
        switchingMatrixSystemUpdate();
        delay(50);
    }
}

