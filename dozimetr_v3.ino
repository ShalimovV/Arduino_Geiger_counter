/*
  LiquidCrystal Library - Hello World

  Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
  library works with all LCD displays that are compatible with the
  Hitachi HD44780 driver. There are many of them out there, and you
  can usually tell them by the 16-pin interface.

  This sketch prints "Hello World!" to the LCD
  and shows the time.

  The circuit:
   LCD RS pin to digital pin 12
   LCD Enable pin to digital pin 11
   LCD D4 pin to digital pin 5
   LCD D5 pin to digital pin 4
   LCD D6 pin to digital pin 3
   LCD D7 pin to digital pin 2
   LCD R/W pin to ground
   LCD VSS pin to ground
   LCD VCC pin to 5V
   10K resistor:
   ends to +5V and ground
   wiper to LCD VO pin (pin 3)

  Library originally added 18 Apr 2008
  by David A. Mellis
  library modified 5 Jul 2009
  by Limor Fried (http://www.ladyada.net)
  example added 9 Jul 2009
  by Tom Igoe
  modified 22 Nov 2010
  by Tom Igoe
  modified 7 Nov 2016
  by Arturo Guadalupi

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/LiquidCrystalHelloWorld

*/

// include the library code:
#include <LiquidCrystal.h>
#include <avr/sleep.h>
#include <avr/wdt.h>


// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to

#define LOG_PERIOD 60000 //время замера в милисекундах 15000-60000.
#define MAX_PERIOD 60000

unsigned long counts;
unsigned long cpm;
unsigned int multiplier;
unsigned long previousMillis;

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 8;

const int buttonPin1 = 6;     // номер входа, подключенный к кнопке
const int buttonPin2 = 7;     // номер входа, подключенный к кнопке
const int lightPin = 9;
const int geigerPin = 10;
const int soundPin = 13;

int button1State = 0;
int button2State = 0;

bool OnFlag = 1;

unsigned char counter;
unsigned int BtnCnt;

int analogInput = 7;
float vout = 0.0;
float vin = 0.0;
float R1 = 96500.0; // resistance of R1 (100K) -see text!
float R2 = 10030.0; // resistance of R2 (10K) - see text!
int value = 0;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void tube_impulse() {
  digitalWrite(soundPin, HIGH);
  delay(5);
  digitalWrite(soundPin, LOW);
  counts = counts + 1;
  
}

ISR (WDT_vect) {
  wdt_disable();
}




void setup() {
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(lightPin, OUTPUT);
  pinMode(geigerPin, OUTPUT);
  pinMode(analogInput, INPUT);
  pinMode(soundPin, OUTPUT);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  analogReference(INTERNAL);

  counter = 200;
  BtnCnt = 0;
  counts = 0;
  cpm = 0;
  multiplier = MAX_PERIOD / LOG_PERIOD;

  digitalWrite(lightPin, HIGH);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Geiger Counter");
  lcd.setCursor(0, 1);
  lcd.print("v1.0 CBM-20");
  delay(2000);

}

void loop() {
  button1State = digitalRead(buttonPin1);
  button2State = digitalRead(buttonPin2);
  unsigned long currentMillis = millis();

  delay(100);

  while (button2State == LOW) {
    BtnCnt++;
    delay(100);
    if (BtnCnt == 30 ) {
      if (OnFlag == 1) {

        OnFlag = 0;
      }
      else {
        OnFlag = 1;
      }
      BtnCnt = 0;
    }
    button2State = digitalRead(buttonPin2);
    lcd.display();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(BtnCnt);
    lcd.setCursor(4, 0);
    if (OnFlag == 1) {
      lcd.print("On");
    }
    else {
      lcd.print("Off");
    }
  }

  BtnCnt = 0;


  if (OnFlag == 1) {

    digitalWrite(geigerPin, HIGH);
    
    lcd.display();
    lcd.clear();
    lcd.setCursor(0, 0);


    if (button1State == LOW) {
      counter = 200;
    }


    if (counter != 0) {
      digitalWrite(lightPin, HIGH);
    }
    else {
      digitalWrite(lightPin, LOW);
    }

    if (counter > 0) {
      counter--;
    }

    if (currentMillis - previousMillis > LOG_PERIOD) {
      previousMillis = currentMillis;
      cpm = counts;
 
      counts = 0;
    }

    lcd.setCursor(10, 0);
    lcd.print("FON");
    lcd.setCursor(13, 0); 
    lcd.print(counts);
    lcd.setCursor(0, 0);
    lcd.print(int((cpm-30)*0.57));
    lcd.print(" uR/h");


    value = analogRead(analogInput);
    vout = (value * 1.1) / 1023.0; // see text
    vin = vout / (R2 / (R1 + R2));
    if (vin < 0.09) {
      vin = 0.0; //statement to quash undesired reading !
    }
    lcd.setCursor(8, 1);
    lcd.print("Bat");
    lcd.print(int((vin - 3.6) * 143));
    lcd.print("%");
    
    attachInterrupt(0, tube_impulse, FALLING);

  }
  else {
      detachInterrupt(0);
      digitalWrite(geigerPin, LOW);
      digitalWrite(lightPin, LOW);
      lcd.noDisplay();
      counter = 0;
      wdt_enable(WDTO_4S); //устанавливаем таймер на 1 секунду
      WDTCSR |= (1 << WDIE); //разрешаем прерывание
      sleep_mode(); //переходим в режим сна, через 4 секунду попадаем в  функцию обработчика прерывания ISR (WDT_vect)
    }

  }
