/*
LED Entlightning with Event Manager library "Eventually"
according to the type of push (long or short)
(Need to be performed again)
Source : https://github.com/johnnyb/Eventually
This is my only relevant activity in arduino eclipse for this week.
My other activities : doc on c++ POO ; other Event Manager library experiment (https://github.com/igormiktor/arduino-EventManager)
but works on arduino 1.8.8 but doesn't on eclipse/arduino
call me : damien.etienne@hotmail.fr
*/

#include "Arduino.h"
#include <Eventually.h>

#define LIGHT_PIN 5
#define BUTTON_PIN 2

EvtManager mgr;
bool pin_state = LOW;
bool button_state = LOW;

unsigned long temps = 0;

bool blink() {
	  digitalWrite(LIGHT_PIN-1, HIGH);
	  return true;
}


bool detect_longPush(){
	// on declenche une fonction pour savoir si l'appui est long
	temps = millis();
	// si l'appui est toujours present
	if (digitalRead(BUTTON_PIN) == HIGH){
		mgr.resetContext();
		mgr.addListener(new EvtTimeListener(250, true, (EvtAction)eval_time_push));
		return false;
	}
	else{
		entlightning();
		return false;
	}
}

bool entlightning() {
  mgr.resetContext();
  // Si un appui bouton se produit, on declenche la fonction detect_longPush
  mgr.addListener(new EvtPinListener(BUTTON_PIN, (EvtAction)detect_longPush));
 
  pin_state = !pin_state; // Change state of the led

  digitalWrite(LIGHT_PIN, pin_state);
  if(pin_state == HIGH) {
	  return false;
  } else {
    return true;
  }
}

bool eval_time_push(){
	// si on appuie sur le bouton
	if ((millis()-temps < 1000) && (digitalRead(BUTTON_PIN) == HIGH)){
			 mgr.resetContext();
			 mgr.addListener(new EvtTimeListener(250, true, (EvtAction)eval_time_push));
			 return false;
	}
	else{
		if (millis()-temps > 1000){
			blink();
			return false;
		}
		else{
			entlightning();
			return false;
		}
	}
}

void setup() {
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(LIGHT_PIN-1, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  entlightning();
}

void loop(){
	mgr.loopIteration();
}
