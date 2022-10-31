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
//reihnfolge der segmente in binary  
//gfedcbaP
const byte display_num[]=
  {
    0b10000001, // = 0
    0b11110011, // = 1
    0b01001001, // = 2
    0b01100001, // = 3
    0b00110011, // = 4
    0b00100101, // = 5
    0b00000101, // = 6
    0b11110001, // = 7
    0b00000001, // = 8
    0b00100001, // = 9
  };

/* ***************************************************
 *                Global Variables                   *
 *************************************************** */

byte display_digits[] = {0,0,0,0}; //write here what to display in each digit
int s_time;

RTC_DS1307 rtc; //RTC Class instance
DateTime weck_zeit(2050, 1, 1, 8, 15, 0); //time for alarm
DateTime neue_zeit; // new time for changing time


//There are 3 switches leading to 8 states represented as a binary number by the switches
//The Pin in the middle is connected to MuxPin 1, 2 and 3
//alle  upper pins to GND all lower pins to 5V
//000 show time normaly
//001 change time witch buttons
//010 activate alarm
//011 set alarm
//100 show date
//101 show year
//110
//111 deativate alarm

//Hour button is on DPin 12
//Minute Button is on Mux pin 0 000
byte wecker_modus = 0;
bool wecker_modus_array[] = {0,0,0}; //binary digits of wecker_modus

int mux_state = 0;  // current state of multiplexer, which pin is connectet to COM
unsigned long mux_time = 0;   // time at last multiplexer change
int mux_highest_pin = 3; //highest mux pin used

unsigned long button_time; //time at last check for button press

/* ***************************************************
 *                   Functions                       *
 *************************************************** */ 
 
//activates every digit for 2ms and shows value from display_digits[]
void startDisplayControl() {    
  for (int i = 0; i < 4; i++) {
    shiftOut(display_dataPin, display_clockPin, MSBFIRST, display_digits[i]);
    digitalWrite(display_digit_pins[i], LOW);
    delay(1);
    digitalWrite(display_digit_pins[i], HIGH);
  }
}

//easy way to change display digits
void writeDisplay(int a,int b,int c,int d) {
    display_digits[0] = display_num[a]; 
    display_digits[1] = display_num[b];
    display_digits[2] = display_num[c];
    display_digits[3] = display_num[d];
}

//sets Multiplexer to given pin number
void setMux(byte num) {
    int c = LOW;  //define default as LOW 
    int b = LOW;
    int a = LOW;
    
    if(num & 4) {c = HIGH;} //num in binary is "0b00000cba" as 3 digits make number 0-7
    if(num & 2) {b = HIGH;} //bitwise and (&) : uses and operation on every bit, "if = True for >=1 
    if(num & 1) {a = HIGH;} // binary: 4 = 0b0000 0100, 2 = 0b0000 0010, 1 = 0b0000 0001

    digitalWrite(muxA,a);
    digitalWrite(muxB,b);
    digitalWrite(muxC,c);

    mux_state = num;
}

//changes mux every 50 ms to check for changes
void timeMux() {
  if((millis() - mux_time) < 5) {return;} //checks if 5ms has passed
  else {
    mux_state++;          //sets next mux state
    if (mux_state == mux_highest_pin + 1) { //resets mux state after max
      mux_state = 0; 
     }
     
    setMux(mux_state);          //sets mux to number from mux_count
    mux_time = millis();  //resets timer for next mux state
  }
}


//debounces button an regulates input speed
//min. 100ms to debounce (entprellen)
//muxcom with muxcom state 0 increments hour
//button1 increments minute
void setWeck_zeit() {
  if((millis() - button_time) < 500) {return;} //checks if 500ms has passed
  else {
    button_time = millis();
    bool min_button = digitalRead(button1);
    bool hour_button = digitalRead(muxcom);
    
    if((mux_state = 0) && (hour_button)) {
      weck_zeit = weck_zeit + TimeSpan(0, 1, 0, 0);
    }
        if(min_button) {
      weck_zeit = weck_zeit + TimeSpan(0, 0, 1, 0);
    }   
  }
}

void setRTC_time() {
  if((millis() - button_time) < 500) {return;} //checks if 500ms has passed
  else {
    button_time = millis();
    bool min_button = digitalRead(button1);
    bool hour_button = digitalRead(muxcom);
    
    if((mux_state = 0) && (hour_button)) {
      neue_zeit = neue_zeit + TimeSpan(0, 1, 0, 0);
    }
        if(min_button) {
      neue_zeit = neue_zeit + TimeSpan(0, 0, 1, 0);
    }
    rtc.adjust(neue_zeit);   
  }
}

//checks for input then , adjust array and then sums over array
void set_wecker_mode() {
  if((mux_state > 0) && (mux_state < 4)) {
  bool mux_input = digitalRead(muxcom);
  wecker_modus_array[mux_state - 1] = mux_input; 
  wecker_modus = wecker_modus_array[0]*1 + wecker_modus_array[1]*2 + wecker_modus_array[2]*4;
 }
}

void alarm(bool alarm) {
  if(alarm) {
    tone(buzzer_pin, 420);
  } else {
    noTone(buzzer_pin);
    digitalWrite(buzzer_pin,LOW);
  }
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
    pinMode(display_clockPin, OUTPUT);
    pinMode(display_dataPin, OUTPUT);
    
    for (int i = 0; i < 4; i++) {
      pinMode(display_digit_pins[i], OUTPUT);       //Sets all display pins to OUTPUT
      digitalWrite(display_digit_pins[i], HIGH);    //and disables them (HIGH = OFF, LOW = ON)
     }

//-------------------------------
//Setup RTC 
//-------------------------------     
    rtc.begin();
    //rtc.adjust(DateTime(2022, 9, 28, 0, 15, 0)); //sets time
    
    
//-------------------------------
//Setup mux 
//-------------------------------
    pinMode(muxA,OUTPUT);
    pinMode(muxB,OUTPUT);
    pinMode(muxC,OUTPUT);
    pinMode(muxcom,INPUT);
    
    setMux(0);


//-------------------------------
//Setup Sonstiges 
//-------------------------------
   pinMode(buzzer_pin,OUTPUT);
   pinMode(button1,INPUT);
   noTone(buzzer_pin);
   digitalWrite(buzzer_pin,LOW);
}



void loop() {
    startDisplayControl(); //starts display digit cycle and displays values in "display_digits"
    s_time = millis()/1000; //time since start in seconds //DEBUG writeDisplay((s_time/1000)%10, (s_time/100)%10, (s_time/10)%10, s_time%10);
       
    DateTime zeit = rtc.now(); //gets current time from RTC
    

    //times mux and changes state of wecker
    timeMux();
    set_wecker_mode();
    Serial.println(wecker_modus);

    
    switch(wecker_modus) {
      case 0:     //show time normally
        writeDisplay(zeit.hour()/ 10, zeit.hour() % 10, zeit.minute()/ 10, zeit.minute() % 10); 
        break;
      case 1:     //change time
        writeDisplay(zeit.hour()/ 10, zeit.hour() % 10, zeit.minute()/ 10, zeit.minute() % 10); 
        neue_zeit = rtc.now();
        setRTC_time();
        break;
      case 2:     //010 activate alarm
        writeDisplay(zeit.hour()/ 10, zeit.hour() % 10, zeit.minute()/ 10, zeit.minute() % 10); 
        if ( (weck_zeit.hour() > zeit.hour() ) && ( weck_zeit.minute() > zeit.minute() ) ) 
        break;
      case 3:     //01)1 set alarm
        writeDisplay(weck_zeit.hour()/ 10, weck_zeit.hour() % 10, weck_zeit.minute()/ 10, weck_zeit.minute() % 10); 
        setWeck_zeit();
        break;
      case 4:      //100 show date
        writeDisplay(zeit.day()/ 10, zeit.day() % 10, zeit.month()/ 10, zeit.month() % 10);
        break;
      case 5:      //101 show year
        writeDisplay((zeit.year()/1000)%10, (zeit.year()/100)%10, (zeit.year()/10)%10, zeit.year()%10);
        break;
      case 6:
        //unused
        break;
      case 7:      //111 deactivate alarm
        alarm(0);
        break;
      }
      
}
