#include <Arduino.h>
#include <math.h>
#include <AnalogButton.h>
#include <buttonHandler.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <time.h>

#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);
#if (SH1106_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SH1106.h!");
#endif

typedef struct {
    uint8_t counter : 4;
}Counter;


#define TOLLERANCE 5
#define BUTTON_1_VALUE 769
#define BUTTON_2_VALUE 616
#define BUTTON_3_VALUE 514
#define BUTTON_4_VALUE 440


#define NO_CABLE_CONNECTION 823                                      //823

#define CABLE_32A_MIN 175                                            //175             /// dla pilota bez mosfetów 180
#define CABLE_32A_MAX 314                                            //314                 //310

#define CABLE_20A_MIN 314                                          //314                //320
#define CABLE_20A_MAX 532                                          //532                //530

#define CABLE_13A_MIN 534                                            //534
#define CABLE_13A_MAX 685                                           //685

#define DELAY_BEFORE_CHANGES 200 

const AnalogButton analogButton1(BUTTON_1_VALUE - TOLLERANCE, BUTTON_1_VALUE + TOLLERANCE);
const AnalogButton analogButton2(BUTTON_2_VALUE - TOLLERANCE, BUTTON_1_VALUE + TOLLERANCE);
const AnalogButton analogButton3(BUTTON_3_VALUE - TOLLERANCE , BUTTON_3_VALUE + TOLLERANCE);
const AnalogButton analogButton4(BUTTON_4_VALUE - TOLLERANCE, BUTTON_4_VALUE + TOLLERANCE);

const AnalogButton noCableAnalogInput(NO_CABLE_CONNECTION - TOLLERANCE, NO_CABLE_CONNECTION + TOLLERANCE);
const AnalogButton cable32A_AnalogInput(CABLE_32A_MIN, CABLE_32A_MAX);
const AnalogButton cable20A_AnalogInput(CABLE_20A_MIN, CABLE_20A_MAX);
const AnalogButton cable13A_AnalogInput(CABLE_13A_MIN, CABLE_13A_MAX);


boolean isNoCableDisplayedRecently = false;
boolean isCable_32ADisplayedRecently = false;
boolean isCable_20AisplayedRecently = false;
boolean isCable_13ADisplayedRecently = false;

boolean timerStarted = true;
boolean isPPValueDisplayed = false;

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
uint8_t countdown = 0;
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
uint8_t selectPilotMode = 1;


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
void startingSelectiveMenu();
void readFrequency();
void swap(float* xp, float* yp);
void selectionSort(float arr[], uint8_t n);
void resetMeasurements();
void resetCountdown();

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

    pinMode(CAR, OUTPUT);
    pinMode(CHARGE, OUTPUT);
    pinMode(CHARGECOOL, OUTPUT);
    pinMode(CP_ERROR_PIN,OUTPUT);
    digitalWrite(CP_ERROR_PIN,LOW);

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
}

void loop()
{   
    readedButtonValue = analogRead(CP_PP_MODE_SELECTOR);


     currentTimer = millis();

     if(currentTimer - previousTimer >= 100){
         previousTimer = currentTimer;

        frequencyValue = calculateFrequency(periodMeasurement);
        pulseWidthValue = calculatePulseWidth(periodMeasurement);
     }

    Serial.println(digitalRead(FREQUENCY_READ));

        

    if(abs(abs(newFrequencyValue) - abs(frequencyValue)) > 10 
     || abs(abs(newPulseWidthValue) - abs(pulseWidthValue)) > 0.5){
         newFrequencyValue = frequencyValue;
         newPulseWidthValue = pulseWidthValue;
         displayResults();
     }
   

    

    //  if( (frequencyValue < 1050.0F || frequencyValue > 950.0F) && frequencyBeyondTheNorm){
    //      frequencyBeyondTheNorm = false;
    //      displayResults();
    //  }
    //  else if((frequencyValue > 1050.0F || frequencyValue < 950.0F) && !frequencyBeyondTheNorm) {
    //      frequencyBeyondTheNorm = true;
    //      displayResults();
    //  }

    readedButtonValue = analogRead(CP_PP_MODE_SELECTOR);
    readedPPVaule = analogRead(PP_CABLE_VOLTAGE);


    if (isAnyAnalogButtonPressed()){
        anyButtonPressed = true;
        pressedButtonTime = millis();
        if(cpErrorOn) cpErrorOn = false;
        resetCountdown();
        
    }
    else if (readedButtonValue == 0)
    {   
        if(countdown > 0){
            countdown = 0;
            displayResults();
        }
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
        if(countdownTime - pressedButtonTime > SEK_1  &&  !sekDisplay[0]){
            countdown = 1;
            sekDisplay[0] = true;
            displayResults();
        }
        if(countdownTime - pressedButtonTime > SEK_2  &&  !sekDisplay[1]){
            countdown = 2;
            sekDisplay[1] = true;
            displayResults();
        }
        if(countdownTime - pressedButtonTime > SEK_3  &&  !sekDisplay[2]){
            countdown = 3;
            sekDisplay[2] = true;
            displayResults();
        }
        if(countdownTime - pressedButtonTime > SEK_4  &&  !sekDisplay[3]){
            countdown = 4;
            sekDisplay[3] = true;
            displayResults();
            
        }
        if(countdownTime - pressedButtonTime > SEK_5  &&  !sekDisplay[4]){
            countdown = 5;
            sekDisplay[4] = true;
            displayResults();

        }
        
        if (countdownTime - pressedButtonTime > TIME_TO_HOLD_BUTTON_FOR_ERROR  && !cpErrorOn){
            digitalWrite(CP_ERROR_PIN,HIGH);
            countdown = 0;
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
    
    if(!cpErrorOn){
        digitalWrite(CP_ERROR_PIN, LOW);
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
    display.setCursor(0, 18);
    if(selectCPMode == 0){
        frequencyValue = 0;
        pulseWidthValue = 0;
    }
    display.println("Frequency: " + String(frequencyValue) + " Hz");
    display.setCursor(0, 36);
    display.println("Pulse Width: " + String(pulseWidthValue) + " %");
    if(countdown != 0){
        display.setCursor(64, 52);
        display.println(String(countdown));
    }
    if(cpErrorOn){
          display.setCursor(0, 52);
          display.println(F("CP short circut"));
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

     for(uint8_t i = 2; i<= measureCounter -3 ; i++){
         average += averagePulseWidth[i];
         Serial.println(averagePulseWidth[i]);
     }
         delay(100);
   
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