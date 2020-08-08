#ifndef BUTTONHANDLER_H
#define BUTTONHANDLER_H
#include <AnalogButton.h>

boolean isAnalogButtonPressed(AnalogButton analogButton, uint16_t readedAnalogValue){
        return readedAnalogValue > analogButton.getButtonMinValue() && readedAnalogValue < analogButton.getButtonMaxValue();
}
boolean isDefinedAnalogValuePresent(AnalogButton analogButton, uint16_t readedAnalogValue){
        return readedAnalogValue > analogButton.getButtonMinValue() && readedAnalogValue < analogButton.getButtonMaxValue();
}

#endif