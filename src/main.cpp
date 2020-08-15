#include <Arduino.h>
#include <math.h>
#include <AnalogButton.h>
#include <buttonHandler.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <time.h>
#include <EEPROM.h>

#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);
#if (SH1106_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SH1106.h!");
#endif

typedef struct {
    uint8_t counter : 4;
}Counter;



#define CONFIG_VERSION "stp"

#define CONFIG_START 32
#define NO_STARTUP_CONFIG 0
#define NO_CABLE_SIMULATING 1
#define CABLE_SIMULATING 2
#define TIME_TO_HOLD_BUTTON_FOR_MENU 3100

struct StoreStruct{
    char version[4];
    uint8_t pilotMode : 2;
}pilotVersion = {
    CONFIG_VERSION,
    0
};


#define TOLLERANCE 5
#define BUTTON_1_VALUE 769
#define BUTTON_2_VALUE 616
#define BUTTON_3_VALUE 514
#define BUTTON_4_VALUE 440

#define NO_CABLE_CONNECTION 917                                      //823

#define CABLE_32A_MIN 305                                            //175             /// dla pilota bez mosfetów 180
#define CABLE_32A_MAX 486                                            //314                 //310

#define CABLE_20A_MIN 486                                          //314                //320
#define CABLE_20A_MAX 706                                         //532                //530

#define CABLE_13A_MIN 706                                           //534
#define CABLE_13A_MAX 825                                           //685

#define DELAY_BEFORE_CHANGES 200 

const AnalogButton analogButton1(BUTTON_1_VALUE - TOLLERANCE, BUTTON_1_VALUE + TOLLERANCE);
const AnalogButton analogButton2(BUTTON_2_VALUE - TOLLERANCE, BUTTON_1_VALUE + TOLLERANCE);
const AnalogButton analogButton3(BUTTON_3_VALUE - TOLLERANCE , BUTTON_3_VALUE + TOLLERANCE);
const AnalogButton analogButton4(BUTTON_4_VALUE - TOLLERANCE, BUTTON_4_VALUE + TOLLERANCE);

const AnalogButton noCableAnalogInput(NO_CABLE_CONNECTION - 10, NO_CABLE_CONNECTION + 10);
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
#define errorButton2 5
boolean errorButton1Pressed = false;
boolean errorButton2Pressed = false;

const String PP_DISCONECT_ERROR = "NO_PP";
const String DIODE_ERROR = "DIODE";

#define CP_PP_MODE_SELECTOR A2
#define PP_CABLE_VOLTAGE A6

#define CP_ERROR_PIN 4
#define FREQUENCY_READ 2
#define CAR 10
#define CHARGE 11
#define CHARGECOOL 12

#define PP_32A A0
#define PP_13A 6
#define PP_20A 13

uint16_t readedButtonValue = 0;
uint16_t readedPPVaule = 0;

#define timeToHoldButton 100
#define TIME_TO_HOLD_BUTTON_FOR_ERROR 6000
#define SEK_1 1000
#define SEK_2 2000
#define SEK_3 3000
#define SEK_4 4000
#define SEK_5 5000

#define COUNTDOWN_START 6
uint8_t countdown = COUNTDOWN_START;
boolean sekDisplay[5] = {0,0,0,0,0};
boolean cpErrorOn = false;
boolean errorSwitch = false;

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

uint8_t counter = 0;
float calculations = 0; 
float frequencyValue = 0;
float newFrequencyValue = 0;

float pulseWidthValue = 0;
float newPulseWidthValue = 0;

boolean frequencyBeyondTheNorm = false;

#define NUMBER_OF_MEASURMENTS 16
#define DATA_COUNT 2
#define MICROS 0
#define STATE 1 
unsigned long periodMeasurement[NUMBER_OF_MEASURMENTS][DATA_COUNT];

Counter m_counter = { .counter = 0 };
Counter counter2 = { .counter = 0};


uint16_t  currentTimer  = 0;
uint16_t  previousTimer = 0;
bool interuptTurnedOn = false;

void displayResults();
void setOutputValues();
String mapCPValueAsModeName(uint8_t value);
String mapPPValueAsModeName(uint8_t value);
boolean isCableChanged();
boolean isAnyAnalogButtonPressed();
void displaySelectiveMenu();
void readFrequency();
void swap(float* xp, float* yp);
void selectionSort(float arr[], uint8_t n);
void resetMeasurements();
void resetCountdown();
void menu();
void loadConfig();
void saveConfig();

float calculateFrequency(unsigned long periodMeasurement[][DATA_COUNT]);
float calculatePulseWidth(unsigned long periodMeasurement[][DATA_COUNT]);

void setup()
{       
    pinMode(FREQUENCY_READ, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(FREQUENCY_READ), readFrequency, CHANGE);
    resetMeasurements();

    pinMode(PP_20A, OUTPUT);
    pinMode(PP_13A, OUTPUT);
    pinMode(PP_32A, OUTPUT);

    pinMode(errorButton1, INPUT);
    pinMode(errorButton2, INPUT);
    

    pinMode(CAR, OUTPUT);
    pinMode(CHARGE, OUTPUT);
    pinMode(CHARGECOOL, OUTPUT);
    pinMode(CP_ERROR_PIN,OUTPUT);
    digitalWrite(CP_ERROR_PIN,LOW);

    loadConfig();

    Wire.begin();
    Serial.begin(115200);
    display.begin(SH1106_SWITCHCAPVCC, 0x3C);
    delay(1000);
    display.clearDisplay();
    display.clearDisplay();
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(15, 0);
    display.println(F("Hello"));
    if(pilotVersion.pilotMode == NO_CABLE_SIMULATING){
        display.setTextSize(1);
        display.setCursor(6, 32);
        display.println(F("Only CP simulate"));
    }
    else if(pilotVersion.pilotMode == CABLE_SIMULATING){
        display.setTextSize(1);
        display.setCursor(6, 32);
        display.println(F("PP and CP simulate"));
    }
    display.display();
    
    delay(1500);
}

void loop()
{   

   
 
//////////////////////////////////////////////////// VERSION CHANGE SECTION //////////////////////////////////////////////////////////
   
    if (pilotVersion.pilotMode == 0)
    {
        displaySelectiveMenu();
        menu();
        displayResults();
    }

    readedButtonValue = analogRead(CP_PP_MODE_SELECTOR);
    readedPPVaule = analogRead(PP_CABLE_VOLTAGE);
    
   // Serial.println(pilotVersion.pilotMode);
    //Serial.println(pilotVersion.version);




    
/////////////////////////////////////////////////// BUTTON SECTION ///////////////////////////////////////////////////////////////

    if (isAnyAnalogButtonPressed()){
        anyButtonPressed = true;
        pressedButtonTime = millis();
        if(cpErrorOn) cpErrorOn = false;
        resetCountdown();
        
    }
    else if (readedButtonValue == 0)
    {   
        if(countdown > 0){
            countdown = COUNTDOWN_START;
            displayResults();
        }
        anyButtonPressed = false;
        functionOccuredOnce = false; 
    }

    if (isAnalogButtonPressed(analogButton1, readedButtonValue) && anyButtonPressed)
    {   
        countdownTime = millis();

        if(pilotVersion.pilotMode == CABLE_SIMULATING){
            
            if (countdownTime - pressedButtonTime > timeToHoldButton && !functionOccuredOnce){
                if (selectPPMode > 0){
                    selectPPMode--;
                }
                if (selectPPMode == 0){
                    selectCPMode = 0;
                }
                setOutputValues();
                displayResults();
                functionOccuredOnce = true;
            }
        }
        if(countdownTime - pressedButtonTime > SEK_1  &&  !sekDisplay[0]){
            countdown = 3;
            sekDisplay[0] = true;
            displayResults();
        }
        if(countdownTime - pressedButtonTime > SEK_2  &&  !sekDisplay[1]){
            countdown = 2;
            sekDisplay[1] = true;
            displayResults();
        }
        if(countdownTime - pressedButtonTime > SEK_3  &&  !sekDisplay[2]){
            countdown = 1;
            sekDisplay[2] = true;
            displayResults();
        }
        if (countdownTime - pressedButtonTime > TIME_TO_HOLD_BUTTON_FOR_MENU){
            countdown = COUNTDOWN_START;
            functionOccuredOnce = true;
            pilotVersion.pilotMode = 0;
            
        } 
    }
    else if (isAnalogButtonPressed(analogButton2, readedButtonValue) && anyButtonPressed)
    {   
        countdownTime = millis();
        if(pilotVersion.pilotMode == CABLE_SIMULATING){
            
            if (countdownTime - pressedButtonTime > timeToHoldButton && !functionOccuredOnce){
                if(selectPPMode < 3){
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
        if(countdownTime - pressedButtonTime > SEK_1  &&  !sekDisplay[0]){
            countdown = 5;
            sekDisplay[0] = true;
            displayResults();
        }
        if(countdownTime - pressedButtonTime > SEK_2  &&  !sekDisplay[1]){
            countdown = 4;
            sekDisplay[1] = true;
            displayResults();
        }
        if(countdownTime - pressedButtonTime > SEK_3  &&  !sekDisplay[2]){
            countdown = 3;
            sekDisplay[2] = true;
            displayResults();
        }
        if(countdownTime - pressedButtonTime > SEK_4  &&  !sekDisplay[3]){
            countdown = 2;
            sekDisplay[3] = true;
            displayResults();
            
        }
        if(countdownTime - pressedButtonTime > SEK_5  &&  !sekDisplay[4]){
            countdown = 1;
            sekDisplay[4] = true;
            displayResults();

        }
        
        if (countdownTime - pressedButtonTime > TIME_TO_HOLD_BUTTON_FOR_ERROR  && !cpErrorOn){
            digitalWrite(CP_ERROR_PIN,HIGH);
            countdown = COUNTDOWN_START;
            cpErrorOn = true;
            displayResults();
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

 ////////////////////////////////////////////// CABLE SECTION   //////////////////////////////////////////////////////////  
    if(pilotVersion.pilotMode == NO_CABLE_SIMULATING){
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
                selectCPMode = 0;
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
/////////////////////////////////////////////   ERROR BUTTON SECTION  /////////////////////////////////////////////////////

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

/////////////////////////////////////////////   FREQUENCY SECTION   ///////////////////////////////////////////////////////////

     currentTimer = millis();

     if(currentTimer - previousTimer >= 100){
         previousTimer = currentTimer;

        frequencyValue = calculateFrequency(periodMeasurement);
        pulseWidthValue = calculatePulseWidth(periodMeasurement);
     }
    

    if(abs(abs(newFrequencyValue) - abs(frequencyValue)) > 10 
     || abs(abs(newPulseWidthValue) - abs(pulseWidthValue)) > 0.5){
         newFrequencyValue = frequencyValue;
         newPulseWidthValue = pulseWidthValue;
         displayResults();
     }
   

    if(!cpErrorOn){
        digitalWrite(CP_ERROR_PIN, LOW);
    }

    //  if( (frequencyValue < 1050.0F || frequencyValue > 950.0F) && frequencyBeyondTheNorm){
    //      frequencyBeyondTheNorm = false;
    //      displayResults();
    //  }
    //  else if((frequencyValue > 1050.0F || frequencyValue < 950.0F) && !frequencyBeyondTheNorm) {
    //      frequencyBeyondTheNorm = true;
    //      displayResults();
    //  }
    
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
    
    if(selectCPMode == 0){
        frequencyValue = 0;
        pulseWidthValue = 0;
    }
    display.setCursor(0, 13);
    display.println(F("Frequency: "));
    display.setCursor(65, 13);
    display.println(String(frequencyValue) + "Hz");
    display.setCursor(0, 23);
    display.println(F("Pulse Width: "));
    display.setCursor(78, 23);
    display.println(String(pulseWidthValue) + "%");

    if(countdown != COUNTDOWN_START){
        display.setCursor(120, 56);
        display.println(String(countdown));
    }
    if(cpErrorOn){
          display.setCursor(0, 38);
          display.println(F("E_CP_SHORT"));
    }
    if (errorButton1Pressed)
    {
      display.setCursor(0, 47);
      display.println("E_" + PP_DISCONECT_ERROR);
    }
    if (errorButton2Pressed)
    {
      display.setCursor(0, 56);
      display.println("E_"  + DIODE_ERROR );
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

void displaySelectiveMenu(){

    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(50,0 );
    display.println(F("Menu"));
    display.setCursor(0,10);
    display.println(F("Button 1"));
    display.setCursor(4,18);
    display.println(F("Only CP Simulate"));
    display.setCursor(0,30);
    display.println(F("Button 2"));
    display.setCursor(4,38);
    display.println(F("PP and CP Simulate"));
    display.display();
}


void readFrequency(){
    periodMeasurement[m_counter.counter][MICROS] = micros();
    periodMeasurement[m_counter.counter][STATE] = digitalRead(FREQUENCY_READ);
    m_counter.counter++;
}


float calculateFrequency(unsigned long periodMeasurement[][DATA_COUNT]){
    float averagePeriod[NUMBER_OF_MEASURMENTS /2] = {0,0,0,0,0,0,0,0};
    float average = 0;
    uint8_t measureCounter = 0;
    for(uint8_t i = 0; i <= NUMBER_OF_MEASURMENTS - 3; i++){
        if(periodMeasurement[i][STATE] == HIGH){
            averagePeriod[measureCounter] = periodMeasurement[i+2][MICROS] - periodMeasurement[i][MICROS];
            measureCounter++;            
        } else {
            continue;
        }  
     }
     selectionSort(averagePeriod, measureCounter);

  
     for(uint8_t i = 1; i <= measureCounter -3 ; i++){
         average += averagePeriod[i];
     }

     average = average / (measureCounter - 3);
     average = (1 / (average / 1000 )) * 1000;


     return average;
}

float calculatePulseWidth(unsigned long periodMeasurement[][DATA_COUNT]){
    float averagePulseWidth[NUMBER_OF_MEASURMENTS /2] = {0,0,0,0,0,0,0,0};
    float average = 0;
    uint8_t measureCounter = 0;
    for(uint8_t i = 0; i <= NUMBER_OF_MEASURMENTS - 3; i++){
        if(periodMeasurement[i][STATE] == HIGH){
            averagePulseWidth[measureCounter] = float(periodMeasurement[i+1][MICROS] - periodMeasurement[i][MICROS]) /
                                                float(periodMeasurement[i+2][MICROS] - periodMeasurement[i][MICROS]);

            measureCounter++;            
        } else {
            continue;
        }  
     }

      selectionSort(averagePulseWidth,measureCounter);
    
          for(uint8_t i = 2; i <= measureCounter -3 ; i++){
         average += averagePulseWidth[i];
     }

        average = average / (measureCounter - 4);

     if( average > 1){
         average = 1;

     }

    return 99 - ( average * 100 );    
}

void swap(float* xp, float* yp) 
{ 
    float temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
} 
  

void selectionSort(float arr[], uint8_t n) 
{ 
    uint8_t i, j, min_idx; 
  
    // One by one move boundary of unsorted subarray 
    for (i = 0; i < n - 1; i++) { 
  
        // Find the minimum element in unsorted array 
        min_idx = i; 
        for (j = i + 1; j < n; j++) 
            if (arr[j] < arr[min_idx]) 
                min_idx = j; 
  
        // Swap the found minimum element 
        // with the first element 
        swap(&arr[min_idx], &arr[i]); 
    } 
} 

void resetMeasurements(){
    for(uint8_t measure=0; measure<= NUMBER_OF_MEASURMENTS -1; measure++){
        for(uint8_t dataType = 0; dataType <= DATA_COUNT -1; dataType++) {
            periodMeasurement[measure][dataType] = 0;
        }
    }
}

void resetCountdown(){
    for(uint8_t i=0; i<=4; i++){
        sekDisplay[i] = false;
    }
}

void loadConfig() {

  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2])
    for (unsigned int t=0; t<sizeof(pilotVersion); t++)
      *((char*)&pilotVersion + t) = EEPROM.read(CONFIG_START + t);
}

void saveConfig() {
  for (unsigned int t=0; t<sizeof(pilotVersion); t++)
    EEPROM.write(CONFIG_START + t, *((char*)&pilotVersion + t));
}

void menu(){
    for(;;){
        if (isAnyAnalogButtonPressed() && !anyButtonPressed){
            anyButtonPressed = true;
            pressedButtonTime = millis();
        }
         else if (readedButtonValue == 0){
            anyButtonPressed = false;
            functionOccuredOnce = false; 
        }

        readedButtonValue = analogRead(CP_PP_MODE_SELECTOR);
        if (isAnalogButtonPressed(analogButton1, readedButtonValue) && anyButtonPressed){
            countdownTime = millis();
            if (countdownTime - pressedButtonTime > timeToHoldButton && !functionOccuredOnce){
                pilotVersion.pilotMode = NO_CABLE_SIMULATING;
                anyButtonPressed = true;
                functionOccuredOnce = true;
                saveConfig();
                return;
            }
        }
    
        if (isAnalogButtonPressed(analogButton2, readedButtonValue) && anyButtonPressed){
            countdownTime = millis();
            if (countdownTime - pressedButtonTime > timeToHoldButton && !functionOccuredOnce){
                pilotVersion.pilotMode = CABLE_SIMULATING;
                anyButtonPressed = true;
                functionOccuredOnce = true;
                saveConfig();
                return;
            }
        }

    }
}

