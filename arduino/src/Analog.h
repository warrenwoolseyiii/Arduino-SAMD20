#ifndef ANALOG_H_
#define ANALOG_H_

#include "sam.h"
#include <stdbool.h>
#include "variant.h"

typedef enum
{
    ana_ref_internal_1v = ADC_REFCTRL_REFSEL_INT1V,
    ana_ref_internal_0_67_vddana = ADC_REFCTRL_REFSEL_INTVCC0,
    ana_ref_internal_0_5_vddana = ADC_REFCTRL_REFSEL_INTVCC1,
    ana_ref_external_a = ADC_REFCTRL_REFSEL_AREFA,
    ana_ref_external_b = ADC_REFCTRL_REFSEL_AREFB
} AnalogReference_t;

typedef enum
{
    ana_accum_1 = ADC_AVGCTRL_SAMPLENUM_1_Val,
    ana_accum_2 = ADC_AVGCTRL_SAMPLENUM_2_Val,
    ana_accum_4 = ADC_AVGCTRL_SAMPLENUM_4_Val,
    ana_accum_8 = ADC_AVGCTRL_SAMPLENUM_8_Val,
    ana_accum_16 = ADC_AVGCTRL_SAMPLENUM_16_Val,
    ana_accum_32 = ADC_AVGCTRL_SAMPLENUM_32_Val,
    ana_accum_64 = ADC_AVGCTRL_SAMPLENUM_64_Val,
    ana_accum_128 = ADC_AVGCTRL_SAMPLENUM_128_Val,
    ana_accum_256 = ADC_AVGCTRL_SAMPLENUM_256_Val,
    ana_accum_512 = ADC_AVGCTRL_SAMPLENUM_512_Val,
    ana_accum_1024 = ADC_AVGCTRL_SAMPLENUM_1024_Val
} AnalogAccum_t;

typedef enum
{
    ana_resolution_8bit = ADC_CTRLB_RESSEL_8BIT,
    ana_resolution_10bit = ADC_CTRLB_RESSEL_10BIT,
    ana_resolution_12bit = ADC_CTRLB_RESSEL_12BIT
} AnalogResolution_t;

typedef enum
{
    ana_clk_div_4 = ADC_CTRLB_PRESCALER_DIV4,
    ana_clk_div_8 = ADC_CTRLB_PRESCALER_DIV8,
    ana_clk_div_16 = ADC_CTRLB_PRESCALER_DIV16,
    ana_clk_div_32 = ADC_CTRLB_PRESCALER_DIV32,
    ana_clk_div_64 = ADC_CTRLB_PRESCALER_DIV64,
    ana_clk_div_128 = ADC_CTRLB_PRESCALER_DIV128,
    ana_clk_div_256 = ADC_CTRLB_PRESCALER_DIV256,
    ana_clk_div_512 = ADC_CTRLB_PRESCALER_DIV512
} AnalogPrescaler_t;

typedef enum
{
    ana_gain_div2 = ADC_INPUTCTRL_GAIN_DIV2,
    ana_gain_1x = ADC_INPUTCTRL_GAIN_1X,
    ana_gain_2x = ADC_INPUTCTRL_GAIN_2X,
    ana_gain_4x = ADC_INPUTCTRL_GAIN_4X,
    ana_gain_8x = ADC_INPUTCTRL_GAIN_8X,
    ana_gain_16x = ADC_INPUTCTRL_GAIN_16X
} AnalogGain_t;

class AnalogSettings
{
  public:
    AnalogSettings()
    {
        _resolution = ana_resolution_12bit;
        _preScaler = ana_clk_div_8;
        _accum = ana_accum_1;
        _ref = ana_ref_internal_1v;
        _gain = ana_gain_1x;
    }

    AnalogSettings( AnalogReference_t refr, AnalogResolution_t res,
                    AnalogPrescaler_t pre, AnalogAccum_t accum,
                    AnalogGain_t gain )
    {
        _resolution = res;
        _preScaler = pre;
        _accum = accum;
        _ref = refr;
        _gain = gain;
    }

    AnalogSettings &operator=( const AnalogSettings &arg )
    {
        this->_gain = arg._gain;
        this->_resolution = arg._resolution;
        this->_ref = arg._ref;
        this->_preScaler = arg._preScaler;
        this->_accum = arg._accum;
        return *this;
    }

  private:
    AnalogResolution_t _resolution;
    AnalogReference_t  _ref;
    AnalogPrescaler_t  _preScaler;
    AnalogAccum_t      _accum;
    AnalogGain_t       _gain;

    friend class Analog;
};

class Analog
{
  public:
    Analog( int32_t posInputPin, int32_t negInputPin = -1 )
    {
        _settings = AnalogSettings();
        _posInputPin = posInputPin;
        _negInputPin = negInputPin;
        setPosChannel( posInputPin );
        setNegChannel( negInputPin );
    }

    Analog( AnalogSettings settings, int32_t posInputPin,
            int32_t negInputPin = -1 )
    {
        _settings = settings;
        _posInputPin = posInputPin;
        _negInputPin = negInputPin;
        setPosChannel( posInputPin );
        setNegChannel( negInputPin );
    }

    int16_t readSingle();
    int16_t readSingle( AnalogSettings settings );
    void    writeSingle( int16_t val, bool outputInternal = false );

    static int16_t readVCC();
    static int16_t readTemperature();

  private:
    AnalogSettings _settings;
    int32_t        _posInputPin, _negInputPin, _posChannel, _negChannel;

    void setPosChannel( int32_t pin );
    void setNegChannel( int32_t pin );
};

#endif /* ANALOG_H_ */
