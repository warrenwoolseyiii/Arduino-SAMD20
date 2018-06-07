#include <Arduino.h>
#include <Debug.h>
#include <FXOS8700.h>
#include <PacketBuilder.h>
#include <PacketParser.h>
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
    PacketParser::InitPacketParser( &gRxBuff );
    
    NetInfo_t info;
    configTable.loadItem( config_item_net_info, &info );
    if( info.network == 0x6969 )
        configTable.eraseItem( config_item_net_info );
    else {
        info.network = 0x6969;
        info.awake = 1;
        info.fastUpdates = 1;
        for( uint8_t i = 0; i < 16; i++ )
        info.key[i] = i;
        configTable.writeItem( config_item_net_info, &info );
    }
    
    fxos.initialize();
    while( !fxos.calibrate() );
    FXOSConfig config;
    configTable.loadItem( config_item_fxos_config, &config );
    fxos.isCalibrated = true;
    fxos.setMeasuringAxis( config.axis, config.range, config.suppressLocalMinima, config.range3dsquared );
    fxos.start();
}

void loop() {
    // Blink this so I know it's alive
    uint32_t prev = gSendPeaksGlobal;
    delay( 250 );
    digitalWrite( LED1, gLEDLevels );
    digitalWrite( LED2, gLEDLevels );
    
    gSendPeaksGlobal = fxos.getPeaks();
    
    DEBUG_( "Time : " );
    DEBUGln( millis() );    
    DEBUG_( "Temp : " );
    DEBUGln( fxos.getTemperature() );
    DEBUG_( "Peaks : " );
    DEBUGln( gSendPeaksGlobal );
    
    if( gSendPeaksGlobal > prev ) {
        gLEDLevels ^= 0x01;
    }
    
    PacketBuilder::GeneratePacket( gTypeFlags );
    gTxBuff.Flush();
}