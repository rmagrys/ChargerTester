#include <Arduino.h>
#include <AnalogButton.h>
#include <buttonHandler.h>

int analogPin = A2;
int readedButtonValue = 0;
unsigned long timer;


const int TOLLERANCE = 20;
const int BUTTON_1_VALUE = 470;
const int BUTTON_2_VALUE = 360;
const int BUTTON_3_VALUE = 290;
const int BUTTON_4_VALUE = 241;

AnalogButton *button1 = new AnalogButton(BUTTON_1_VALUE, TOLLERANCE);
AnalogButton *button2 = new AnalogButton(BUTTON_2_VALUE, TOLLERANCE);
AnalogButton *button3 = new AnalogButton(BUTTON_3_VALUE, TOLLERANCE);
AnalogButton *button4 = new AnalogButton(BUTTON_4_VALUE, TOLLERANCE);

void setup() {
  Serial.begin(115200);
  
}

void loop() {
  readedButtonValue = analogRead(analogPin);
  Serial.println(readedButtonValue);

  if(isAnalogButtonPressed(*button1, readedButtonValue)){
      timer = millis()  %10000;
      Serial.println("button1");
      Serial.println(timer);
  }
    if(isAnalogButtonPressed(*button2, readedButtonValue)){

      Serial.println("button2");
  }
    if(isAnalogButtonPressed(*button3, readedButtonValue)){

      Serial.println("button3");
  }
    if(isAnalogButtonPressed(*button4, readedButtonValue)){

      Serial.println("button4");
  }
  
}
