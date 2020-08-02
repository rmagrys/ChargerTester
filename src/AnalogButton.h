#ifndef ANALOGBUTTON_H
#define ANALOGBUTTON_H

#include <Arduino.h>
class AnalogButton{

    uint16_t workingValue;
    uint8_t tollerance;

    public:

    AnalogButton();
    AnalogButton(uint_fast16_t workingVaule, uint_fast8_t tollerance);
    uint8_t buttonMinValue();
    uint8_t buttonMaxValue();
        
    


};
#endif