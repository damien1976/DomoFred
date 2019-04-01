#include <Ethernet.h>
#include <EthernetUdp.h>
#include <ArduinoJson.h>
#include <Eventually.h>

// Create EventManager : 
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

// Fonctionnement UDP : 
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
IPAddress ip(192, 168, 0, 1);
unsigned int localPort = 8888;      // local port to listen on

// Remote IP and port
IPAddress remIP(192, 168, 0, 2);
unsigned int remPort = 8888;      // local port to listen on

#define UDP_TX_PACKET_MAX_SIZE 276 //increase UDP size

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged";        // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;


// Envoi d'un message à l'arduino master : 
bool send_message(EvtListener *listener, EvtContext *ctx) {
  EvtPinListener* pinLst = (EvtPinListener*)listener;
  ExtraData* ex = pinLst->extraData;
  
  Udp.beginPacket(remIP, remPort);
  Udp.print((String)ex->light_pin);
  Udp.endPacket();
  delay(1000);

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

// Allumer Led : 
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

EvtAction doActionsArray[] = 
{
  send_message, // Action 0
  allumerLed_fred, // Action 1
  allumerLed_with_sensor, // Action 2
};

void setup() {
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

  // On attend que l'arduino master finisse son travail : 
  delay(20000);
  // if there's data available, read a packet
  Serial.println("Read packet buffer");
  int packetSize = Udp.parsePacket();
  Serial.println(packetSize);
  if (packetSize) {
    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.println(" *** ");
    Serial.println(packetBuffer);
  }
  delay(200);

  // On lit les données recues : 
  StaticJsonBuffer<4096> jsonBuffer;
  JsonArray& root = jsonBuffer.parseArray(packetBuffer);

  // Walk the JsonArray efficiently
  //for (JsonObject repo : root) { // if C++11 is available
  for (JsonArray::iterator it=root.begin(); it!=root.end(); ++it) { // When C++11 is not available
    JsonObject& repo = *it;
    if (repo["n"].as<int>()==1 && repo["p"].as<int>()>0 && repo["r"].as<int>()==2){ // cas des boutons
      pinMode(atoi(repo["p"].as<int>()), INPUT); 
    }
    if (repo["n"].as<int>() == 2 && repo["p"].as<int>()>0 && repo["r"].as<int>()==2){ // cas des leds
      pinMode(atoi(repo["p"].as<int>()), OUTPUT);   
    }
    if ((int)repo["r"]==2){ // cas des INPUT/OUTPUT sur l'arduino-esclave 
      Serial.println("Pin number : ");
      Serial.print(repo["p"].as<int>());
      Serial.print(" avec ");
      Serial.print(repo["c"][0].as<int>());
      Serial.println(" *** ");
    }
  }

  mgr.resetContext();

  for (JsonArray::iterator it=root.begin(); it!=root.end(); ++it) { // When C++11 is not available
    JsonObject& repo = *it;
    if (repo["n"].as<int>() == 1 && repo["p"].as<int>()>0 && repo["r"].as<int>()==2){ // cas des boutons
      EvtPinListener* evt = new EvtPinListener(repo["p"].as<int>(), 80, doActionsArray[repo["a"].as<int>()]); 
      ExtraData *ex = new ExtraData();
      ex->pinNumber = repo["p"].as<int>();
      ex->pin_state = HIGH;
      ex->light_pin = repo["c"][0].as<int>();
      evt->extraData = (void *)ex;
      mgr.addListener(evt);  
    }
  }
}

void loop() {
  mgr.loopIteration();
  delay(200);

  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    digitalWrite(atoi(packetBuffer), !digitalRead(atoi(packetBuffer)));
  }
  delay(200);
  
}
