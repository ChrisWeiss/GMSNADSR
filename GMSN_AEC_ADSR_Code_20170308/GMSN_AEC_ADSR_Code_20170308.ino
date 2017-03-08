/*
 * SA - Share & Adapt.
 * BY - Credit where credit is due.
 * For any purpose including Commercial and Group Buys.
 * No pressure to share design files.
 * R&D funded by donation.
 * https://gmsn.co.uk/products/r-d-funded-by-donations

 * Creative Commons License
 * Licensed under a Creative Commons Attribution 4.0 International License.
 * http://creativecommons.org/licenses/by/4.0/
 * 
 * Source from 
 * https://gmsn.co.uk/products/gmsn-pure-adsr
 * 
 * original design and code from:
 * https://gmsn.co.uk/
 * 
 * This version by A.Cobley
 * andy@r2-dvd.org
 */




#include "SPI.h"

float aPot, aCoeff, enVal = 0, dPot, dCoeff, sPot, sCoeff, sVal, rPot, rCoeff;
boolean gate = 0, rising = 0;
int buttonState, lastButtonState = HIGH, loopStage = 0, x = 0;
long lastDebounceTime = 0, debounceDelay = 500;
int TRIGGER = 2;
int BUTTONTRIGGER = 3;
int BUTTON = 4;
int GATEIN = 5;
int SW1 = 6;
int SW2 = 7;
int DACCS = 10;

void setup() {
  //DAC Comms
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);

  //Pins
  pinMode(DACCS, OUTPUT); //DAC CS
  pinMode(TRIGGER, INPUT); //TRIGGER IN  NOTE, this is inverted
  pinMode(BUTTONTRIGGER, INPUT); //BUTTON TRIGGER
  pinMode(BUTTON, INPUT); //Button
  pinMode(GATEIN, INPUT); //Gate In
  pinMode(SW1, INPUT); //MODE SW1
  pinMode(SW2, INPUT); //MODE SW2
  digitalWrite(DACCS, HIGH);

  //Interupts
  attachInterrupt(digitalPinToInterrupt(TRIGGER), gateOn, FALLING); //Actually on rising, the gate is inverted.
  //  Say we are alive by flashing the LED
  flash (10,500);
}

void loop() {

  //Fast attack label
attack:



  //Check if Gate is On
  if (digitalRead(BUTTON) == LOW || digitalRead(GATEIN) == LOW) {

    //Attack
    if (rising) {
      mcpWrite((int)enVal);
      //delay(5);
      if (enVal == 4095) {
        rising = 0;
      }
    }

    //Not Attack
    if (!rising) {

      //Check if in AR Mode and goto release
      if (digitalRead(SW1) == LOW && digitalRead(SW2) == LOW) {
        goto rlease;
      }

      //else continue with decay to sustain
      dPot = map(analogRead(A2), 0, 1024, 200, 0);
      float dCoeff = dPot / 16384;
      sPot = map(analogRead(A1), 0, 1024, 0, 4096);
      enVal += dCoeff * (sPot - enVal);
      mcpWrite((int)enVal);
    }
  }

  // If no Gate, write release values
  else {

    //Quick release label
rlease:

    rPot = map(analogRead(A0), 0, 1024, 80, 0);
    float rCoeff = rPot / 16384;
    enVal += rCoeff * (-100 - enVal);
    if (enVal < 0) {
      enVal = 0;
    }
    mcpWrite((int)enVal);
  }

  //Poll Trigger Button, debounce, initialise and goto Attack
  int reading = digitalRead(BUTTON);

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        enVal = 0;
        rising = 1;
        rising = 1;
        lastDebounceTime = millis();
        goto attack;
      }
    }
  }
  lastButtonState = reading;

  //Get Attack Values. Moved down here to enable fast attack
  if (rising) {
    aPot = map(analogRead(A3), 0, 1024, 1024, 0);
    if (aPot == 1024) {
      enVal = 4095;
    } else {
      float aCoeff = aPot / 16384;
      enVal += aCoeff * (4311 - enVal);
      if (enVal > 4095) {
        enVal = 4095;
      }
    }
  }
  delayMicroseconds(50);



  //Poll Trigger Button, debounce, initialise Up phase
 reading = digitalRead(BUTTON);

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        enVal = 0;
        x = 0;
        loopStage = 0;
        lastDebounceTime = millis();
      }
    }
  }
  lastButtonState = reading;
  delayMicroseconds(50);
}


//Interrupt routine for rising edge of Gate
void gateOn() {
  enVal = 0;
  rising = 1;
  loopStage = 0;
  x = 0;
}

//Function for writing value to DAC. 0 = Off 4095 = Full on.

void mcpWrite(int value) {

  //CS
  digitalWrite(DACCS, LOW);

  //DAC1 write

  //set top 4 bits of value integer to data variable
  byte data = value >> 8;
  data = data & B00001111;
  data = data | B00110000;
  SPI.transfer(data);

  data = value;
  SPI.transfer(data);

  // Set digital pin DACCS HIGH
  digitalWrite(DACCS, HIGH);
}

//Test function for flashing the led. Value = no of flashes, time = time between flashes in mS
void flash(int value, int time) {
  int x = 0;
  while (x < value) {
    mcpWrite(4000);
    delay(time);
    mcpWrite(0);
    delay(200);
    x++;
  }
}