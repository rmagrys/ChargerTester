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



#define TOLLERANCE 5
#define BUTTON_1_VALUE 769
#define BUTTON_2_VALUE 616
#define BUTTON_3_VALUE 514
#define BUTTON_4_VALUE 440


#define NO_CABLE_CONNECTION 917                                      //823

#define CABLE_32A_MIN 305                                            //175             /// dla pilota bez mosfetów 180
#define CABLE_32A_MAX 486                                            //314                 //310

#define CABLE_20A_MIN 486                                          //314                //320
#define CABLE_20A_MAX 706                                            //532                //530

#define CABLE_13A_MIN 706                                            //534
#define CABLE_13A_MAX 825                                           //685

#define DELAY_BEFORE_CHANGES 200 

const AnalogButton analogButton1(BUTTON_1_VALUE - TOLLERANCE, BUTTON_1_VALUE + TOLLERANCE);
const AnalogButton analogButton2(BUTTON_2_VALUE - TOLLERANCE, BUTTON_1_VALUE + TOLLERANCE);
const AnalogButton analogButton3(BUTTON_3_VALUE - TOLLERANCE , BUTTON_3_VALUE + TOLLERANCE);
const AnalogButton analogButton4(BUTTON_4_VALUE - TOLLERANCE, BUTTON_4_VALUE + TOLLERANCE);

const AnalogButton noCableAnalogInput(NO_CABLE_CONNECTION- TOLLERANCE, NO_CABLE_CONNECTION + TOLLERANCE);
const AnalogButton cable32A_AnalogInput(CABLE_32A_MIN, CABLE_32A_MAX);
const AnalogButton cable20A_AnalogInput(CABLE_20A_MIN, CABLE_20A_MAX);
const AnalogButton cable13A_AnalogInput(CABLE_13A_MIN, CABLE_13A_MAX);


boolean isNoCableDisplayedRecently = false;
boolean isCable_32ADisplayedRecently = false;
boolean isCable_20AisplayedRecently = false;
boolean isCable_13ADisplayedRecently = false;

boolean timerStarted = true;
boolean isPPValueDisplayed = false;

#define errorButton1 3
#define errorButton2 2
#define errorButton3 5
boolean errorButton1Pressed = false;
boolean errorButton2Pressed = false;
boolean errorButton3Pressed = false;

const String CP_PP_SHORT_ERROR = "CP PP SHORT";
const String PP_DISCONECT_ERROR = "PP DISCONNECT";
const String DIODE_ERROR = "CP NO DIODE";

#define CP_PP_MODE_SELECTOR A2
#define PP_CABLE_VOLTAGE A6

#define CAR 10
#define CHARGE 11
#define CHARGECOOL 12

#define PP_32A A0
#define PP_13A 6
#define PP_20A 13

uint16_t readedButtonValue = 0;
uint16_t readedPPVaule = 0;

#define timeToHoldButton 100

boolean anyButtonPressed = false;
boolean functionOccuredOnce = false;
uint16_t pressedButtonTime = 0;
uint16_t countdownTime = 0;
unsigned long timer;

uint16_t timeReadedAfterChange = 0;
uint16_t countdownToDisplay = 0;


uint8_t selectCPMode = 0;
uint8_t selectPPMode = 0;
uint8_t selectPPModeByButton = 0;
uint8_t selectPilotMode = 0;

void displayResults();
void setOutputValues();
String mapCPValueAsModeName(uint8_t value);
String mapPPValueAsModeName(uint8_t value);
boolean isCableChanged();
boolean isAnyAnalogButtonPressed();
void startingSelectiveMenu();

void setup()
{   
    pinMode(errorButton1, INPUT);
    pinMode(errorButton2, INPUT);
    pinMode(errorButton3, INPUT);

    pinMode(PP_20A, OUTPUT);
    pinMode(PP_13A, OUTPUT);
    pinMode(PP_32A, OUTPUT);

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
    display.setCursor(15, 0);
    display.println(F("Witaj"));
    display.display();
    delay(1500);
    startingSelectiveMenu();
}

void loop()
{   
    readedButtonValue = analogRead(CP_PP_MODE_SELECTOR);

    if( selectPilotMode == 0){
        
            if (isAnalogButtonPressed(analogButton1, readedButtonValue)){
                selectPilotMode = 1;
                anyButtonPressed = true;
                functionOccuredOnce = true;
                
            }
            else if (isAnalogButtonPressed(analogButton2, readedButtonValue))
            {   
                anyButtonPressed = true;
                functionOccuredOnce = true;
                selectPilotMode = 2; 
            }
        
    }
    


    readedButtonValue = analogRead(CP_PP_MODE_SELECTOR);
    readedPPVaule = analogRead(PP_CABLE_VOLTAGE);

    Serial.println(readedButtonValue);

    if (isAnyAnalogButtonPressed()){
        anyButtonPressed = true;
        pressedButtonTime = millis();
    }
    else if (readedButtonValue == 0)
    {
        anyButtonPressed = false;
        functionOccuredOnce = false;
    }

    if( selectPilotMode == 1){
    if(isCableChanged() && !timerStarted){
        timeReadedAfterChange = millis();
        timerStarted = true;
    }
 

    if (isDefinedAnalogValuePresent(noCableAnalogInput, readedPPVaule) && !isNoCableDisplayedRecently) {
        countdownToDisplay = millis();
        if (countdownToDisplay - timeReadedAfterChange > DELAY_BEFORE_CHANGES) {
            isNoCableDisplayedRecently = true;
            isCable_13ADisplayedRecently = false;
            isCable_20AisplayedRecently = false;
            isCable_32ADisplayedRecently = false;
            selectPPMode = 0;
            Serial.println("No Cable");

            displayResults();
            timerStarted = false;
        }
    }
    else if (isDefinedAnalogValuePresent(cable32A_AnalogInput, readedPPVaule) && !isCable_32ADisplayedRecently) {
        countdownToDisplay = millis();
        if (countdownToDisplay - timeReadedAfterChange > DELAY_BEFORE_CHANGES) {
            isCable_32ADisplayedRecently = true;
            isNoCableDisplayedRecently = false;
            isCable_13ADisplayedRecently = false;
            isCable_20AisplayedRecently = false;
            selectPPMode = 1;
            Serial.println("32 A");
        
            displayResults();
            timerStarted = false;
        }
    }
    else if (isDefinedAnalogValuePresent(cable20A_AnalogInput, readedPPVaule) && !isCable_20AisplayedRecently) {
        countdownToDisplay = millis();
        if (countdownToDisplay - timeReadedAfterChange > DELAY_BEFORE_CHANGES) {
            isCable_20AisplayedRecently = true;
            isNoCableDisplayedRecently = false;
            isCable_13ADisplayedRecently = false;
            isCable_32ADisplayedRecently = false;
            Serial.println("20 A");
            selectPPMode = 2;
            displayResults();
            timerStarted = false;
       }
    }
    else if (isDefinedAnalogValuePresent(cable13A_AnalogInput, readedPPVaule) && !isCable_13ADisplayedRecently) {
        countdownToDisplay = millis();
        if (countdownToDisplay - timeReadedAfterChange > DELAY_BEFORE_CHANGES) {
            isCable_13ADisplayedRecently = true;
            isNoCableDisplayedRecently = false;
            isCable_20AisplayedRecently = false;
            isCable_32ADisplayedRecently = false;
            Serial.println("13 A");
            selectPPMode = 3;
            displayResults();
            timerStarted = false;
        }
    }
    }
    else if(selectPilotMode == 2){

    
    if (isAnalogButtonPressed(analogButton1, readedButtonValue) && anyButtonPressed)
    {
        countdownTime = millis();
        if (countdownTime - pressedButtonTime > timeToHoldButton && !functionOccuredOnce)
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
        functionOccuredOnce = true;
        }
    }
    else if (isAnalogButtonPressed(analogButton2, readedButtonValue) && anyButtonPressed)
    {
        countdownTime = millis();
        if (countdownTime - pressedButtonTime > timeToHoldButton && !functionOccuredOnce)
        {
        if(selectPPMode < 4){
        selectPPMode++;
        }
        displayResults();
        setOutputValues();
        functionOccuredOnce = true;
        }
    }
    }

    if (isAnalogButtonPressed(analogButton3, readedButtonValue) && anyButtonPressed)
    {
        countdownTime = millis();
        if (countdownTime - pressedButtonTime > timeToHoldButton && !functionOccuredOnce)
        {
            if (selectCPMode > 0)
            {
                selectCPMode--;
            }
            setOutputValues();
            displayResults();
            functionOccuredOnce = true;
        }
    }
    else if (isAnalogButtonPressed(analogButton4, readedButtonValue) && anyButtonPressed)
    {
        countdownTime = millis();
        if (countdownTime - pressedButtonTime > timeToHoldButton && !functionOccuredOnce)
        {
            if (selectCPMode < 3) 
            {
                selectCPMode++;
            }
            setOutputValues();
            displayResults();
            functionOccuredOnce = true;
        }
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
      display.println("ERROR " + PP_DISCONECT_ERROR);
    }
    if (errorButton2Pressed)
    {
      display.setCursor(0, 48);
      display.println("ERROR "  + CP_PP_SHORT_ERROR);
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
    switch (selectPPModeByButton)
    {
    case 0:
      
      digitalWrite(PP_13A, LOW);
      digitalWrite(PP_20A, LOW);
      digitalWrite(PP_32A, LOW);
      break;

    case 1:
      
      digitalWrite(PP_13A, LOW);
      digitalWrite(PP_20A, LOW);
      digitalWrite(PP_32A, HIGH);
      break;

    case 2:
      digitalWrite(PP_32A, LOW);
      digitalWrite(PP_13A, LOW);
      digitalWrite(PP_20A, HIGH);
      
      break;

    case 3:
      digitalWrite(PP_32A, LOW);
      digitalWrite(PP_20A, LOW);
      digitalWrite(PP_13A, HIGH);
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

boolean isCableChanged(){
    return (isDefinedAnalogValuePresent(noCableAnalogInput, readedPPVaule) && !isNoCableDisplayedRecently) ||
        (isDefinedAnalogValuePresent(cable32A_AnalogInput, readedPPVaule) && !isCable_32ADisplayedRecently) ||
        (isDefinedAnalogValuePresent(cable20A_AnalogInput, readedPPVaule) && !isCable_20AisplayedRecently) ||
        (isDefinedAnalogValuePresent(cable13A_AnalogInput, readedPPVaule) && !isCable_13ADisplayedRecently);
}

boolean isAnyAnalogButtonPressed(){
    return (isAnalogButtonPressed(analogButton1, readedButtonValue) && !anyButtonPressed ) ||
     (isAnalogButtonPressed(analogButton2, readedButtonValue) && !anyButtonPressed ) ||
     (isAnalogButtonPressed(analogButton3, readedButtonValue) && !anyButtonPressed && selectPPMode > 0) ||
     (isAnalogButtonPressed(analogButton4, readedButtonValue) && !anyButtonPressed && selectPPMode > 0);

}

void startingSelectiveMenu(){

    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(50,0 );
    display.println(F("Menu"));
    display.setCursor(0,10);
    display.println(F("Button 1"));
    display.setCursor(4,18);
    display.println("You have PP cable");
    display.setCursor(0,30);
    display.println("Button 2");
    display.setCursor(4,38);
    display.println("Simulate PP cable");
    display.display();
}