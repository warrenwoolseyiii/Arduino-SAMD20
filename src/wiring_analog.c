/*
  Copyright (c) 2014 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Arduino.h"
#include "wiring_private.h"

#ifdef SAMD20
#define TCC_INST_NUM 0
#endif /* SAMD20 */

#define BAND_GAP_MV 1100
#define ADC_WAIT_SYNC while( ADC->STATUS.bit.SYNCBUSY )

#ifdef __cplusplus
extern "C" {
#endif

static int _readResolution = 10;
static int _ADCResolution = 10;
static int _writeResolution = 8;

uint32_t _ctrlB = ADC_CTRLB_PRESCALER_DIV512;
uint32_t _inputCtrl = ADC_INPUTCTRL_GAIN_1X;
uint32_t _ref = 0;

// Wait for synchronization of registers between the clock domains
static __inline__ void syncDAC() __attribute__( ( always_inline, unused ) );
static void            syncDAC()
{
    while( DAC->STATUS.bit.SYNCBUSY == 1 )
        ;
}

// Wait for synchronization of registers between the clock domains
static __inline__ void syncTC_16( Tc *TCx )
    __attribute__( ( always_inline, unused ) );
static void syncTC_16( Tc *TCx )
{
    while( TCx->COUNT16.STATUS.bit.SYNCBUSY )
        ;
}

#ifndef SAMD20
// Wait for synchronization of registers between the clock domains
static __inline__ void syncTCC( Tcc *TCCx )
    __attribute__( ( always_inline, unused ) );
static void syncTCC( Tcc *TCCx )
{
    while( TCCx->SYNCBUSY.reg & TCC_SYNCBUSY_MASK )
        ;
}
#endif /* SAMD20 */

void enableADC()
{
    enableAPBCClk( PM_APBCMASK_ADC, 1 );
    initGenericClk( GCLK_CLKCTRL_GEN_GCLK0_Val, GCLK_CLKCTRL_ID_ADC_Val );

    // Reset the peripheral
    ADC->CTRLA.bit.SWRST = 1;
    while( ADC->CTRLA.bit.SWRST && ADC->STATUS.bit.SYNCBUSY )
        ;

    // Source clock settings & resolution
    ADC->CTRLB.reg = _ctrlB;
    ADC_WAIT_SYNC;

    // Reference and sample length
    ADC->REFCTRL.bit.REFSEL = _ref;
    ADC->SAMPCTRL.reg = 0x3F; // ( 63 + 1 ) * ( SysCoreClk / 512 / 2 )

    // Single shot sample, adjusting result by 0
    ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1 | ADC_AVGCTRL_ADJRES( 0 );
}

uint32_t performConversion( uint32_t numSamples )
{
    uint32_t val;
    uint32_t accum;

    // The first conversion after the reference is changed must not be used.
    ADC->SWTRIG.bit.START = 1;
    ADC_WAIT_SYNC;

    // Waiting for conversion to complete
    while( !ADC->INTFLAG.bit.RESRDY )
        ;

    // Grab the value
    ADC_WAIT_SYNC;
    val = ADC->RESULT.reg;

    val = 0;
    for( accum = 0; accum < numSamples; accum++ ) {
        ADC->SWTRIG.bit.START = 1;
        ADC_WAIT_SYNC;

        // Waiting for conversion to complete
        while( !ADC->INTFLAG.bit.RESRDY )
            ;

        // Grab the value
        ADC_WAIT_SYNC;
        val += ADC->RESULT.reg;
    }

    return ( val / accum );
}

void analogReadResolution( int res )
{
    _readResolution = res;
    _ctrlB &= ~ADC_CTRLB_RESSEL_Msk;
    if( res > 10 ) {
        _ctrlB |= ADC_CTRLB_RESSEL_12BIT;
        _ADCResolution = 12;
    }
    else if( res > 8 ) {
        _ctrlB |= ADC_CTRLB_RESSEL_10BIT;
        _ADCResolution = 10;
    }
    else {
        _ctrlB |= ADC_CTRLB_RESSEL_8BIT;
        _ADCResolution = 8;
    }
}

void analogWriteResolution( int res )
{
    _writeResolution = res;
}

static inline uint32_t mapResolution( uint32_t value, uint32_t from,
                                      uint32_t to )
{
    if( from == to ) {
        return value;
    }
    if( from > to ) {
        return value >> ( from - to );
    }
    return value << ( to - from );
}

void analogReference( eAnalogReference mode )
{
    _inputCtrl &= ~ADC_INPUTCTRL_GAIN_Msk;
    switch( mode ) {
        case AR_INTERNAL:
        case AR_INTERNAL2V23:
            _inputCtrl |= ADC_INPUTCTRL_GAIN_1X;
            _ref = ADC_REFCTRL_REFSEL_INTVCC0_Val; // 1/1.48 VDDANA
            break;

        case AR_EXTERNAL:
            _inputCtrl |= ADC_INPUTCTRL_GAIN_1X;
            _ref = ADC_REFCTRL_REFSEL_AREFA_Val;
            break;

        case AR_INTERNAL1V0:
            _inputCtrl |= ADC_INPUTCTRL_GAIN_1X;
            _ref = ADC_REFCTRL_REFSEL_INT1V_Val; // 1.0V voltage reference
            break;

        case AR_INTERNAL1V65:
            _inputCtrl |= ADC_INPUTCTRL_GAIN_1X;
            _ref =
                ADC_REFCTRL_REFSEL_INTVCC1_Val; // 1/2 VDDANA
            break;

        case AR_DEFAULT:
        default:
            _inputCtrl |= ADC_INPUTCTRL_GAIN_DIV2;
            _ref =
                ADC_REFCTRL_REFSEL_INTVCC1_Val; // 1/2 VDDANA
            break;
    }
}

// Reads the band gap voltage from the internal VREF component. The band gap
// voltage can be re-directed to an ADC input, see Data Sheet section 16.6.9.1.
// For more information on band gap voltage and reverse calculating to VCC see
// https://en.wikipedia.org/wiki/Bandgap_voltage_reference. Returns extrapolated
// measurement of VCC in mV.
uint32_t analogReadVcc()
{
    uint32_t read;
    uint32_t vccMv = 0;

    analogReference( AR_INTERNAL1V65 );
    analogReadResolution( 12 );

    SYSCTRL->VREF.bit.BGOUTEN = 1;

    enableADC();

    // Selection for the positive ADC input is band gap
    _inputCtrl &= ~( ADC_INPUTCTRL_MUXNEG_Msk | ADC_INPUTCTRL_MUXPOS_Msk );
    _inputCtrl |= ADC_INPUTCTRL_MUXPOS( ADC_INPUTCTRL_MUXPOS_BANDGAP_Val );
    _inputCtrl |= ADC_INPUTCTRL_MUXNEG_GND;
    ADC->INPUTCTRL.reg = _inputCtrl;
    ADC_WAIT_SYNC;

    // Enable
    ADC->CTRLA.bit.ENABLE = 1;
    ADC_WAIT_SYNC;

    read = performConversion( 10 );

    // Disable ADC
    ADC->CTRLA.bit.ENABLE = 0;
    ADC_WAIT_SYNC;

    disableGenericClk( GCLK_CLKCTRL_ID_ADC_Val );
    enableAPBCClk( PM_APBCMASK_ADC, 0 );

    SYSCTRL->VREF.bit.BGOUTEN = 0;

    vccMv = ( ( BAND_GAP_MV * 4095 ) / read ) * 2;
    return vccMv;
}

uint32_t analogRead( uint32_t pin )
{
    uint32_t read = 0;

    if( pin < A0 ) pin += A0;

    pinPeripheral( pin, PIO_ANALOG );

    if( pin == A0 ) { // Disable DAC, if analogWrite(A0,dval) used previously
                      // the DAC is enabled
        syncDAC();
        DAC->CTRLA.bit.ENABLE = 0x00; // Disable DAC
        // DAC->CTRLB.bit.EOEN = 0x00; // The DAC output is turned off.
        syncDAC();
    }

    enableADC();

    // Selection for the positive ADC input
    _inputCtrl &= ~( ADC_INPUTCTRL_MUXNEG_Msk | ADC_INPUTCTRL_MUXPOS_Msk );
    _inputCtrl |=
        ADC_INPUTCTRL_MUXPOS( g_APinDescription[pin].ulADCChannelNumber );
    _inputCtrl |= ADC_INPUTCTRL_MUXNEG_GND;
    ADC->INPUTCTRL.reg = _inputCtrl;
    ADC_WAIT_SYNC;

    // Enable
    ADC->CTRLA.bit.ENABLE = 1;
    ADC_WAIT_SYNC;

    read = performConversion( 1 );

    // Disable ADC
    ADC->CTRLA.bit.ENABLE = 0;
    ADC_WAIT_SYNC;

    disableGenericClk( GCLK_CLKCTRL_ID_ADC_Val );
    enableAPBCClk( PM_APBCMASK_ADC, 0 );

    return mapResolution( read, _ADCResolution, _readResolution );
}

// TODO: analogWrite
// Right now, PWM output only works on the pins with
// hardware support.  These are defined in the appropriate
// pins_*.c file.  For the rest of the pins, we default
// to digital output.
void analogWrite( uint32_t pin, uint32_t value )
{
    PinDescription pinDesc = g_APinDescription[pin];
    uint32_t       attr = pinDesc.ulPinAttribute;

    if( ( attr & PIN_ATTR_ANALOG ) == PIN_ATTR_ANALOG ) {
        // DAC handling code

        if( pin != PIN_A0 ) { // Only 1 DAC on A0 (PA02)
            return;
        }

        value = mapResolution( value, _writeResolution, 10 );

        syncDAC();
        DAC->DATA.reg = value & 0x3FF; // DAC on 10 bits.
        syncDAC();
        DAC->CTRLA.bit.ENABLE = 0x01; // Enable DAC
        syncDAC();
        return;
    }

    if( ( attr & PIN_ATTR_PWM ) == PIN_ATTR_PWM ) {
        value = mapResolution( value, _writeResolution, 16 );

        uint32_t tcNum = GetTCNumber( pinDesc.ulPWMChannel );
        uint8_t  tcChannel = GetTCChannelNumber( pinDesc.ulPWMChannel );
#ifndef SAMD20
        static bool tcEnabled[TCC_INST_NUM + TC_INST_NUM];
#else
        static bool tcEnabled[TC_INST_NUM];
#endif /* SAMD20 */

        if( attr & PIN_ATTR_TIMER ) {
#if !( ARDUINO_SAMD_VARIANT_COMPLIANCE >= 10603 )
            // Compatibility for cores based on SAMD core <=1.6.2
            if( pinDesc.ulPinType == PIO_TIMER_ALT ) {
                pinPeripheral( pin, PIO_TIMER_ALT );
            }
            else
#endif
            {
                pinPeripheral( pin, PIO_TIMER );
            }
        }
        else {
            // We suppose that attr has PIN_ATTR_TIMER_ALT bit set...
            pinPeripheral( pin, PIO_TIMER_ALT );
        }

        if( !tcEnabled[tcNum] ) {
            tcEnabled[tcNum] = true;

            uint16_t GCLK_CLKCTRL_IDs[] = {
#ifndef SAMD20
                GCLK_CLKCTRL_ID( GCM_TCC0_TCC1 ), // TCC0
                GCLK_CLKCTRL_ID( GCM_TCC0_TCC1 ), // TCC1
                GCLK_CLKCTRL_ID( GCM_TCC2_TC3 ),  // TCC2
                GCLK_CLKCTRL_ID( GCM_TCC2_TC3 ),  // TC3
#else
                GCLK_CLKCTRL_ID( GCM_TC0_TC1 ), // TC0
                GCLK_CLKCTRL_ID( GCM_TC0_TC1 ), // TC1
                GCLK_CLKCTRL_ID( GCM_TC2_TC3 ), // TC2
                GCLK_CLKCTRL_ID( GCM_TC2_TC3 ), // TC3
#endif                                          /* SAMD20 */
                GCLK_CLKCTRL_ID( GCM_TC4_TC5 ), // TC4
                GCLK_CLKCTRL_ID( GCM_TC4_TC5 ), // TC5
                GCLK_CLKCTRL_ID( GCM_TC6_TC7 ), // TC6
                GCLK_CLKCTRL_ID( GCM_TC6_TC7 ), // TC7
            };
            GCLK->CLKCTRL.reg =
                ( uint16_t )( GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 |
                              GCLK_CLKCTRL_IDs[tcNum] );
            while( GCLK->STATUS.bit.SYNCBUSY == 1 )
                ;

            // Set PORT
            if( tcNum >= TCC_INST_NUM ) {
                // -- Configure TC
                Tc *TCx = (Tc *)GetTC( pinDesc.ulPWMChannel );
                // Disable TCx
                TCx->COUNT16.CTRLA.bit.ENABLE = 0;
                syncTC_16( TCx );
                // Set Timer counter Mode to 16 bits, normal PWM
                TCx->COUNT16.CTRLA.reg |=
                    TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_NPWM;
                syncTC_16( TCx );
                // Set the initial value
                TCx->COUNT16.CC[tcChannel].reg = (uint32_t)value;
                syncTC_16( TCx );
                // Enable TCx
                TCx->COUNT16.CTRLA.bit.ENABLE = 1;
                syncTC_16( TCx );
            }
            else {
#ifndef SAMD20
                // -- Configure TCC
                Tcc *TCCx = (Tcc *)GetTC( pinDesc.ulPWMChannel );
                // Disable TCCx
                TCCx->CTRLA.bit.ENABLE = 0;
                syncTCC( TCCx );
                // Set TCCx as normal PWM
                TCCx->WAVE.reg |= TCC_WAVE_WAVEGEN_NPWM;
                syncTCC( TCCx );
                // Set the initial value
                TCCx->CC[tcChannel].reg = (uint32_t)value;
                syncTCC( TCCx );
                // Set PER to maximum counter value (resolution : 0xFFFF)
                TCCx->PER.reg = 0xFFFF;
                syncTCC( TCCx );
                // Enable TCCx
                TCCx->CTRLA.bit.ENABLE = 1;
                syncTCC( TCCx );
#endif /* SAMD20 */
            }
        }
        else {
            if( tcNum >= TCC_INST_NUM ) {
                Tc *TCx = (Tc *)GetTC( pinDesc.ulPWMChannel );
                TCx->COUNT16.CC[tcChannel].reg = (uint32_t)value;
                syncTC_16( TCx );
            }
            else {
#ifndef SAMD20
                Tcc *TCCx = (Tcc *)GetTC( pinDesc.ulPWMChannel );
                TCCx->CTRLBSET.bit.LUPD = 1;
                syncTCC( TCCx );
                TCCx->CCB[tcChannel].reg = (uint32_t)value;
                syncTCC( TCCx );
                TCCx->CTRLBCLR.bit.LUPD = 1;
                syncTCC( TCCx );
#endif /* SAMD20 */
            }
        }
        return;
    }

    // -- Defaults to digital write
    pinMode( pin, OUTPUT );
    value = mapResolution( value, _writeResolution, 8 );
    if( value < 128 ) {
        digitalWrite( pin, LOW );
    }
    else {
        digitalWrite( pin, HIGH );
    }
}

#ifdef __cplusplus
}
#endif
