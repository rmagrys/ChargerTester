#ifndef ANALOGBUTTON_H
#define ANALOGBUTTON_H

#include <Arduino.h>
class AnalogButton{

    uint16_t buttonMinValue;
    uint16_t buttonMaxValue;

    public:

    AnalogButton();
    AnalogButton(uint_fast16_t buttonMinValue, uint_fast16_t buttonMaxValue);
    uint16_t getButtonMinValue();
    uint16_t getButtonMaxValue();
        
};
#endif