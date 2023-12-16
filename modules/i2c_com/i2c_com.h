//=====[#include guards - begin]===============================================
#ifndef I2C_COM__H
#define I2C_COM__H

//=====[Declaration of public defines]=========================================

//=====[Declaration of public data types]======================================

//=====[Declarations (prototypes) of public functions]=========================
void i2cComInit();
void i2cComUpdate();

int  i2cComGetSubmoduleQty();
int  i2cComGetIdOfFirstSubmodule();
bool i2cComChangeInSubmodulesRecognized();
void i2cComStartIdentificationOfSubmodule(int submoduleId);
void i2cComStopIdentificationOfSubmodule(int submoduleId);
void i2cComChangeInSubmodulesHandled();

void i2cComSetRelay11Id(int submoduleId, int relayId);
void i2cComSetRelay12Id(int submoduleId, int relayId);
void i2cComSetRelay21Id(int submoduleId, int relayId);
void i2cComSetRelay22Id(int submoduleId, int relayId);
void i2cComWriteStateOfRelay(int relayId, int state);
int  i2cComReadStateOfRelay(int relayId);

#endif