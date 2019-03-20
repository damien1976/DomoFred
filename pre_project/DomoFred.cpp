#include <Arduino.h>

void loop() {

}


/*
 Debounce

 Each time the input pin goes from LOW to HIGH (e.g. because of a push-button
 press), the output pin is toggled from LOW to HIGH or HIGH to LOW.  There's
 a minimum delay between toggles to debounce the circuit (i.e. to ignore
 noise).

 The circuit:
 * LED attached from pin 13 to ground
 * pushbutton attached from pin 2 to +5V
 * 10K resistor attached from pin 2 to ground

 * Note: On most Arduino boards, there is already an LED on the board
 connected to pin 13, so you don't need any extra components for this example.


 created 21 November 2006
 by David A. Mellis
 modified 30 Aug 2011
 by Limor Fried
 modified 28 Dec 2012
 by Mike Walters
 modified 30 Aug 2016
 by Arturo Guadalupi


 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/Debounce
 */

// constants won't change. They're used here to
// set pin numbers:

#include "Event.h"
#include "EventStore.h"
#include "NaturePin.h"

using namespace fred;

const int buttonPin = 42;    // the number of the pushbutton pin
const int ledPin = 13;      // the number of the LED pin

static const int inputPinStart = 31;
static const int inputPinEnd   = 40;
static const int outputPinStart = 41;
static const int outputPinEnd   = 50;
static char inputPin[64]={0}; // sensors
static char outputPin[64]={0}; // actors

static char inputPinReadResult[64]={0}; // sensor read result
static char inputPinLastReadResult[64]={0}; // sensor read result
static unsigned long inputPinLastDebounceTime[64]={0}; // sensor read result
static char inputPinState[64]={0}; // sensor read result


// Variables will change:
int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 25;    // the debounce time; increase if the output flickers
unsigned long multiToggleTime = 1000;
unsigned long multiToggles = 0;

void setup() {
  memset(&inputPin,0,64);
  memset(&outputPin,0,64);

  int idx = 0;
  while ( (idx + inputPinStart) != inputPinEnd) {
    inputPin[idx] = idx + inputPinStart;
    pinMode(inputPin[idx], INPUT);
    ++idx;
  }
  idx = 0;
  while ( (idx + outputPinStart) != outputPinEnd) {
    outputPin[idx] = idx + outputPinStart;
    pinMode(outputPin[idx], OUTPUT);
    ++idx;
  }

  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);


  // set initial LED state
  digitalWrite(ledPin, ledState);
}



static EventStore estore;

void readEvents() {

  int idx = 0;
  while ( (idx + inputPinStart) != inputPinEnd) {

    inputPinReadResult[idx] = digitalRead(inputPin[idx]);

    // If the switch changed, due to noise or pressing:
    if (inputPinReadResult[idx] != inputPinLastReadResult[idx]) {
      // reset the debouncing timer
      inputPinLastDebounceTime[idx] = millis();
    }

    if ((millis() - inputPinLastDebounceTime[idx]) > debounceDelay) {
      // whatever the reading is at, it's been there for longer
      // than the debounce delay, so take it as the actual current state:

      // if the button state has changed:
      if (inputPinReadResult[idx] != inputPinState[idx]) {
       inputPinState[idx] = inputPinReadResult[idx];

        // only toggle the LED if the new button state is HIGH
        if (inputPinState[idx] == HIGH) {
         // something has happened
         //ledState = !ledState;
         Event *e = new Event(ONRELEASE);
         NaturePin *n = new NaturePin();
         n->pinNumber = inputPin[idx];
         n->natureType=PININPUT;

         e->nature = (NatureBase*)n;
         estore.setEvent(e);
        }
      }
    }

    // save the reading.  Next time through the loop,
    // it'll be the lastButtonState:
    inputPinLastReadResult[idx] = inputPinReadResult[idx];

    ++idx;
  }

/*
  // read the state of the switch into a local variable:
  int pinStateResult = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (pinStateResult != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (pinStateResult != buttonState) {
      buttonState = pinStateResult;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
      }
    }
  }

  // set the LED:
  digitalWrite(ledPin, ledState);

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = pinStateResult;

  */
}
