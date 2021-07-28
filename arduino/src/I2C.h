#ifndef I2C_H_
#define I2C_H_

#include <Arduino.h>

class I2C
{
  public:
    I2C( SERCOM *p_sercom, int pinSDA, int pinSCL );
    void InitMaster( bool fastMode );
    int  MasterWrite( uint8_t addr, uint8_t *data, int len, bool stop );
    int MasterRead( uint8_t addr, uint8_t *data, int len, bool ack, bool stop );
    int MasterStartTransac( uint8_t addr, bool isWrite );
    int MasterSendBytes( uint8_t *data, int len, bool stop );
    int MasterReceiveBytes( uint8_t *data, int len, bool ack, bool stop );
    void ResolveError( int errorCode );
    void ClearBusBusyError();
    void ClearBusErrorCondition();
    void ClearLowTimout();
    void ClearRXNack();
    void ForceResetBus();
    void End();

  private:
    SERCOM *_pSercom;
    int     _SDA, _SCL;
};

extern I2C TwoWire;

#endif /* I2C_H_ */
