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

const AnalogButton analogButton1(BUTTON_1_VALUE, TOLLERANCE);
const AnalogButton analogButton2(BUTTON_2_VALUE, TOLLERANCE);
const AnalogButton analogButton3(BUTTON_3_VALUE, TOLLERANCE);
const AnalogButton analogButton4(BUTTON_4_VALUE, TOLLERANCE);

const uint8_t CP_PP_MODE_SELECTOR = A2;

const uint8_t errorButton1 = 3;
const uint8_t errorButton2 = 2;
const uint8_t errorButton3 = 5;
boolean errorButton1Pressed = false;
boolean errorButton2Pressed = false;
boolean errorButton3Pressed = false;

const uint8_t CABLE = 4;
const uint8_t CAR = 10;
const uint8_t CHARGE = 11;
const uint8_t CHARGECOOL = 12;

const uint8_t PP_6 = A0;
const uint8_t PP_16 = 6;
const uint8_t PP_20 = 13;
const uint8_t PP_32 = A6;

const String CP_PP_SHORT_ERROR = "CP PP SHORT";
const String PP_DISCONECT_ERROR = "PP DISCONNECT";
const String DIODE_ERROR = "CP NO DIODE";

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
void setOutputValues();
String mapCPValueAsModeName(uint8_t value);
String mapPPValueAsModeName(uint8_t value);

void setup()
{
  pinMode(errorButton1, INPUT);
  pinMode(errorButton2, INPUT);
  pinMode(errorButton3, INPUT);

  pinMode(CABLE, OUTPUT);
  pinMode(CAR, OUTPUT);
  pinMode(CHARGE, OUTPUT);
  pinMode(CHARGECOOL, OUTPUT);

  pinMode(PP_6, OUTPUT);
  pinMode(PP_20, OUTPUT);
  pinMode(PP_16, OUTPUT);
  pinMode(PP_32, OUTPUT);

  Wire.begin();
  Serial.begin(115200);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  delay(1000);
  display.clearDisplay();
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("tester"));
  display.display();
  delay(3000);
}

void loop()
{
  readedButtonValue = analogRead(CP_PP_MODE_SELECTOR);

  if (isAnalogButtonPressed(analogButton1, readedButtonValue) && !anyButtonPressed)
  {
    anyButtonPressed = true;
    pressedButtonTime = millis();
  }
  else if (isAnalogButtonPressed(analogButton2, readedButtonValue) && !anyButtonPressed)
  {
    anyButtonPressed = true;
    pressedButtonTime = millis();
  }
  else if (isAnalogButtonPressed(analogButton3, readedButtonValue) && !anyButtonPressed && selectPPMode > 0)
  {
    anyButtonPressed = true;
    pressedButtonTime = millis();
  }
  else if (isAnalogButtonPressed(analogButton4, readedButtonValue) && !anyButtonPressed && selectPPMode > 0)
  {
    anyButtonPressed = true;
    pressedButtonTime = millis();
  }
  else if (readedButtonValue == 0)
  {
    anyButtonPressed = false;
    buttonFunctionOccured = false;
  }


  if (digitalRead(errorButton1) == HIGH && !errorButton1Pressed)
  {
    errorButton1Pressed = true;
    displayResults();
  }
  else if (digitalRead(errorButton1) == LOW && errorButton1Pressed)
  {
    errorButton1Pressed = false;
    displayResults();
  }

  if (digitalRead(errorButton2) == HIGH && !errorButton2Pressed)
  {
    errorButton2Pressed = true;
    displayResults();
  }
  else if (digitalRead(errorButton2) == LOW && errorButton2Pressed)
  {
    errorButton2Pressed = false;
    displayResults();
  }

  if (digitalRead(errorButton3) == HIGH && !errorButton3Pressed)
  {
    errorButton3Pressed = true;

    displayResults();
  }
  else if (digitalRead(errorButton3) == LOW && errorButton3Pressed)
  {
    errorButton3Pressed = false;
    displayResults();
  }

  if (isAnalogButtonPressed(analogButton1, readedButtonValue) && anyButtonPressed)
  {
    countdownTime = millis();
    if (countdownTime - pressedButtonTime > timeToHoldButton && !buttonFunctionOccured)
    {
      if (selectPPMode > 0)
      {
        selectPPMode--;
      }
      if (selectPPMode == 0)
      {
        selectCPMode = 0;
      }
      setOutputValues();
      displayResults();
      buttonFunctionOccured = true;
    }
  }
  else if (isAnalogButtonPressed(analogButton2, readedButtonValue) && anyButtonPressed)
  {
    countdownTime = millis();
    if (countdownTime - pressedButtonTime > timeToHoldButton && !buttonFunctionOccured)
    {
      if(selectPPMode < 4){
      selectPPMode++;
      }
      displayResults();
      setOutputValues();
      buttonFunctionOccured = true;
    }
  }
  else if (isAnalogButtonPressed(analogButton3, readedButtonValue) && anyButtonPressed)
  {
    countdownTime = millis();
    if (countdownTime - pressedButtonTime > timeToHoldButton && !buttonFunctionOccured)
    {
      if (selectCPMode > 0)
      {
        selectCPMode--;
      }
      setOutputValues();
      displayResults();
      buttonFunctionOccured = true;
  }
  }
  else if (isAnalogButtonPressed(analogButton4, readedButtonValue) && anyButtonPressed)
  {
    countdownTime = millis();
    if (countdownTime - pressedButtonTime > timeToHoldButton && !buttonFunctionOccured)
    {
      if(selectCPMode < 3){
        selectCPMode++;
        }
        setOutputValues();
        displayResults();
    
      buttonFunctionOccured = true;
    }
  }
}
  void displayResults()
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(mapPPValueAsModeName(selectPPMode));
    display.setCursor(64, 0);
    display.println(mapCPValueAsModeName(selectCPMode));
    if (errorButton1Pressed)
    {
      display.setCursor(0, 40);
      display.println("ERROR " + CP_PP_SHORT_ERROR);
    }
    if (errorButton2Pressed)
    {
      display.setCursor(0, 48);
      display.println("ERROR " + PP_DISCONECT_ERROR);
    }
    if (errorButton3Pressed)
    {
      display.setCursor(0, 56);
      display.println("ERROR " + DIODE_ERROR);
    }
    display.display();
  }

  void setOutputValues()
  {

    switch (selectPPMode)
    {
    case 0:
      digitalWrite(PP_6, LOW);
      digitalWrite(PP_16, LOW);
      digitalWrite(PP_20, LOW);
      digitalWrite(PP_32, LOW);
      break;

    case 1:
      digitalWrite(PP_6, HIGH);
      digitalWrite(PP_16, LOW);
      digitalWrite(PP_20, LOW);
      digitalWrite(PP_32, LOW);
      break;

    case 2:
      digitalWrite(PP_6, LOW);
      digitalWrite(PP_16, HIGH);
      digitalWrite(PP_20, LOW);
      digitalWrite(PP_32, LOW);
      break;

    case 3:
      digitalWrite(PP_6, LOW);
      digitalWrite(PP_16, LOW);
      digitalWrite(PP_20, HIGH);
      digitalWrite(PP_32, LOW);
      break;

    case 4:
      digitalWrite(PP_6, LOW);
      digitalWrite(PP_16, LOW);
      digitalWrite(PP_20, LOW);
      digitalWrite(PP_32, HIGH);
      break;

    default:
      break;
    }

    switch (selectCPMode)
    {
    case 0:
      digitalWrite(CAR, LOW);
      digitalWrite(CHARGE, LOW);
      digitalWrite(CHARGECOOL, LOW);
      break;

    case 1:
      digitalWrite(CAR, HIGH);
      digitalWrite(CHARGE, LOW);
      digitalWrite(CHARGECOOL, LOW);
      break;

    case 2:
      digitalWrite(CHARGECOOL, LOW);
      digitalWrite(CHARGE, HIGH);
      break;

    case 3:
      digitalWrite(CHARGE, LOW);
      digitalWrite(CHARGECOOL, HIGH);
    default:
      break;
    }
  }

String mapCPValueAsModeName(uint8_t value){
  switch (value)
  {
  case 0:
    return F("NO CONNECT");
  
  case 1:
    return F("CAR");

  case 2:
    return F("CHARGE");
  
  case 3 :
    return F("CHARGECOOL");
  default:
    return F("ERROR");
    
  }
}

String mapPPValueAsModeName(uint8_t value){
   switch (value)
  {
  case 0:
    return F("NO CONNECT");

  case 1:
    return F("PP-6");
   
  case 2:
    return F("PP-16");
 
  case 3:
    return F("PP-20");
    
  case 4 :
    return F("PP-32");
    
  default:
    return F("ERROR");
  
  }
}