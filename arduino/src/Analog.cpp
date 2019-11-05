#include "Analog.h"
#include "clocks.h"
#include "GPIO.h"

#define BAND_GAP_MV 1100

int32_t _ctrlB;

// Wait for the ADC to synchronize
#define ADC_WAIT_SYNC while( ADC->STATUS.bit.SYNCBUSY )

// Sets the internal voltage reference for the ADC
#define ADC_SET_REF( x )                         \
    ADC->REFCTRL.reg &= ~ADC_REFCTRL_REFSEL_Msk; \
    ADC->REFCTRL.reg |= x;

// Sets the number of samples to accumulate and average for the ADC and
// corresponding configuration bits
#define ADC_SET_SAMPLE_ACCUM( x )                       \
    {                                                   \
        ADC->AVGCTRL.reg = 0;                           \
        ADC->AVGCTRL.reg |= ADC_AVGCTRL_SAMPLENUM( x ); \
                                                        \
        if( x > ADC_AVGCTRL_SAMPLENUM_1_Val ) {         \
            _ctrlB &= ~ADC_CTRLB_RESSEL_Msk;            \
            _ctrlB |= ADC_CTRLB_RESSEL_16BIT;           \
        }                                               \
                                                        \
        uint32_t y = (uint32_t)x;                       \
        if( y > ADC_AVGCTRL_SAMPLENUM_16_Val )          \
            y = ADC_AVGCTRL_SAMPLENUM_16_Val;           \
        ADC->AVGCTRL.reg |= ADC_AVGCTRL_ADJRES( y );    \
    }

// Sets the read resolution of the ADC assuming we don't use averaging
#define ADC_SET_RESOLUTION( x )      \
    _ctrlB &= ~ADC_CTRLB_RESSEL_Msk; \
    _ctrlB |= x;

// Sets the ADC clock input divider
#define ADC_SET_PRESCALER( x )          \
    _ctrlB &= ~ADC_CTRLB_PRESCALER_Msk; \
    _ctrlB |= x;

// Bring up ADC
#define BRING_UP_ADC                                                       \
    enableAPBCClk( PM_APBCMASK_ADC, 1 );                                   \
    initGenericClk( GCLK_CLKCTRL_GEN_GCLK0_Val, GCLK_CLKCTRL_ID_ADC_Val ); \
    _ctrlB = 0;                                                            \
    if( ADC->CTRLA.bit.ENABLE ) {                                          \
        ADC->CTRLA.bit.SWRST = 1;                                          \
        while( ADC->CTRLA.bit.SWRST && ADC->STATUS.bit.SYNCBUSY )          \
            ;                                                              \
    }

// Take down ADC
#define TAKE_DOWN_ADC                             \
    ADC->CTRLA.bit.ENABLE = 0;                    \
    ADC_WAIT_SYNC;                                \
    disableGenericClk( GCLK_CLKCTRL_ID_ADC_Val ); \
    enableAPBCClk( PM_APBCMASK_ADC, 0 );

// Wait for the DAC to synchronize
#define DAC_WAIT_SYNC while( DAC->STATUS.bit.SYNCBUSY )

int16_t singleShotConversion()
{
    // The first conversion after the reference is changed must not be used.
    int16_t val;
    for( uint8_t i = 0; i < 2; i++ ) {
        // Start the next conversion
        ADC->SWTRIG.bit.START = 1;
        ADC_WAIT_SYNC;

        // Waiting for conversion to complete
        while( !ADC->INTFLAG.bit.RESRDY )
            ;

        // Grab the value
        ADC_WAIT_SYNC;
        val = ADC->RESULT.reg;
    }

    return val;
}

int16_t Analog::readSingle()
{
    // Ensure the ADC is powered up
    BRING_UP_ADC

    // Ensure the positive input channel is actually an analog channel
    if( _posChannel == -1 ) return -1;

    // Configure input pins, if using dual-ended input configure for
    // differential mode
    pinMode( _posInputPin, gArduinoPins[_posInputPin].analog );
    if( _negInputPin != -1 ) {
        pinMode( _negInputPin, gArduinoPins[_negInputPin].analog );
        _ctrlB |= ADC_CTRLB_DIFFMODE;
    }

    // Configure the read parameters
    ADC_SET_REF( _settings._ref );
    ADC_SET_RESOLUTION( _settings._resolution );
    ADC_SET_PRESCALER( _settings._preScaler );
    ADC_SET_SAMPLE_ACCUM( _settings._accum );
    ADC->CTRLB.reg = _ctrlB;
    ADC_WAIT_SYNC;

    // Configure the gain, sample length (fixed), and the input channels
    ADC->SAMPCTRL.reg = ADC_SAMPCTRL_MASK; // 64 ADC clock cycles
    ADC->INPUTCTRL.reg = _settings._gain | ADC_INPUTCTRL_MUXPOS( _posChannel ) |
                         ADC_INPUTCTRL_MUXNEG( _negChannel );
    ADC_WAIT_SYNC;

    // Enable the ADC
    ADC->CTRLA.bit.ENABLE = 1;
    ADC_WAIT_SYNC;

    int16_t val = singleShotConversion();

    // Disable the peripheral to save power
    TAKE_DOWN_ADC

    return val;
}

int16_t Analog::readSingle( AnalogSettings settings )
{
    _settings = settings;
    return readSingle();
}

void Analog::writeSingle( int16_t val, bool outputInternal )
{
    // TODO: perhaps handle pins that aren't the DAC output pin
    if( gArduinoPins[_posInputPin].pin != 2 )
        if( !outputInternal ) return;

    enableAPBCClk( PM_APBCMASK_DAC, 1 );
    initGenericClk( GCLK_CLKCTRL_GEN_GCLK0_Val, GCLK_CLKCTRL_ID_DAC_Val );
    pinMode( _posInputPin, gArduinoPins[_posInputPin].analog );

    // SWRST
    if( !( DAC->CTRLA.reg & DAC_CTRLA_ENABLE ) ) {
        DAC->CTRLA.reg |= DAC_CTRLA_SWRST;
        while( DAC->CTRLA.bit.SWRST || DAC->STATUS.bit.SYNCBUSY )
            ;

        DAC->CTRLB.reg = DAC_CTRLB_REFSEL_AVCC | DAC_CTRLB_EOEN;
        DAC->CTRLA.reg |= DAC_CTRLA_ENABLE;
        DAC_WAIT_SYNC;
    }

    DAC->DATA.reg = val & 0x3FF;
    DAC_WAIT_SYNC;
}

int16_t Analog::readVCC()
{
    BRING_UP_ADC

    // Have to set the band-gap channel as an input into the ADC
    SYSCTRL->VREF.bit.BGOUTEN = 1;

    // Configure the read parameters
    ADC_SET_REF( ana_ref_internal_0_5_vddana );
    ADC_SET_RESOLUTION( ana_resolution_12bit );
    ADC_SET_PRESCALER( ana_clk_div_8 );
    ADC_SET_SAMPLE_ACCUM( ana_accum_64 );
    ADC->CTRLB.reg = _ctrlB;
    ADC_WAIT_SYNC;

    // Configure the gain, sample length (fixed), and the input channels
    ADC->SAMPCTRL.reg = ADC_SAMPCTRL_MASK; // 64 ADC clock cycles
    ADC->INPUTCTRL.reg = ADC_INPUTCTRL_GAIN_1X | ADC_INPUTCTRL_MUXPOS_BANDGAP |
                         ADC_INPUTCTRL_MUXNEG_GND;
    ADC_WAIT_SYNC;

    // Enable the ADC
    ADC->CTRLA.bit.ENABLE = 1;
    ADC_WAIT_SYNC;

    int16_t val = singleShotConversion();

    // Disable the peripheral to save power
    TAKE_DOWN_ADC

    // Take down the band gap input
    SYSCTRL->VREF.bit.BGOUTEN = 0;

    // Convert band-gap to VCC
    int16_t vccMv = ( ( BAND_GAP_MV * 4095 ) / val ) * 2;
    return vccMv;
}

int16_t Analog::readTemperature()
{
    // Read calibration values
    uint32_t *tempLogLow = (uint32_t *)NVMCTRL_TEMP_LOG;
    uint32_t *tempLogHigh = (uint32_t *)( NVMCTRL_TEMP_LOG + 4 );
    int8_t    roomTempVal =
        ( ( *tempLogLow ) & NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_Msk ) >>
        NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_Pos;
    int8_t hotRoomTempVal =
        ( ( *tempLogLow ) & NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Msk ) >>
        NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Pos;
    int16_t roomADCVal =
        ( ( *tempLogHigh ) & NVMCTRL_FUSES_ROOM_ADC_VAL_Msk ) >>
        NVMCTRL_FUSES_ROOM_ADC_VAL_Pos;
    int16_t hotADCVal = ( ( *tempLogHigh ) & NVMCTRL_FUSES_HOT_ADC_VAL_Msk ) >>
                        NVMCTRL_FUSES_HOT_ADC_VAL_Pos;

    BRING_UP_ADC

    // Have to set the temperature sensor channel as an input into the ADC
    SYSCTRL->VREF.bit.TSEN = 1;

    // Configure the read parameters
    ADC_SET_REF( ana_ref_internal_1v );
    ADC_SET_RESOLUTION( ana_resolution_12bit );
    ADC_SET_PRESCALER( ana_clk_div_8 );
    ADC_SET_SAMPLE_ACCUM( ana_accum_64 );
    ADC->CTRLB.reg = _ctrlB;
    ADC_WAIT_SYNC;

    // Configure the gain, sample length (fixed), and the input channels
    ADC->SAMPCTRL.reg = ADC_SAMPCTRL_MASK; // 64 ADC clock cycles
    ADC->INPUTCTRL.reg = ADC_INPUTCTRL_GAIN_1X | ADC_INPUTCTRL_MUXPOS_TEMP |
                         ADC_INPUTCTRL_MUXNEG_GND;
    ADC_WAIT_SYNC;

    // Enable the ADC
    ADC->CTRLA.bit.ENABLE = 1;
    ADC_WAIT_SYNC;

    int16_t val = singleShotConversion();

    // Disable the peripheral to save power
    TAKE_DOWN_ADC

    // Take down the temperature sensor
    SYSCTRL->VREF.bit.TSEN = 0;

    // Perform temperature conversion via interpolation using the calibration
    // values. Calculation assumes that reference voltage is 1 volt, taken from
    // the data sheet
    int16_t temp =
        roomTempVal +
        ( ( ( val - roomADCVal ) * ( hotRoomTempVal - roomTempVal ) ) /
          ( hotADCVal - roomADCVal ) );

    return temp;
}

void Analog::setPosChannel( int32_t pin )
{
    // Ensure the pin supports analog, and that the pin is within valid
    // range of analog support
    if( gArduinoPins[pin].analog == -1 || pin > PINS_COUNT )
        _posChannel = -1;
    else {
        // Set the positive pin, pin mapping is taken directly from the data
        // sheet. Allowable negative input pins for the SAMD20 are AIN0 -
        // AIN19
        _posChannel = gArduinoPins[pin].pin;
        if( _posChannel < 4 )
            _posChannel &= 0x1; // Low channels
        else if( _posChannel > 7 && _posChannel < 12 )
            _posChannel += 8; // High channels
    }
}

void Analog::setNegChannel( int32_t pin )
{
    // Ensure the pin supports analog, and that the pin is within valid
    // range of analog support
    if( pin == -1 || gArduinoPins[pin].analog == -1 ||
        gArduinoPins[pin].pin > 7 || pin > PINS_COUNT ) {
        _negChannel = ADC_INPUTCTRL_MUXNEG_GND_Val;
        _negInputPin = -1;
    }
    else {
        // Set the negative pin, pin mapping is taken directly from the data
        // sheet. Allowable negative input pins for the SAMD20 are AIN0 -
        // AIN7
        _negChannel = gArduinoPins[pin].pin;
        if( _negChannel < 4 ) _negChannel &= 0x1;
    }
}
