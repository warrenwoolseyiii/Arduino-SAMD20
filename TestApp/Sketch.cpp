#include <Arduino.h>
#include <Debug.h>
#include <FXOS8700.h>
#include <PacketBuilder.h>

uint8_t gLEDLevels = 0x1;

// PacketBuilder / Parser
Buffer_t gTxBuff, gRxBuff;
uint8_t gTypeFlags = 0;
uint32_t    gSendPeaksGlobal;
const char *baked_sha20 = "12345678901234567890";

void setup() {
    DEBUGbegin( SERIAL_BAUD );
    
    pinMode( LED1, OUTPUT );
    pinMode( LED2, OUTPUT );
    
    pinMode( RFM_SS, OUTPUT );
    pinMode( FLASH_SS, OUTPUT );
    
    PacketBuilder::InitPacketBuilder( &gTypeFlags, &gTxBuff );
}

void loop() {
    // Blink this so I know it's alive
    delay( 250 );
    digitalWrite( LED1, gLEDLevels );
    digitalWrite( LED2, gLEDLevels );
    
    if( fxos.initialize() )
        gLEDLevels ^= 0x1;
        
    PacketBuilder::GeneratePacket( gTypeFlags );
    gTxBuff.Flush();
}