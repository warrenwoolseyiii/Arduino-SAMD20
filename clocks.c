/*
  Written by Warren Woolsey

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

#include "sam.h"
#include "clocks.h"

#define GCLK_WAIT_SYNC while( GCLK->STATUS.bit.SYNCBUSY )
#define SYSCTRL_DFLL_WAIT_SYNC while( !SYSCTRL->PCLKSR.bit.DFLLRDY )
#define SYSCTRL_OSC8M_WAIT_SYNC            \
    while( !SYSCTRL->PCLKSR.bit.OSC8MRDY ) \
        ;

void resetGCLK()
{
    PM->APBAMASK.reg |= PM_APBAMASK_GCLK;
    GCLK->CTRL.reg = GCLK_CTRL_SWRST;

    while( ( GCLK->CTRL.bit.SWRST ) && ( GCLK->STATUS.bit.SYNCBUSY ) )
        ;
}

/* Note: Caller is responsible for knowing the limitations of dividers and setup
 * for each of the various clock sources, generators, and dividers */
int8_t initClkGenerator( uint32_t clkSrc, uint32_t id, uint32_t div,
                         uint8_t runInStdBy, uint8_t outPutToPin )
{
    uint32_t genDivReg = 0;
    uint32_t genCtrlReg = 0;
    if( clkSrc > GCLK_GENCTRL_SRC_DFLL48M_Val ) return -1;
    if( id > GCLK_GENDIV_ID_GCLK7_Val ) return -1;

    genDivReg = GCLK_GENDIV_ID( id ) | GCLK_GENDIV_DIV( div );
    GCLK->GENDIV.reg = genDivReg;
    GCLK_WAIT_SYNC;

    genCtrlReg = GCLK_GENCTRL_ID( id ) | GCLK_GENCTRL_SRC( clkSrc ) |
                 GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN;
    if( runInStdBy ) genCtrlReg |= GCLK_GENCTRL_RUNSTDBY;
    if( div != 0 ) genCtrlReg |= GCLK_GENCTRL_DIVSEL;
    if( outPutToPin ) genCtrlReg |= GCLK_GENCTRL_OE;

    GCLK->GENCTRL.reg = genCtrlReg;
    GCLK_WAIT_SYNC;

    return 0;
}

void disableClkGenerator( uint32_t id )
{
    if( id <= GCLK_GENDIV_ID_GCLK7_Val ) {
        GCLK->GENCTRL.reg = GCLK_GENCTRL_ID( id );
        GCLK_WAIT_SYNC;
    }

    return;
}

int8_t initGenericClk( uint32_t genClk, uint32_t id )
{
    if( genClk > GCLK_CLKCTRL_GEN_GCLK7_Val ) return -1;
    if( id > GCLK_CLKCTRL_ID_PTC_Val ) return -1;

    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_ID( id ) | GCLK_CLKCTRL_GEN( genClk ) | GCLK_CLKCTRL_CLKEN;
    GCLK_WAIT_SYNC;

    return 0;
}

void disableGenericClk( uint32_t id )
{
    if( id <= GCLK_CLKCTRL_ID_PTC_Val ) {
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( id );
        GCLK_WAIT_SYNC;
    }
}

void enableAPBAClk( uint32_t item, uint8_t enable )
{
    item &= PM_APBAMASK_MASK;
    if( enable )
        PM->APBAMASK.reg |= item;
    else
        PM->APBAMASK.reg &= ~( item );
}

void enableAPBBClk( uint32_t item, uint8_t enable )
{
    item &= PM_APBBMASK_MASK;
    if( enable )
        PM->APBBMASK.reg |= item;
    else
        PM->APBBMASK.reg &= ~( item );
}

void enableAPBCClk( uint32_t item, uint8_t enable )
{
    item &= PM_APBCMASK_MASK;
    if( enable )
        PM->APBCMASK.reg |= item;
    else
        PM->APBCMASK.reg &= ~( item );
}

void initXOSC32()
{
    // Longest start up time, run in stand by
    SYSCTRL->XOSC32K.reg = SYSCTRL_XOSC32K_STARTUP( 0x7u ) |
                           SYSCTRL_XOSC32K_XTALEN | SYSCTRL_XOSC32K_EN32K |
                           SYSCTRL_XOSC32K_RUNSTDBY;
    SYSCTRL->XOSC32K.bit.ENABLE =
        1; // Set in a separate call as called out in 16.6.3

    while( !SYSCTRL->PCLKSR.bit.XOSC32KRDY )
        ;
}

void initOSC23K()
{
#define FUSES_OSC32K_CAL_ADDR \
    ( uint32_t )( 0x806020 + 38 ) // SAMD20 data sheet section 9.5, table 9-4
    uint32_t calib = ( *( (uint32_t *)FUSES_OSC32K_CAL_ADDR ) & 0x00FFFFFF );

    SYSCTRL->OSC32K.reg =
        SYSCTRL_OSC32K_CALIB( calib ) |
        SYSCTRL_OSC32K_STARTUP(
            0x7u ) // cf table 15.10 of product data sheet in chapter 15.8.6
        | SYSCTRL_OSC32K_EN32K | SYSCTRL_OSC32K_ENABLE;

    while( !SYSCTRL->PCLKSR.bit.OSC32KRDY )
        ;
}

void initDFLL48( uint32_t sourceFreq )
{
    // DFLL Configuration in Closed Loop mode, cf product datasheet
    // chapter 15.6.7.1 - Closed-Loop Operation Remove the OnDemand mode, Bug
    // http://avr32.icgroup.norway.atmel.com/bugzilla/show_bug.cgi?id=9905
    SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE;
    SYSCTRL_DFLL_WAIT_SYNC;

    SYSCTRL->DFLLMUL.reg =
        SYSCTRL_DFLLMUL_CSTEP( 31 ) // Coarse step is 31, half of the max value
        |
        SYSCTRL_DFLLMUL_FSTEP( 511 ) // Fine step is 511, half of the max value
        | SYSCTRL_DFLLMUL_MUL( 48000000ul / sourceFreq );

    SYSCTRL_DFLL_WAIT_SYNC;

    // Run in closed-loop mode, disable quick lock
    SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_MODE | SYSCTRL_DFLLCTRL_QLDIS;

    SYSCTRL_DFLL_WAIT_SYNC;

    // Enable and wait for lock
    SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_ENABLE;
    while( ( SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKC ) == 0 ||
           ( SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKF ) == 0 )
        ;

    SYSCTRL_DFLL_WAIT_SYNC;
}

void disableDFLL48()
{
    SYSCTRL->DFLLCTRL.reg = 0;
    SYSCTRL_DFLL_WAIT_SYNC;
}

int8_t initOSC8M( uint32_t divBits )
{
    if( divBits > 0x3 ) return -1;

    SYSCTRL->OSC8M.bit.ONDEMAND = 0;
    SYSCTRL->OSC8M.bit.PRESC = divBits;
    SYSCTRL->OSC8M.bit.ENABLE = 1;
    SYSCTRL_OSC8M_WAIT_SYNC;

    return 0;
}

void disableOSC8M()
{
    SYSCTRL->OSC8M.bit.ENABLE = 0;
}