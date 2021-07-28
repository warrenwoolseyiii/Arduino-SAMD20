#include "I2C.h"

I2C::I2C( SERCOM *pSercom, int pinSDA, int pinSCL )
{
    _SDA = pinSDA;
    _SCL = pinSCL;
    _pSercom = pSercom;
}

void I2C::InitMaster( bool fastMode )
{
    pinMode( _SDA, gArduinoPins[_SDA].i2c );
    pinMode( _SCL, gArduinoPins[_SCL].i2c );
    _pSercom->initMasterWIRE( fastMode );
}

int I2C::MasterWrite( uint8_t addr, uint8_t *data, int len, bool stop )
{
    // Start the write
    int status = _pSercom->startTransmissionWIRE( addr, true );

    // If we have no errors proceed
    if( status == I2CM_ERR_NONE )
        status = _pSercom->sendDataMasterWIRE( data, len, stop );

    return status;
}

int I2C::MasterRead( uint8_t addr, uint8_t *data, int len, bool ack, bool stop )
{
    // Start the read
    int status = _pSercom->startTransmissionWIRE( addr, false );

    // If we have no errors proceed
    if( status == I2CM_ERR_NONE )
        status = _pSercom->readDataMasterWire( data, len, ack, stop );

    return status;
}

int I2C::MasterStartTransac( uint8_t addr, bool isWrite )
{
    return _pSercom->startTransmissionWIRE( addr, isWrite );
}

int I2C::MasterSendBytes( uint8_t *data, int len, bool stop )
{
    return _pSercom->sendDataMasterWIRE( data, len, stop );
}

int I2C::MasterReceiveBytes( uint8_t *data, int len, bool ack, bool stop )
{
    return _pSercom->readDataMasterWire( data, len, ack, stop );
}

void I2C::ResolveError( int errorCode )
{
    switch( errorCode ) {
        case I2CM_ERR_BUS_BUSY: ClearBusBusyError(); break;
        case I2CM_ERR_CONDITION: ClearBusErrorCondition(); break;
        case I2CM_ERR_LOW_TIMEOUT: ClearLowTimout(); break;
        case I2CM_ERR_RX_NACK: ClearRXNack(); break;
    }
}

void I2C::ClearBusBusyError()
{
    // Bus busy should be cleared on a stop condition
    _pSercom->prepareMasterCommandWIRE( 3 );
}

void I2C::ClearBusErrorCondition()
{
    // Clear the error bits and force the bus state to idle
    _pSercom->writeStatusWire( SERCOM_I2CM_STATUS_BUSERR |
                               SERCOM_I2CM_STATUS_ARBLOST |
                               SERCOM_I2CM_STATUS_BUSSTATE( 1 ) );
}

void I2C::ClearLowTimout()
{
    // Clear the clock time out by sending a stop condition
    _pSercom->prepareMasterCommandWIRE( 3 );

    // Clear the LOWTOUT bit
    _pSercom->writeStatusWire( SERCOM_I2CM_STATUS_LOWTOUT );
}

void I2C::ClearRXNack()
{
    _pSercom->prepareMasterCommandWIRE( 3 );
}

void I2C::ForceResetBus()
{
    _pSercom->resetWIRE();
}

void I2C::End()
{
    _pSercom->disableWIRE();
}

I2C TwoWire( &PERIPH_WIRE, PIN_WIRE_SDA, PIN_WIRE_SCL );
