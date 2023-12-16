//=====[#include guards - begin]===============================================

#ifndef _SD_CARD_H_
#define _SD_CARD_H_

#include "mbed.h"
//=====[Declaration of public defines]=========================================

#define SD_CARD_MAX_FILE_LIST       10
#define SD_CARD_FILENAME_MAX_LENGTH 32

//=====[Declaration of public data types]======================================

//=====[Declarations (prototypes) of public functions]=========================

bool sdCardInit();
bool sdCardWriteFile( FILE* sdCardFilePointer, const char* writeBuffer );


//=====[#include guards - end]=================================================

#endif // _SD_CARD_H_