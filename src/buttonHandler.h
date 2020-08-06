#ifndef BUTTONHANDLER_H
#define BUTTONHANDLER_H
#include <AnalogButton.h>

boolean isAnalogButtonPressed(AnalogButton analogButton, uint16_t readedAnalogValue){
        return readedAnalogValue > analogButton.buttonMinValue() && readedAnalogValue < analogButton.buttonMaxValue();
}
boolean isDefinedAnalogValuePresent(AnalogButton analogButton, uint16_t readedAnalogValue){
        return readedAnalogValue > analogButton.buttonMinValue() && readedAnalogValue < analogButton.buttonMaxValue();
}

#endif