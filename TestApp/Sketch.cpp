#include <Arduino.h>
#include <Debug.h>
#include <FXOS8700.h>
#include <PacketBuilder.h>
#include <SPIFlash.h>
#include <AMRConfigTable.h>

uint8_t gLEDLevels = 0x0;

// PacketBuilder / Parser
Buffer_t gTxBuff, gRxBuff;
uint8_t gTypeFlags = 0;
uint32_t    gSendPeaksGlobal;
const char *baked_sha20 = "12345678901234567890";

// SPIFlash
SPIFlash flash(FLASH_SS);

// Config table
AMRConfigTable configTable;

void setup() {
    DEBUGbegin( SERIAL_BAUD );
    
    pinMode( LED1, OUTPUT );
    pinMode( LED2, OUTPUT );
    
    pinMode( RFM_SS, OUTPUT );
    digitalWrite( RFM_SS, HIGH );
    pinMode( FLASH_SS, OUTPUT );
    
    PacketBuilder::InitPacketBuilder( &gTypeFlags, &gTxBuff );
    
    NetInfo_t netInfo;
    configTable.loadItem( config_item_net_info, &netInfo );
    if( netInfo.network != 0x6969 ) {
        netInfo.network = 0x6969;
        netInfo.awake = 1;
        netInfo.fastUpdates = 1;
        for( uint8_t i = 0; i < 16; i++ ) 
            netInfo.key[i] = i;
        configTable.writeItem( config_item_net_info, &netInfo );
    }
}

void loop() {
    // Blink this so I know it's alive
    delay( 250 );
    digitalWrite( LED1, gLEDLevels );
    digitalWrite( LED2, gLEDLevels );
    
    fxos.initialize();
        
    DEBUG_( "Time : " );
    DEBUGln( millis() );    
    DEBUG_( "Temp : " );
    DEBUGln( fxos.getTemperature() );
    
    fxos.start();
    
    PacketBuilder::GeneratePacket( gTypeFlags );
    gTxBuff.Flush();
}