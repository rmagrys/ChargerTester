#include <AnalogButton.h>

AnalogButton::AnalogButton(){
    this->buttonMaxValue = 0;
    this->buttonMinValue = 0;
}

AnalogButton::AnalogButton(uint16_t buttonMinValue, uint16_t buttonMaxValue){
    this->buttonMaxValue = buttonMaxValue;
    this->buttonMinValue = buttonMinValue;
}

uint16_t AnalogButton::getButtonMinValue() {
    return this->buttonMinValue;
}

uint16_t AnalogButton::getButtonMaxValue() {
    return this->buttonMaxValue;

}