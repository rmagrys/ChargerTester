#include <AnalogButton.h>

AnalogButton::AnalogButton(){
    this->tollerance = 0;
    this->workingValue = 0;
}

AnalogButton::AnalogButton(uint16_t workingValue, uint8_t tollerance){
    this->workingValue = workingValue;
    this->tollerance = tollerance;
}

uint8_t AnalogButton::buttonMinValue() {
    return this->workingValue - this->tollerance;
}

uint8_t AnalogButton::buttonMaxValue() {
    return this->workingValue - this->tollerance;
}