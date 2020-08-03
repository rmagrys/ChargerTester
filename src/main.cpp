#include <Arduino.h>
#include <AnalogButton.h>
#include <buttonHandler.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#define OLED_RESET 4

Adafruit_SH1106 display(OLED_RESET);
//Adafruit_SSD1306 display(SCREEN_WIDTH ,SCREN_HEIGHT, &Wire , OLED_RESET);

#if (SH1106_LCDHEIGHT != 64)
  #error("Height incorrect, please fix Adafruit_SH1106.h!");
#endif

const uint8_t TOLLERANCE = 2;
const uint16_t BUTTON_1_VALUE = 769;
const uint16_t BUTTON_2_VALUE = 616;
const uint16_t BUTTON_3_VALUE = 514;
const uint16_t BUTTON_4_VALUE = 440;


const AnalogButton button1(BUTTON_1_VALUE, TOLLERANCE);
const AnalogButton button2(BUTTON_2_VALUE, TOLLERANCE);
const AnalogButton button3(BUTTON_3_VALUE, TOLLERANCE);
const AnalogButton button4(BUTTON_4_VALUE, TOLLERANCE);

const uint8_t analogPin = A2;
uint16_t readedButtonValue = 0;

const uint16_t timeToHoldButton = 100;
boolean anyButtonPressed = false;
boolean buttonFunctionOccured = false;
uint16_t pressedButtonTime = 0;
uint16_t countdownTime = 0;
unsigned long timer;

uint8_t selectCPMode = 0;
uint8_t selectPPMode = 0;

void displayResults();

void setup()
{
  Wire.begin();
  Serial.begin(115200);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  delay(1000);
  display.clearDisplay();
  display.clearDisplay();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("dziala"));
  display.display();
  delay(3000);
}

void loop()
{
  readedButtonValue = analogRead(analogPin);

  if (isAnalogButtonPressed(button1, readedButtonValue) && !anyButtonPressed)
  {
    anyButtonPressed = true;
    pressedButtonTime = millis();

    Serial.println(timer);
  }
  else
  if (isAnalogButtonPressed(button2, readedButtonValue) && !anyButtonPressed)
  {
    anyButtonPressed = true;
    pressedButtonTime = millis();
  }
  else
  if (isAnalogButtonPressed(button3, readedButtonValue) && !anyButtonPressed)
  {
    anyButtonPressed = true;
    pressedButtonTime = millis();
  }
  else
  if (isAnalogButtonPressed(button4, readedButtonValue) && !anyButtonPressed)
  {
    anyButtonPressed = true;
    pressedButtonTime = millis();
  }
  else
  if (readedButtonValue == 0)
  {
    anyButtonPressed = false;
    buttonFunctionOccured = false;
  }

  if (isAnalogButtonPressed(button1, readedButtonValue) && anyButtonPressed)
  {
    countdownTime = millis();
    if (countdownTime - pressedButtonTime > timeToHoldButton && !buttonFunctionOccured)
    {
      selectPPMode++;
      displayResults();
      buttonFunctionOccured = true;
    }
  }
  else
  if (isAnalogButtonPressed(button2, readedButtonValue) && anyButtonPressed)
  {
    countdownTime = millis();
    if (countdownTime - pressedButtonTime > timeToHoldButton && !buttonFunctionOccured)
    {
      if(selectPPMode > 0){
        selectPPMode--;
      }
      displayResults();
      buttonFunctionOccured = true;
    }
  }
  else
  if (isAnalogButtonPressed(button3, readedButtonValue) && anyButtonPressed)
  {
    countdownTime = millis();
    if (countdownTime - pressedButtonTime > timeToHoldButton && !buttonFunctionOccured)
    {
     
      selectCPMode++;
      displayResults();
      buttonFunctionOccured = true;
    }
  }
  else
  if (isAnalogButtonPressed(button4, readedButtonValue) && anyButtonPressed)
  {
    countdownTime = millis();
    if (countdownTime - pressedButtonTime > timeToHoldButton && !buttonFunctionOccured)
    {
      if(selectCPMode > 0){
     
          selectCPMode--; 
      }
      displayResults();
      buttonFunctionOccured = true;
    }
  }
     
}

void displayResults(){
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println("PP mode " + String(selectPPMode));
      display.setCursor(64, 0);
      display.println("CP mode " + String(selectCPMode));
      display.display();
}

