#include <RTClib.h>

//#define DS1307_ADDRESS 0x68 // I²C Address for the RTC module

/* ***************************************************
 *                Global Constants                   *
 *************************************************** */
//Display pins
const byte display_clockPin = 9;   // clock/ latch pin (shift register)
const byte display_dataPin = 10;   // data pin (shift register)
const byte display_digit_pins[] = {5,6,7,8}; // pins for the four digits

//MUX
const byte muxA = 2;
const byte muxB = 3;
const byte muxC = 4;
const byte muxcom = 11;

//Buzzer
const byte buzzer_pin = 13;

//Button 1
const byte button1 = 12;

//Tabelle über Zahlen
//reihnfolge der segmente in binary bestimmen und jeweils die stellen in der zahlen auf 1 setzen zum aktiveren 
const byte display_num[]=
  {
    0b00000000, // = 0
    0b00000000, // = 1
    0b00000000, // = 2
    0b00000000, // = 3
    0b00000000, // = 4
    0b00000000, // = 5
    0b00000000, // = 6
    0b00000000, // = 7
    0b00000000, // = 8
    0b00000000, // = 9
  };

/* ***************************************************
 *                Global Variables                   *
 *************************************************** */
RTC_DS1307 rtc; //RTC Class instance
DateTime weck_zeit; //time for alarm

//Es gibt 3 Schalter welche eine binärzahl von 0 bis 7 darstellen 
//000 
//001 
//010 
//011 
//100
//101
//110
//111 
//Stunden Knopf ist auf DPin 12
//Minuten Knopf ist auf Mux pin 0 000
byte wecker_modus = 0;

/* ***************************************************
 *                   Functions                       *
 *************************************************** */ 
 
//aktiviert alle 2ms das display und zeigt die gewünschten zahlen an
void startDisplayControl() {    
}


//setzt Multiplexer auf angegeben pin
void setMux(byte num) {
}

//kümmert sich um das timing für den multiplexer
void timeMux(byte num) {
}

void zeitEinstellen() {

}

void weck_zeitEinstellen() {
  
}

aktiviert deaktivert den alarm
void alarm() {
}
  
void setup() {
//-------------------------------
//Setup Serial 
//-------------------------------  
    Serial.begin(9600);
    Serial.println("Hello");


//-------------------------------
//Setup display
//-------------------------------
 
//-------------------------------
//Setup RTC 
//-------------------------------     
    rtc.begin();
    //rtc.adjust(DateTime(2022, 9, 28, 0, 15, 0)); //sets time
    
//-------------------------------
//Setup mux 
//-------------------------------
    pinMode(muxcom,INPUT);
    
//-------------------------------
//Setup Sonstiges 
//-------------------------------
   pinMode(buzzer_pin,OUTPUT);
   pinMode(button1,INPUT);
  
}



void loop() {
    startDisplayControl(); //aktiviert in jedem durchlauf das display
    
      
}
