#include <Wire.h>
#include <DS3231.h>

DS3231 rtc;

byte Year;
byte Month;
byte Date;
byte DoW;
byte Hour;
byte Minute;
byte Second;

byte level = 20;

void GetDateStuff(byte& Year, byte& Month, byte& Day, byte& DoW, 
    byte& Hour, byte& Minute, byte& Second) {
  // Call this if you notice something coming in on 
  // the serial port. The stuff coming in should be in 
  // the order YYMMDDwHHMMSS with an 'x' at the end.
  // 211210w042420x
  boolean GotString = false;
  char InChar;
  byte Temp1, Temp2;
  char InString[20];

  byte j=0;
  while (!GotString) {
    if (Serial.available()) {
      InChar = Serial.read();
      InString[j] = InChar;
      j += 1;
      if (InChar == 'x') {
        GotString = true;
      }
    }
  }
  Serial.println(InString);
  // Read Year first
  Temp1 = (byte)InString[0] -48;
  Temp2 = (byte)InString[1] -48;
  Year = Temp1*10 + Temp2;
  // now month
  Temp1 = (byte)InString[2] -48;
  Temp2 = (byte)InString[3] -48;
  Month = Temp1*10 + Temp2;
  // now date
  Temp1 = (byte)InString[4] -48;
  Temp2 = (byte)InString[5] -48;
  Day = Temp1*10 + Temp2;
  // now Day of Week
  DoW = (byte)InString[6] - 48;   
  // now Hour
  Temp1 = (byte)InString[7] -48;
  Temp2 = (byte)InString[8] -48;
  Hour = Temp1*10 + Temp2;
  // now Minute
  Temp1 = (byte)InString[9] -48;
  Temp2 = (byte)InString[10] -48;
  Minute = Temp1*10 + Temp2;
  // now Second
  Temp1 = (byte)InString[11] -48;
  Temp2 = (byte)InString[12] -48;
  Second = Temp1*10 + Temp2;
}


#include <TLC5926.h>
//EEPROM will be used to store list.
#include <EEPROM.h>

#define DIGITS 8

//rotally switch
#define outputA 8 //right
#define outputB 7 //left
#define BTN_enter 9 //push of rotally switch
#define BTN_cancel 10 //tact switch

//light sensor photocell 27-66kohm
int lightSensorPin = A0;
int analogValue = 0;

//variables for rotary switch
byte counter = 0;
int aState = 0;
int aLastState = -1;  

//Pin definition
const int SDI_pin = 5;  
const int CLK_pin = 3;
const int LE_pin  = 4;
const int iOE_pin = 11;  // put this on a PWM pin
const int SDO_pin = 6;
const int CHAIN_CT = DIGITS; // number of digits
const int TEST_NUM = 2;

TLC5926 tlc;


void setup() {

  //light sensor setup
  pinMode(lightSensorPin, INPUT_PULLUP);  // set pull-up on analog pin 0 

  //rotary switch
  pinMode (outputA,INPUT);
  digitalWrite(outputA, HIGH); // turn on internal pullup resistor
  pinMode (outputB,INPUT);
  digitalWrite(outputB, HIGH); // turn on internal pullup resistor
  pinMode (BTN_enter,INPUT);
  digitalWrite(BTN_enter, HIGH); // turn on internal pullup resistor
  pinMode (BTN_cancel,INPUT);
  digitalWrite(BTN_cancel, HIGH); // turn on internal pullup resistor

  // make pin 2 to High-Z
  pinMode (2,INPUT); //high-Z

  // Reads the initial state of the outputA
  aLastState = digitalRead(outputA); 
  
  while (!Serial) {
    delay(1);  // for Leonardo/Micro/Zero
  }

  Serial.begin(9600); //for warnings

  // turn on warnings. probably turn it off when in production
  //tlc.debug(1);
  tlc.attach(CHAIN_CT, SDI_pin, CLK_pin, LE_pin, iOE_pin, SDO_pin);
  tlc.all(LOW);
  tlc.reset();
  tlc.brightness(level);

  // Start the I2C interface
  Wire.begin();
  }

/*
 * Convert character to 16seg pattern
 */

word char2word(char input){
  /*
    Look up table for PSA08-11SRWA (available at Digikey)
    //AMKHUSTGFEDRPCNB  OUT15 - OUT0 
  */
  word segtable[68]= {
    //AMKHUSTGFEDRPCNB
    0b0000000000000000, //' '
    0b0000000000100100, //'!'
    0b0100000000000100, //'"'
    0b0101110111101100, //'#'
    0b1101110011101001, //'$'
    0b1101111001101010, //'%'
    0b1110100111010000, //'&'
    0b0100000000000000, //'''
    0b0000000000010010, //'('
    0b0010001000000000, //')'
    0b0110111000011010, //'*'
    0b0100110000001000, //'+'
    0b0000001000000000, //','
    0b0000100000001000, //'-'
    0b0000010000000000, //'.'
    0b0000001000000010, //'/'
    0b1001001111100111, //'0'
    0b0000000000100110, //'1'
    0b1000100111001101, //'2'
    0b1000000011101101, //'3'
    0b0001100000101100, //'4'
    0b1001100011010001, //'5'
    0b1001100111101001, //'6'
    0b1000000000100101, //'7'
    0b1001100111101101, //'8'
    0b1001100011101101, //'9' 
    0b0100010000000000, //':'
    0b0100001000000000, //';'
    0b0000010000000010, //'<'
    0b0000100011001000, //'='
    0b0010001000000000, //'>"
    0b1000010000001101, //'?"
    0b1101000111001001, //'@"
    0b1001100100101101, //'A'
    0b1100010011101101, //'B'
    0b1001000111000001, //'C'
    0b1100010011100101, //'D'
    0b1001100111000001, //'E'
    0b1001100100000001, //'F'
    0b1001000111101001, //'G'
    0b0001100100101100, //'H'
    0b1100010011000001, //'I'
    0b0000000111100100, //'J'
    0b0001100100010010, //'K'
    0b0001000111000000, //'L'
    0b0011000100100110, //'M'
    0b0011000100110100, //'N'
    0b1001000111100101, //'O'
    0b1001100100001101, //'P'
    0b1001000111110101, //'Q'
    0b1001100100011101, //'R'
    0b1001100011101001, //'S'
    0b1100010000000001, //'T'
    0b0001000111100100, //'U'
    0b0001001100000010, //'V'
    0b0001001100110100, //'W'
    0b0010001000010010, //'X'
    0b0001100011101100, //'Y'
    0b1000001011000011, //'Z'
    0b0100010001000001, //'['
    0b0010000000010000, //'\'
    0b1100010010000000, //']'
    0b0000001000010000, //'^'
    0b0000000011000000, //'_'
    0b0010000000000000, //'`'
  };

  if(input >= ' ' && input <= '`'){
    return(segtable[(int)(input-' ')]);
  }else{
    return(0);
  }   
}

void print16seg(String input){
  int i=0;
  tlc.off();
  for(i=input.length()-1;i>=0;i--){
    tlc.send(char2word(input.charAt(i)));
  }
  tlc.brightness(level);
}

//RTC
int presec = -1;
int first = 1; // first time in loop for initing

bool enter_flag = false;
bool cansel_flag = false;

void loop() {

  if(!enter_flag && !digitalRead(BTN_enter)){
    Serial.print("ENTER");
    enter_flag = true;
    delay(20);
  }else if(enter_flag && digitalRead(BTN_enter)){
    enter_flag = false;
    delay(20);
  }

  if(!cansel_flag && !digitalRead(BTN_cancel)){
    Serial.print("CANSEL");
    cansel_flag = true;
    delay(20);
  }else if(cansel_flag && digitalRead(BTN_cancel)){
    cansel_flag = false;
    delay(20);
  }

  // Setting time through serial data!!!!!
  if (Serial.available()) {
    GetDateStuff(Year, Month, Date, DoW, Hour, Minute, Second);

    rtc.setClockMode(false);  // set to 24h
    rtc.setYear(Year);
    rtc.setMonth(Month);
    rtc.setDate(Date);
    rtc.setDoW(DoW);
    rtc.setHour(Hour);
    rtc.setMinute(Minute);
    rtc.setSecond(Second);

    // Test of alarm functions
    // set A1 to one minute past the time we just set the clock
    // on current day of week.
    rtc.setA1Time(DoW, Hour, Minute+1, Second, 0x0, true, false, false);
    // set A2 to two minutes past, on current day of month.
    rtc.setA2Time(Date, Hour, Minute+2, 0x0, false, false,false);
    // Turn on both alarms, with external interrupt
    rtc.turnOnAlarm(1);
    rtc.turnOnAlarm(2);
    delay(100);
  }

  bool b_12h, b_pm, b_century;
  char* buf = "        ";
  
  //check dial
  aState = digitalRead(outputA); // Reads the "current" state of the outputA
  // If the previous and the current state of the outputA are different, that means a Pulse has occured
  if (aState != aLastState){     
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(outputB) != aState) { 
      counter++;
    } else {
      counter--;
    }
    
    switch (counter % 9){
    case 0:
      //showing time (default)
    break;
    case 1:
      //showing data
      sprintf(buf,"%02d/%02d/%02d",rtc.getYear(),rtc.getMonth(b_century),rtc.getDate());
      print16seg(buf);
    break;
    case 2:
      //laboratoly
      print16seg("LABRATRY");
    break;
    case 3:
      //laboratoly
      print16seg("MEETING ");
    break;
    case 4:
      //laboratoly
      print16seg("ROBOTICS");
    break;
    case 5:
      //laboratoly
      print16seg("GARAGE  ");
    break;
    case 6:
      //laboratoly
      print16seg("QUIETROM");
    break;
    case 7:
      //laboratoly
      print16seg("TRAVEL  ");
    break;
    case 8:
      //laboratoly
      print16seg("LUNCH   ");
    break;
    }

    //Serial.print("Position: ");
    //Serial.println(counter);
    aLastState = aState; // Updates the previous state of the outputA with the current state
  } 

  if (counter % 9 == 0){
      int newsec = rtc.getSecond();
      if(presec != newsec){
        sprintf(buf,"%02d-%02d-%02d",rtc.getHour(b_12h,b_pm),rtc.getMinute(),rtc.getSecond());
        print16seg(buf);
        presec= newsec;
      }
  }

  analogValue = analogRead(lightSensorPin);
  Serial.println(analogValue);
  level = (1.0 - (analogValue-300)/700.0)*24.0;
  tlc.brightness(level);

  first = 0;
}          
