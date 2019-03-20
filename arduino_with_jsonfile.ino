#include "Arduino.h"
#include <ArduinoJson.h>
#include <String.h>
#include <SPI.h>
#include <SD.h>
#include <Eventually.h>

// since it is difficult to read A0 from file as an int,
// we use an array of digital pins : 
int sensorPins[9] = {A0, A1, A2, A3, A4, A5, A6, A7, A8};

// create EvtManager
EvtManager mgr;

// Create ExtraData class
class ExtraData {
    public:
      ExtraData(): pinNumber(0), pin_state(HIGH), light_pin(0), temp_min(0), temp_max(25) {
    
      }
      virtual ~ExtraData() {}
      int pinNumber;
      bool pin_state;
      int light_pin;
      float temp_min;
      float temp_max;
};

/*
class ExtraDataSensor {
  public:
    ExtraDataSensor(): pinNumber(0), pin_state(HIGH), light_pin(0), temp_min(0), temp_max(25){
      
    }
    virtual ~ExtraDataSensor() {}
    int pinNumber;
    bool pin_state;
    int light_pin;
    float temp_min;
    float temp_max;  
};
*/
const float baselineTemp = 25.0;
const int sensorPin = A0;

bool allumerLed_fred(EvtListener *listener, EvtContext *ctx) {

  EvtPinListener* pinLst = (EvtPinListener*)listener;
  ExtraData* ex = pinLst->extraData;

  ex->pin_state = !ex->pin_state; // Change state of the led
  digitalWrite(ex->light_pin, ex->pin_state);
  if(ex->pin_state == HIGH) {
    return false;
  } else {
    return true;
  }
}

bool allumerLed_with_sensor(EvtListener *listener, EvtContext *ctx){
  EvtTimeListener* pinLst = (EvtTimeListener*)listener;
  ExtraData *ex = pinLst->extraData;
  int value = analogRead(ex->pinNumber);
  float voltage = (value/1024.0)*5.0;
  float temperature = (voltage -.5)*100;
  if (temperature > ex->temp_min && temperature < ex->temp_max){
    digitalWrite(ex->light_pin, HIGH);
  }
  else{
    digitalWrite(ex->light_pin, LOW);
  }
  return false;

}

EvtAction doActionsArray[] = 
{
  allumerLed_fred, // Action 0
  allumerLed_fred, // Action 1
  allumerLed_with_sensor, // Action 2
};

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  // Return content of the config file :
  String texte = returnContentFile("config.txt"); 
  Serial.println(texte);
  // Parser les données json.
  // Premiere façon : 
  //const size_t capacity = 2*JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(2) + 2*JSON_OBJECT_SIZE(0) + 2*JSON_OBJECT_SIZE(7)+162;
  //DynamicJsonBuffer jsonBuffer(capacity);
  
  // Deuxieme façon : 
  StaticJsonBuffer<4096> jsonBuffer;
  JsonArray& root = jsonBuffer.parseArray(texte);
  //JsonArray& root = jsonBuffer.createArray(); // serializing (inutile dans un premier temps)
  
  // Déclaration INPUT - OUTPUT
  int idx = 0;
  while (idx < root.size()){
    //JsonObject& root_0 = root[idx];
    if (strcmp((char*) root[idx]["pinNumber"], "INPUT")==0 && (int) root[idx]["pinNumber"]>0){ // cas des boutons
      pinMode((int) root[idx]["pinNumber"], INPUT); 
      Serial.print("BUTTON : ");
      Serial.print((int) root[idx]["pinNumber"]);
      Serial.println(" ");
    }
    if (strcmp((char*) root[idx]["naturePin"], "OUTPUT")==0 && (int) root[idx]["pinNumber"]>0){ // cas des leds
      pinMode((int) root[idx]["pinNumber"], OUTPUT); 
      Serial.print("LED ");
      Serial.print((int) root[idx]["pinNumber"]);
      Serial.println(" ");   
    }
    ++idx;
  }

  mgr.resetContext();
  
  idx = 0;
  while (idx < root.size()){
    //JsonObject& root_0 = root[idx];
    if (strcmp((char*) root[idx]["naturePin"], "INPUT")==0 && (int) root[idx]["pinNumber"]>0){ // cas des boutons
      EvtPinListener* evt = new EvtPinListener((int) root[idx]["pinNumber"], 80, doActionsArray[(int) root[idx]["action"]]); //allumerLed_fred); // doActionsArray[(int) root_0["action"]]);
      //evt->extraData = (void *)root_0;
      Serial.println("allumer_fred : ");
      Serial.print((int) root[idx]["pinNumber"]);
      Serial.print(" avec ");
      Serial.print((int) root[idx]["control"][0]);
      Serial.println(" ");
      ExtraData *ex = new ExtraData();
      ex->pinNumber = (int) root[idx]["pinNumber"];
      ex->pin_state = HIGH;
      ex->light_pin = (int) root[idx]["control"][0];
      evt->extraData = (void *)ex;
      mgr.addListener(evt);  
    }
    
    if (strcmp((char*) root[idx]["naturePin"], "sensor")==0){ // cas des sensors
      EvtTimeListener* evt = new EvtTimeListener(250, true, allumerLed_with_sensor); // En fait : doActionsArray[(int) root_0["action"]);
                                                                                     // mais erreur dans le fichier config.
                                                                                     
      Serial.print("allumer_sensor : ");
      Serial.print(sensorPins[(int) root[idx]["pinNumber"]]);
      Serial.print(" avec ");
      Serial.print((int) root[idx]["control"][0]);
      Serial.println(" ");
      ExtraData *ex = new ExtraData();
      ex->pinNumber = sensorPins[(int) root[idx]["pinNumber"]];
      ex->light_pin = (int) root[idx]["control"][0];
      ex->temp_min = (float) root[idx]["feature"]["temperature_low"];
      ex->temp_max = (float) root[idx]["feature"]["temperature_high"];
      evt->extraData = (void *)ex;
      mgr.addListener(evt);  
    }
    ++idx;
  }
  
/*
  while((idx+inputPinStart) != inputPinEnd ){
    EvtPinListener* evt = new EvtPinListener(idx+inputPinStart, 100, allumerLed_fred);
    ExtraData *ex = new ExtraData();
    ex->pinNumber = idx+inputPinStart;
    ex->pin_state = LOW;
    ex->light_pin = idx+outputPinStart;
    evt->extraData = (void *)ex;
    mgr.addListener(evt);
    ++idx;

  }
  */

  
  // essai : commander plusieurs leds à partir d'un seul bouton = fonctionne.
  /*
  if ((idx+inputPinStart) == 2){ // ajout de la diode rouge numéro 8 au bouton 2
    // a modifier : voir pour une liste chaînée.
    EvtPinListener* evt = new EvtPinListener(idx+inputPinStart, 100, allumerLed_fred);
    ExtraData *ex = new ExtraData();
    ex->pinNumber = 2;
    ex->pin_state = LOW;
    ex->light_pin = 7;
    evt->extraData = (void *)ex;
    mgr.addListener(evt);
  }
  */
/*
  // Allumer led with sensor
  EvtTimeListener* evt = new EvtTimeListener(250, true, allumerLed_with_sensor);
  ExtraData *ex = new ExtraData();
  ex->pinNumber = sensorPin;
  ex->light_pin = 7;
  evt->extraData = (void *)ex;
  mgr.addListener(evt);
*/
}

























  
    //Serial.println((char*) root_0["pinNumber"]);
    //Serial.println((char*) root_0["naturePin"]);
    //for (int i =0; i<root_0["control"].size(); i++){
    //  Serial.println((int) root_0["control"][i]);
    //}
    //Serial.println((int)root_0.measureLength());
    //Serial.println((int) root_0["feature"].size());
    //if (root_0["feature"].size() !=0){
    //   Serial.println((char*)root_0["feature"]["temperature_low"]);
    //   Serial.println((char*)root_0["feature"]["temperature_high"]);

  /*
  JsonObject& root_0 = root[0];
  int root_0_pinNumber = root_0["pinNumber"]; // 2
  int root_0_pinNumber_ref = root_0["pinNumber_ref"]; // 1
  const char* root_0_naturePin = root_0["naturePin"]; // "INPUT"
  
  int root_0_control_0 = root_0["control"][0]; // 5
  
  int root_0_action = root_0["action"]; // 1
  const char* root_0_value = root_0["value"]; // "LOW"
  
  JsonObject& root_1 = root[1];
  int root_1_pinNumber = root_1["pinNumber"]; // 3
  int root_1_pinNumber_ref = root_1["pinNumber_ref"]; // 1
  const char* root_1_naturePin = root_1["naturePin"]; // "INPUT"
  
  int root_1_control_0 = root_1["control"][0]; // 6
  
  int root_1_action = root_1["action"]; // 1
  const char* root_1_value = root_1["value"]; // "LOW"

  Serial.println("******************");
  Serial.println(root_1_naturePin);
 */

void loop() {
  mgr.loopIteration();
  delay(150);
  /*
  int idx = 0;
  while (idx < root.size()){
    JsonObject& root_0 = root[idx];
    if (strcmp((char*)root_0["naturePin"], "INPUT")){ // pour chaque bouton, ...
      int state = digitalRead(root_0["pinNumber"]); // on lit son état
      if (digitalRead(root_0["pinNumber"]) == HIGH){ // si son état est HIGH ...
        for (int i =0; i<root_0["control"].size(); i++){ // pour chaque led qu'il contrôle ...
          int state = digitalRead((int) root_0["control"][i]);
          digitalWrite((int) root_0["control"][i], !state);// on inverse son état. 
          }
      }
    }
    idx++;
  }
  */
}

/*
 * GESTION FICHIER 
 */

// Tester l'existence d'un fichier :
bool existFile(char* filename){
  return SD.exists(filename);
}

// create a file : 
// open the file. note that only one file can be open at a time,
// so you have to close this one before opening another.
void createFile(char* filename){
  File myFile = SD.open(filename);
  // if the file opened okay, write to it:
  if (myFile) {
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

// Retourne le contenu d'un fichier dans une variable : 
String returnContentFile(char* filename){
  File myFile = SD.open(filename);
  String content = "";
  while (myFile.available()) {
    content += (char)myFile.read();
  }
  myFile.close();
  return content;
}

// Ajouter du contenu dans un fichier :
bool addContent(char* filename, String content){
  File myFile = SD.open(filename, FILE_WRITE);
  if (myFile){
    myFile.println(content);
  }
  else{
    return false;
  }
  myFile.close();// close the file
  return true;
  
}

// Suppression d'un fichier:
bool removeFile(char* filename){
  File myFile = SD.open(filename);
  if (myFile){
    SD.remove(filename);
    return true;
  }
  else{
    return false;
  }
}

 /*
 * FIN GESTION FICHIER
 */
