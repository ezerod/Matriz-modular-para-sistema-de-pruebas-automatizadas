//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"

#include "sd_card.h"

#include "event_log.h"
#include "date_and_time.h"
#include "pc_serial_com.h"

#include "FATFileSystem.h"
#include "SDBlockDevice.h"
#include <string>
#include "platform/mbed_retarget.h"

//=====[Declaration of private defines]========================================

#define SPI3_MOSI   PC_12
#define SPI3_MISO   PC_11
#define SPI3_SCK    PC_10
#define SPI3_CS     PA_4_ALT0

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

SDBlockDevice sd(SPI3_MOSI, SPI3_MISO , SPI3_SCK, SPI3_CS);

FATFileSystem sdCardFileSystem("sd", &sd);
//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============

//=====[Declarations (prototypes) of private functions]========================


//=====[Implementations of public functions]===================================

bool sdCardInit()
{
    pcSerialComStringWrite("---------------------------------------------------------------\n");
    pcSerialComStringWrite("Looking for a filesystem in the SD card... \r\n");
    sdCardFileSystem.mount(&sd);
    DIR *sdCardListOfDirectories = opendir("/sd/");
    if ( sdCardListOfDirectories != NULL ) {
        pcSerialComStringWrite("Filesystem found in the SD card. \r\n");
        pcSerialComStringWrite("---------------------------------------------------------------\n");
        closedir(sdCardListOfDirectories);
        return true;
    } else {
        pcSerialComStringWrite("Filesystem not mounted. \r\n");
        pcSerialComStringWrite("Insert an SD card and ");
        pcSerialComStringWrite("reset the NUCLEO board.\r\n");
        pcSerialComStringWrite("---------------------------------------------------------------\n");
        return false;
    }
}

bool sdCardWriteFile( FILE* sdCardFilePointer, const char* writeBuffer )
{
    if ( sdCardFilePointer != NULL ) {
        fprintf( sdCardFilePointer, "%s", writeBuffer );                       
        return true;
    } else {
        return false;
    }
}