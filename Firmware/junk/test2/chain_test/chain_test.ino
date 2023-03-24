//EEPROM will be used to store list.
#include <EEPROM.h>
#include <DS3231.h>   

//rotally switch
#define outputA 8 //right
#define outputB 7 //left
#define outputC 9 //push
//variables for rotary switch
int counter = 0; 
int aState;
int aLastState;  

// Date and time functions using a PCF8523 RTC connected via I2C and Wire lib
#include "RTClib.h"
RTC_PCF8523 rtc;

//16segLEG controller
#include <TLC5926.h>

//Pin definition
const int SDI_pin = 2;
const int CLK_pin = 3;
const int LE_pin  = 4;
const int iOE_pin = 5;  // put this on a PWM pin
const int SDO_pin = 6;  // put this on a PWM pin
const int CHAIN_CT = 2; // number of digits
const int TEST_NUM = 1;
TLC5926 tlc;

//light sensor photocell 27-66kohm
int lightSensorPin = A0;
int analogValue = 0;


void setup() {

  //rotary switch
  pinMode (outputA,INPUT);
  digitalWrite(outputA, HIGH); // turn on internal pullup resistor
  pinMode (outputB,INPUT);
  digitalWrite(outputB, HIGH); // turn on internal pullup resistor
  pinMode (outputC,INPUT);
  digitalWrite(outputC, HIGH); // turn on internal pullup resistor
  // Reads the initial state of the outputA
  aLastState = digitalRead(outputA); 
  
  while (!Serial) {
    delay(1);  // for Leonardo/Micro/Zero
  }

  Serial.begin(9600); //for warnings

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.initialized()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  
  // turn on warnings. probably turn it off when in production
  tlc.debug(1);
  tlc.attach(CHAIN_CT, SDI_pin, CLK_pin, LE_pin, iOE_pin, SDO_pin);
  tlc.off();
  tlc.reset();
  tlc.brightness(100);
}

/*
 Look up table for LTP-587G (might be changed)
 //BAMKHGTFESRDUPCN  OUT0 - OUT15 
 */

word char2word(char input){
  word segtable[68]= {
    //BAMKHGTFESRDUPCN
    0b0000000000000000, //' '
    0b0000000000010010, //'!'
    0b0010000000000010, //'"'
    0b0010110111011110, //'#'
    0b1110100111011100, //'$'
    0b0110101011011101, //'%'
    0b0111010110101000, //'&'
    0b0010000000000000, //'''
    0b0000000000100001, //'('
    0b0001001000000000, //')'
    0b0011001001101101, //'*'
    0b0010000001001100, //'+'
    0b0000001000000000, //','
    0b0000000000001100, //'-'
    0b0000000001000000, //'.'
    0b0000001000000001, //'/'
    0b1100111110010011, //'0'
    0b0000000000010011, //'1'
    0b1100010110001110, //'2'
    0b1100000110010110, //'3'
    0b0000100000011110, //'4'
    0b1100100110101000, //'5'
    0b1100110110011100, //'6'
    0b1100000000010010, //'7'
    0b1100110110011110, //'8'
    0b1100100110011110, //'9' 
    0b0010000001000000, //':'
    0b0010001000000000, //';'
    0b0000000001000001, //'<'
    0b0000000110001100, //'='
    0b0001001000000000, //'>"
    0b1100000001000110, //'?"
    0b1110110110000100, //'@"
    0b1100110000011110, //'A'
    0b1110000111010110, //'B'
    0b1100110110000000, //'C'
    0b1110000111010010, //'D'
    0b1100110110001000, //'E'
    0b1100110000001000, //'F'
    0b1100110110010100, //'G'
    0b0000110000011110, //'H'
    0b1110000111000000, //'I'
    0b0000010110010010, //'J'
    0b0000110000101001, //'K'
    0b0000110110000000, //'L'
    0b0001110000010011, //'M'
    0b0001110000110010, //'N'
    0b1100110110010010, //'O'
    0b1100110000001110, //'P'
    0b1100110110110010, //'Q'
    0b1100110000101110, //'R'
    0b1100100110011100, //'S'
    0b1110000001000000, //'T'
    0b0000110110010010, //'U'
    0b0000111000000001, //'V'
    0b0000111000110010, //'W'
    0b0001001000100001, //'X'
    0b0000100110011110, //'Y'
    0b1100001110000001, //'Z'
    0b1010000011000000, //'['
    0b0001000000100000, //'\'
    0b0110000101000000, //']'
    0b0000001000100000, //'^'
    0b0000000110000000, //'_'
    0b0001000000000000, //'`'
  };

  if(input >= ' ' && input <= '`'){
    return(segtable[(int)(input-' ')]);
  }else{
     return(0);
  }
    
}


void print16seg(String input){
  int i=0;

  for(i=input.length()-1;i>=0;i--){
    tlc.send(char2word(input.charAt(i)));
    //tlc.delay(500);
  }
  //tlc.latch_pulse();
}

//RTC
int pretime = -1;

// A pattern of alternating on/off
const word on_off_pattern = 0xAAAA; // 16 bits. beware signed int's! high int may not get set!
// ... off/on
const word off_on_pattern = 0x5555;
const int marguee_pattern = 0b11101110;

int first = 1; // first time in loop for initing

void loop() {

  if(!digitalRead(outputC)){
    Serial.print("press");
  }
  
  //tlc.all(LOW);
  //check dialsoooooooooooooooooooooo
  aState = digitalRead(outputA); // Reads the "current" state of the outputA
  // If the previous and the current state of the outputA are different, that means a Pulse has occured
  if (aState != aLastState){     
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(outputB) != aState) { 
      counter++;
    } else {
      counter--;
    }
    
    tlc.send(char2word(counter + 'A'));
     Serial.print("Position: ");
     Serial.println(counter);
  } 
  aLastState = aState; // Updates the previous state of the outputA with the current state

  //obtain data from PCF8523

  char* buf = "10";
  DateTime now = rtc.now();
  int newtime = 0;
  
  if(counter % 3 == 0 ){
    newtime = now.hour();
  }else if(counter % 3 == 1){
    newtime = now.minute();
  }else{
    newtime = now.second();
  }

  if(pretime != newtime){
    sprintf(buf,"%02d",newtime);
    //Serial.println(buf);
    print16seg(buf);
    pretime = newtime;
  }

  analogValue = analogRead(lightSensorPin);
  Serial.println(analogValue);
  tlc.brightness(analogValue/10.23);

  //Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  
  switch (TEST_NUM) {
  //my code.
  case 1:
    int i=0;
    //print16seg("BT");
    break;
    
  case 2:
    tlc.all(HIGH)->delay(900)
      ->all(LOW)->delay(100)
        ;        
    break;

    // OE test - Should turn on/off & PWM dim
  case 3:

    if (first) { 
      tlc.all(HIGH); 
      tlc.off(); 
    }
    Serial.print(first);
    tlc.on()->delay(900)
      ->off()->delay(100)
        ;
    tlc.on()->delay(500)
      ->brightness(100)->delay(500)
        ->brightness(50)->delay(500)
          ->brightness(10)->delay(500)
            ;
    break;

    // LE test - Should have NO flicker
  case 4:
    tlc.all(LOW)->delay(500);
    for (int i=0; i<CHAIN_CT; i++) {
      tlc.shift(0xFFFF); // no flicker here
    }
    tlc.delay(400);
    tlc.latch_pulse();
    tlc.delay(500);
    break;

    // SDO test, only 1 shift-register -- Check the serial monitor to confirm
    // Blinks while reading the SDO
  case 5:
    {
      tlc.on();
      unsigned int pattern;
      pattern = 0b0011000111001101;
      tlc.send(pattern); // prime it, SDO is 0 (MSB)

      Serial.println("Want");
      Serial.println("| Saw");
      Serial.println("| | Good?");

      int fill = 1;
      for(int i=0; i<16; i++) {
        int want = bitRead(pattern, 15-i); // SDO shows MSB
        pattern >> 1;
        int val = tlc.read_sdo();
        Serial.print(want); 
        Serial.print(" ");
        Serial.print(val); 
        Serial.print(" ");
        Serial.println(val == want ? 1 : 0);

        // we'll fill w/ 1010..., slow enough to see
        tlc.send_bits(1,fill, 100);
        fill = fill ? 0 : 1;
      }
      tlc.all(HIGH)->delay(400)->flash()->delay(400);
    }
    break;

    // error-detect - short 1 pin, load 1 pin, leave 1 pin open
  case 6:
    {
      int status;
      if (first) tlc.all(HIGH)->on()->delay(300);
      status = tlc.error_detect();
      Serial.print("Error Status: ");
      Serial.println(status,BIN);
      tlc.all(HIGH)->on()->delay(10000);
    }
    break;

    // Use the current-gain-multiplier to set the range down
    // Does a bunch of blinking to confirm mode switch
  case 7:
    Serial.println("On Max");
    tlc.config(1,1,127);
    tlc.all(HIGH)->on()->delay(1000);

    Serial.println("On Min");
    tlc.config(0,0,0);
    tlc.all(HIGH)->on()->delay(1000);

    // This confirms that we are back in normal mode
    Serial.println("shifting");
    tlc.all(LOW)->delay(300);
    tlc.all(HIGH)->delay(300);
    tlc.all(LOW)->delay(300);
    tlc.all(HIGH)->delay(300);
    tlc.all(LOW)->delay(300);
    tlc.all(HIGH)->delay(300);
    tlc.all(LOW)->delay(300);

    break;

  // Assuming config() test above works,
  // This shows what it looks like to set config
  // without delays,
  // And the full-range effect
  // Set tlc.debug(0): should be smooth (no flash)
  case 8:
    if (first) {Serial.println("FIRST"); tlc.debug(0); tlc.all(HIGH)->on(); }
    Serial.println("Segment ends");
    tlc.config(1,1,127)->on()->all(HIGH)->delay(200);
    tlc.config(0,1,127)->on()->all(HIGH)->delay(200);
    tlc.config(1,0,127)->on()->all(HIGH)->delay(200);
    tlc.config(0,0,127)->on()->all(HIGH)->delay(200);
    tlc.flash();
    
    // Full range
    Serial.println("Full range");
    for (int cm=1; cm>=0; cm--) {
      for (int vb=1; vb>=0; vb--) {
        for (int vg=127; vg>=0; vg -=16) {
          tlc.config(cm,vb,vg)->on()->all(HIGH)->delay(200);
        }
      }
    }
    break;
  
  // Calibrate the trim-pot vs. current-gain
  // Does Max-mid-min so you can see what effect trimpot has
  case 9:
    if (first) tlc.all(HIGH)->on();
    tlc.config(1,1,127)->on()->all(HIGH)->delay(400);
    tlc.config(1,0,0)->on()->all(HIGH)->delay(400);
    tlc.config(0,0,0)->on()->all(HIGH)->delay(400);
    break;
    
  case 10:
     break;
  }
  
  first = 0;
}          
