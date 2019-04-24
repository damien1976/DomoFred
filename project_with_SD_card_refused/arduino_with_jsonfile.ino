#include <Ethernet.h>
#include <EthernetUdp.h>
#include "Arduino.h"
#include <ArduinoJson.h>
#include <String.h>
#include <SPI.h>
#include <SD.h>
#include <Eventually.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 2);
unsigned int localPort = 8888;      // local port to listen on

// Remote address and port : 
IPAddress remIP(192, 168, 0, 1); 
unsigned int remPort = 8888; 

// buffers for receiving and sending data
// #include <EthernetUdp.h>   // A tester **************
#define UDP_TX_PACKET_MAX_SIZE 512 //increase UDP size

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged";        // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

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

// Envoi d'un message à un arduino distant : 
bool send_message(EvtListener *listener, EvtContext *ctx) {
  EvtPinListener* pinLst = (EvtPinListener*)listener;
  ExtraData* ex = pinLst->extraData;
  
  Udp.beginPacket(remIP, remPort);
  //Udp.print("Appui :");
  //Udp.print((String)ex->pinNumber);
  //Udp.print("Allumer :");
  Udp.print((String)ex->light_pin);
  Udp.endPacket();
  
  delay(1000);
  return false;
}

EvtAction doActionsArray[] = 
{
  send_message, // Action 0
  allumerLed_fred, // Action 1
  allumerLed_with_sensor, // Action 2
};

void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // start the Ethernet
  Ethernet.begin(mac, ip);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start UDP
  Udp.begin(localPort);
  
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
  String texte = returnContentFile("CONFIG.txt"); //"config1.txt"); 
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
  /*
  int idx = 0;
  while (idx < root.size()){
    //JsonObject& root_0 = root[idx];
    if (strcmp((char*) root[idx]["naturePin"], "INPUT")==0 && (int) root[idx]["pinNumber"]>0 && (int)root[idx]["pinNumber_ref"]==1){ // cas des boutons
      pinMode((int) root[idx]["pinNumber"], INPUT); 
      Serial.println((int) root[idx]["pinNumber"]);
    }
    if (strcmp((char*) root[idx]["naturePin"], "OUTPUT")==0 && (int) root[idx]["pinNumber"]>0 && (int)root[idx]["pinNumber_ref"]==1){ // cas des leds
      pinMode((int) root[idx]["pinNumber"], OUTPUT);   
      Serial.println((int) root[idx]["pinNumber"]);
    }
    ++idx;
  }
  */
  int idx = 0;
  while (idx < root.size()){
    //JsonObject& root_0 = root[idx];
    if ((int) root[idx]["n"]==1 && (int) root[idx]["p"]>0 && (int)root[idx]["r"]==1){ // cas des boutons
      pinMode((int) root[idx]["p"], INPUT); 
      Serial.println((int) root[idx]["p"]);
    }
    if ((int) root[idx]["n"]==2 && (int) root[idx]["p"]>0 && (int)root[idx]["r"]==1){ // cas des leds
      pinMode((int) root[idx]["p"], OUTPUT);   
      Serial.println((int) root[idx]["p"]);
    }
    ++idx;
  }
  // Envoi pour la configuration de l'autre arduino-esclave : 
  // Length (with one extra character for the null terminator)
  int str_len = texte.length() + 1; 
  Serial.println("Size : ");
  Serial.println(str_len);

  // Prepare the character array (the buffer) 
  char char_array[276];
  
  // Copy it over 
  texte.toCharArray(char_array, 276);
  Serial.println("Char _ array : ");
  Serial.println(char_array);
  Udp.beginPacket(remIP, remPort);
  Udp.print(char_array);
  Udp.endPacket();
  
  mgr.resetContext();
  
  idx = 0;
  while (idx < root.size()){
    //JsonObject& root_0 = root[idx];
    if ((int) root[idx]["n"]==1 && (int) root[idx]["p"]>0 && (int)root[idx]["r"]==1){ // cas des boutons
      EvtPinListener* evt = new EvtPinListener((int) root[idx]["p"], 80, doActionsArray[(int) root[idx]["a"]]); //allumerLed_fred); // doActionsArray[(int) root_0["action"]]);
      //evt->extraData = (void *)root_0;
      Serial.println("allumer_fred : ");
      Serial.print((int) root[idx]["p"]);
      Serial.print(" avec ");
      Serial.print((int) root[idx]["c"][0]);
      Serial.println(" ");
      ExtraData *ex = new ExtraData();
      ex->pinNumber = (int) root[idx]["p"];
      ex->pin_state = HIGH;
      ex->light_pin = (int) root[idx]["c"][0];
      evt->extraData = (void *)ex;
      mgr.addListener(evt);  
    }
    
    if ((int)root[idx]["n"]==3 && (int)root[idx]["r"]==1){ // cas des sensors
      EvtTimeListener* evt = new EvtTimeListener(250, true, allumerLed_with_sensor); // En fait : doActionsArray[(int) root_0["action"]);
                                                                                     // mais erreur dans le fichier config.
                                                                                     
      Serial.print("allumer_sensor : ");
      Serial.print(sensorPins[(int) root[idx]["p"]]);
      Serial.print(" avec ");
      Serial.print((int) root[idx]["c"][0]);
      Serial.println(" ");
      ExtraData *ex = new ExtraData();
      ex->pinNumber = sensorPins[(int) root[idx]["p"]];
      ex->light_pin = (int) root[idx]["c"][0];
      ex->temp_min = (float) root[idx]["f"]["tl"];
      ex->temp_max = (float) root[idx]["f"]["th"];
      evt->extraData = (void *)ex;
      mgr.addListener(evt);  
    }
    ++idx;
  }

  // Essai : à supprimer par la suite
  //pinMode(8, OUTPUT);
}

void loop() {
  mgr.loopIteration();
  delay(150);

  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    digitalWrite(atoi(packetBuffer), !digitalRead(atoi(packetBuffer)));
  }
  delay(200);
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
    Serial.println("error opening file");
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
