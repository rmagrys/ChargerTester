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

const uint8_t TOLLERANCE = 5;
// const uint16_t BUTTON_1_VALUE = 769;
// const uint16_t BUTTON_2_VALUE = 616;
const uint16_t BUTTON_3_VALUE = 514;
const uint16_t BUTTON_4_VALUE = 440;

#define PP_READ_TOLLERANCE 40
#define NO_CABLE_CONNECTION 823
#define CABLE_32A 237
#define CABLE_20A 455
#define CABLE_13A 603

// const AnalogButton analogButton1(BUTTON_1_VALUE, TOLLERANCE);
// const AnalogButton analogButton2(BUTTON_2_VALUE, TOLLERANCE);
const AnalogButton analogButton3(BUTTON_3_VALUE, TOLLERANCE);
const AnalogButton analogButton4(BUTTON_4_VALUE, TOLLERANCE);

const AnalogButton noCableAnalogInput(NO_CABLE_CONNECTION, PP_READ_TOLLERANCE);
const AnalogButton cable32A_AnalogInput(CABLE_32A, PP_READ_TOLLERANCE);
const AnalogButton cable20A_AnalogInput(CABLE_20A, PP_READ_TOLLERANCE);
const AnalogButton cable13A_AnalogInput(CABLE_13A, PP_READ_TOLLERANCE);


boolean isNoCableDisplayedRecently = false;
boolean isCable_32ADisplayedRecently = false;
boolean isCable_20AisplayedRecently = false;
boolean isCable_13ADisplayedRecently = false;

boolean isChangeDetected = false;
boolean isPPValueDisplayed = false;


const uint8_t CP_PP_MODE_SELECTOR = A2;
const uint8_t PP_CABLE_VOLTAGE = A0;


const uint8_t CAR = 10;
const uint8_t CHARGE = 11;
const uint8_t CHARGECOOL = 12;

uint16_t readedButtonValue = 0;
uint16_t previousReadedPPValue = 0;
uint16_t readedPPVaule = 0;

const uint16_t timeToHoldButton = 100;

boolean anyButtonPressed = false;
boolean buttonFunctionOccured = false;
uint16_t pressedButtonTime = 0;
uint16_t countdownTime = 0;
unsigned long timer;

uint16_t timeReadedAfterChangeInLoop = 0;
uint16_t timeAfterChange = 0;


uint8_t selectCPMode = 0;
uint8_t selectPPMode = 0;

void displayResults();
void setOutputValues();
String mapCPValueAsModeName(uint8_t value);
String mapPPValueAsModeName(uint8_t value);

void setup()
{

    pinMode(CAR, OUTPUT);
    pinMode(CHARGE, OUTPUT);
    pinMode(CHARGECOOL, OUTPUT);

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
    readedPPVaule = analogRead(PP_CABLE_VOLTAGE);

    Serial.println(readedButtonValue);

    if (abs(previousReadedPPValue - readedButtonValue) > 100) {
        timeReadedAfterChangeInLoop = millis();
    }
    previousReadedPPValue = analogRead(PP_CABLE_VOLTAGE);
    // Serial.println(readedPPVaule);


    if (isAnalogButtonPressed(analogButton3, readedButtonValue) && !anyButtonPressed)
    {
        anyButtonPressed = true;
        pressedButtonTime = millis();
    }
    else if (isAnalogButtonPressed(analogButton4, readedButtonValue) && !anyButtonPressed)
    {
        anyButtonPressed = true;
        pressedButtonTime = millis();
    }
    else if (readedButtonValue == 0)
    {
        anyButtonPressed = false;
        buttonFunctionOccured = false;
    }

    if (isDefinedAnalogValuePresent(noCableAnalogInput, readedPPVaule) && !isNoCableDisplayedRecently) {
        timeAfterChange = millis();
       // if (timeAfterChange - timeReadedAfterChangeInLoop > 100) {
            isNoCableDisplayedRecently = true;
            isCable_13ADisplayedRecently = false;
            isCable_20AisplayedRecently = false;
            isCable_32ADisplayedRecently = false;
            selectPPMode = 0;
            Serial.println("No Cable");

            displayResults();
       // }
    }
    if (isDefinedAnalogValuePresent(cable32A_AnalogInput, readedPPVaule) && !isCable_32ADisplayedRecently) {
        timeAfterChange = millis();
       // if (timeAfterChange - timeReadedAfterChangeInLoop > 100) {
            isCable_32ADisplayedRecently = true;
            isNoCableDisplayedRecently = false;
            isCable_13ADisplayedRecently = false;
            isCable_20AisplayedRecently = false;

            Serial.println("32 A");
            selectPPMode = 1;

            displayResults();
       // }
    }
    if (isDefinedAnalogValuePresent(cable20A_AnalogInput, readedPPVaule) && !isCable_20AisplayedRecently) {
        timeAfterChange = millis();
      //  if (timeAfterChange - timeReadedAfterChangeInLoop > 100) {
            isCable_20AisplayedRecently = true;
            isNoCableDisplayedRecently = false;
            isCable_13ADisplayedRecently = false;
            isCable_32ADisplayedRecently = false;
            Serial.println("20 A");
            selectPPMode = 2;
            displayResults();
      //  }
    }
    if (isDefinedAnalogValuePresent(cable13A_AnalogInput, readedPPVaule) && !isCable_13ADisplayedRecently) {
        timeAfterChange = millis();
       // if (timeAfterChange - timeReadedAfterChangeInLoop > 100) {
            isCable_13ADisplayedRecently = true;
            isNoCableDisplayedRecently = false;
            isCable_20AisplayedRecently = false;
            isCable_32ADisplayedRecently = false;
            Serial.println("13 A");
            selectPPMode = 3;
            displayResults();
       // }
    }



    if (isAnalogButtonPressed(analogButton3, readedButtonValue) && anyButtonPressed)
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
            if (selectCPMode < 3) 
            {
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
    display.display();
}

void setOutputValues()
{
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

String mapCPValueAsModeName(uint8_t value) {
    switch (value)
    {
    case 0:
        return F("NO CONNECT");

    case 1:
        return F("CAR");

    case 2:
        return F("CHARGE");

    case 3:
        return F("CHARGECOOL");
    default:

        return F("ERROR");

    }
}

String mapPPValueAsModeName(uint8_t value) {
    switch (value)
    {
    case 0:
        return F("NO CABLE");

    case 1:
        return F("32 A");

    case 2:
        return F("20 A");

    case 3:
        return F("13 A");

    default:
        return F("ERROR");

    }
}
