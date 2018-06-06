#include <Arduino.h>
#include <Debug.h>
#include <FXOS8700.h>
#include <PacketBuilder.h>
#include <SPIFlash.h>

uint8_t gLEDLevels = 0x1;

// PacketBuilder / Parser
Buffer_t gTxBuff, gRxBuff;
uint8_t gTypeFlags = 0;
uint32_t    gSendPeaksGlobal;
const char *baked_sha20 = "12345678901234567890";

// SPIFlash
SPIFlash flash(FLASH_SS);
NetInfo infoWrite, infoRead;

void setup() {
    DEBUGbegin( SERIAL_BAUD );
    
    pinMode( LED1, OUTPUT );
    pinMode( LED2, OUTPUT );
    
    pinMode( RFM_SS, OUTPUT );
    digitalWrite( RFM_SS, HIGH );
    pinMode( FLASH_SS, OUTPUT );
    
    PacketBuilder::InitPacketBuilder( &gTypeFlags, &gTxBuff );
    
    // Test this
    flash.initialize();
    
    //NetInfo infoWrite;
    infoWrite.erase();
    infoWrite.awake = 69;
    infoWrite.fastUpdates = 1;
    infoWrite.network = 0xF165;
    for( uint8_t i = 0; i < 16; i++ ) infoWrite.key[i] = i;
    infoWrite.save();
    
    //NetInfo infoRead;
    infoRead.load();
}

void loop() {
    // Blink this so I know it's alive
    delay( 250 );
    digitalWrite( LED1, gLEDLevels );
    digitalWrite( LED2, gLEDLevels );
    
    fxos.initialize();
    
    if( infoRead.network == infoWrite.network )
        gLEDLevels ^= 0x1;
        
    DEBUG_( "Time : " );
    DEBUGln( millis() );    
    DEBUG_( "Temp : " );
    DEBUGln( fxos.getTemperature() );
    
    fxos.start();
    
    PacketBuilder::GeneratePacket( gTypeFlags );
    gTxBuff.Flush();
}