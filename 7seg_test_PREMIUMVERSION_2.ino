#include <Adafruit_Sensor.h>
#include <DHT.h> //Temperatur

//Realt Time Clock
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

//Temperatur
#define DHTPIN 2 //Pin Temperaturmesser
#define DHTTYPE DHT11
DHT dht = DHT(DHTPIN, DHTTYPE);

//IO Expander
#include <PCF8574.h>
PCF8574 pcf20(0x20);

/* ********************************************************************** 
 * Four Digit Hex Counter
 *   Uses: one 74HC595 shift register
 *         one four digit seven segment display - common cathode (5641AS)
 *         four PN2222 transistors
 *  
 *  Verify circuit connections:
 *           74HC595 pin     Q7,Q6,Q5,Q4,Q3,Q2,Q1,Q0 
 *           Mapping to       g, c,DP, d, e, b, f, a (7-Segment LED)
 *           for array purposes D4 = digit0, etc.
 ********************************************************************* */

/* ***************************************************
 *                Global Constants                   *
 *************************************************** */
const int dataPin  = 12;  // 74HC595 pin 8 DS
const int latchPin = 11;  // 74HC595 pin 9 STCP
const int clockPin = 9;   // 74HC595 pin 10 SHCP

const int digit0   = 7;   // 7-Segment pin D4
const int digit1   = 6;   // 7-Segment pin D3
const int digit2   = 5;   // 7-Segment pin D2
const int digit3   = 4;   // 7-Segment pin D1 

/* ***************************************************
 *                Global Variables                   *
 *************************************************** */
// Hex values reference which LED segments are turned on
// and may vary from circuit to circuit.  Note the mapping above.
byte table[]= 
    {   0xD7,  // = 0
        0x14,  // = 1
        0xCD,  // = 2
        0x5D,  // = 3
        0x1E,  // = 4
        0x5B,  // = 5
        0xDB,  // = 6
        0x15,  // = 7
        0xDF,  // = 8
        0x5F,  // = 9
        0x9F,  // = A
        0xDA,  // = b
        0xC3,  // = C
        0xDC,  // = d
        0xCB,  // = E
        0x8B,  // = F
        0x0F,  // = grad symbol
        0x00   // blank
        
    };  //Hex shown edpcgbfa Binary pin 7seg
byte digitDP = 32;  // 0x20 - adds this to digit to show decimal point
byte controlDigits[] = { digit0, digit1, digit2, digit3 };  // pins to turn off & on digits
byte displayDigits[] = { 0,0,0,0,0 }; // ie: { 1, 0, 7, 13, 0} == d701 (all values from table array)
    /* Each array value holds digit values as table array index, or raw byte
     *  parameters: digit0, digit1, digit2, digit3, digitSwitch
     *  
     * digitSwitch: the four least significant bits controls data handling, 
     *              each bit controls associated digit
     *              starting with least-significant bit 0, 
     *              i.e. B1010, digit1 & digit3 are raw, 
     *                          digit0 & digit2 use table array 
     *       1 = raw byte
     *       0 = table array index                                         */  
unsigned long onTime = 0;             // tracks time
unsigned int counter = 0;             // RawDisplay counter

/* ***************************************************
 *           Global Adjustable Variables             *
 *************************************************** */
int digitDelay = 250;                  // delay between incrementing digits (ms)
int brightness = 90;                  // valid range of 0-100, 100=brightest
//unsigned int ShowSegCount = 250;      // number of RawDisplay loops before switching again 
int delay_num = 100;
//bool commonCathode = true;

int temp;    //Variables for Temp and Humidity
int humid;

tmElements_t current_time;

/* ***************************************************
 *                   Void Setup                      *
 *************************************************** */



void setup() {
    dht.begin(); // Temp Sensor setup
    pcf20.begin(); //IO Expander setup

  //Display Setup
    pinMode(latchPin,OUTPUT);
    pinMode(clockPin,OUTPUT);
    pinMode(dataPin,OUTPUT);
    for (int x=0; x<4; x++){
        pinMode(controlDigits[x],OUTPUT);
        digitalWrite(controlDigits[x],LOW);  // Turns off the digit  
    }
}

/* ***************************************************
 *                   Functions                       *
 *************************************************** */    
void DisplaySegments(){
    /* Display will send out all four digits
     * one at a time.  Elegoo kit only has 1 74HC595, so
     * the Arduino will control the digits
     *   displayDigits[4] = the right nibble controls output type
     *                      1 = raw, 0 = table array
     *                  upper (left) nibble ignored
     *                  starting with 0, the least-significant (rightmost) bit
     */
    
    for (int x=0; x<4; x++){
        for (int j=0; j<4; j++){
            digitalWrite(controlDigits[j],LOW);    // turn off digits
        }
        digitalWrite(latchPin,LOW);
        if (bitRead(displayDigits[4],x)==1){
            // raw byte value is sent to shift register
            shiftOut(dataPin,clockPin,MSBFIRST,displayDigits[x]);
        } else {
            // table array value is sent to the shift register
            shiftOut(dataPin,clockPin,MSBFIRST,table[displayDigits[x]]);
        }
        
        digitalWrite(latchPin,HIGH);
        digitalWrite(controlDigits[x],HIGH);   // turn on one digit
        delay(1);                              // 1 or 2 is ok
        if (delay_num>1) {
          delay_num--;
        } else {
          delay_num=100;
        }
    }
    for (int j=0; j<4; j++){
        digitalWrite(controlDigits[j],LOW);    // turn off digits
    }
}

void RDisplay(){
    // HALO edpcgbfa
    displayDigits[0] = table[7];  // 1 //ganz rechts
    displayDigits[1] = table[16];  // 3 //mitte rechts
    //displayDigits[2] = B00110100;  // 1 B00010100 1. B00110100 //mittelinks
    displayDigits[3] = table[8];  // 2 //links
     // Set digitSwitch option
    displayDigits[4] = B1111;


    if (counter %3 == 0) {
       displayDigits[2] = B00010100;
       counter++;
    } else {
      displayDigits[2] = B00110100;
      counter++;
    }
}

void RandD_temp(){

    int tens;
    int ones;
    // Read values
    temp = dht.readTemperature();
    humid = dht.readHumidity();

    //Display values
    //convert temp and humid in displayable format
    Serial.begin(9600);
    Serial.println(temp);
    
    // Show °C
    displayDigits[0] = table[12]; //C
    displayDigits[1] =table[16]; //grad
    
    //show temp
    ones = temp % 10;
    Serial.println(ones);
    tens = (temp - ones)/10;
    Serial.println(tens);
    displayDigits[3] = table[tens];
    displayDigits[2] = table[ones];

    displayDigits[4] = B1111;
}

void Display_Time(int time_h,int time_min){
    int h_t, h_o;
    int min_t, min_o;

    Serial.begin(9600);
    
    
    //divide time in digits
    h_o = time_h % 10;
    min_o = time_min % 10;
    h_t = (time_h - h_o)/10;
    min_t = (time_min - min_o)/10;

    Serial.println(min_t);

    //display time
    displayDigits[3] = table[h_t];
    displayDigits[2] = table[h_o];
    displayDigits[1] = table[min_t];
    displayDigits[0] = table[min_o];

    displayDigits[4] = B1111;

    //blinkender punkt
    //TO DO
 
}

void showTime() {
  RTC.read(current_time);
  Display_Time(current_time.Hour, current_time.Minute);
  }

void changeTime() {
  tmElements_t tm;
  tm.Hour = 17;
  tm.Minute = 53;
  RTC.write(tm);
  }

/* ***************************************************
 *                   Void Loop                       *
 *************************************************** */
void loop() {

    
    DisplaySegments();                                      // Caution: Avoid extra delays
    /* *************************************
     *         Control Brightness          *
     * *********************************** */
    delayMicroseconds(1638*((100-brightness)/10));         // largest value 16383
    
    /* *************************************
     *        Selects Display Type         *
     * *********************************** */
    unsigned long nowValue = millis() - onTime;
    if (nowValue >= long(digitDelay)){
        onTime = millis();
        //changeTime();
        showTime();
        //Display_Time(12,15);
        //RDisplay();
        //RandD_temp();

        //IO Esp Tests
        pcf20.write8(LOW);
        pcf20.write(0, HIGH);
    }

    
}
