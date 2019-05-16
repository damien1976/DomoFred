#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Eventually.h>
#include <ArduinoJson.h>
#include <EEPROM.h>


// A modifier selon le numéro de l'arduino
int this_arduino_ref=1;

// Tableau d'addresse IP ( l'élément n° this_arduino_ref coreespond à l'adresse IP de cet arduino.
IPAddress ip0(192,168,177,1);
IPAddress ip1(192,168,0,1);
IPAddress ip2(192,168,0,2);
IPAddress ip3(192,168,0,3);

IPAddress ipaddress_array[4] = {ip0, ip1, ip2, ip3};


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
unsigned int localPort = 8888;      // local port to listen on

// Remote port (same as localPort) : 
unsigned int remPort = 8888; 

// buffers for receiving and sending data
#define UDP_TX_PACKET_MAX_SIZE 24 //increase UDP size

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // buffer to hold incoming packet,
char ReplyBuffer[] = "";        // a string to send back

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
      ExtraData(): pinNumber(0), pin_state(HIGH), light_pin(0), temp_min(0), temp_max(25), remRef(this_arduino_ref) {
    
      }
      virtual ~ExtraData() {}
      int pinNumber;
      bool pin_state;
      int light_pin;
      float temp_min;
      float temp_max;
      int remRef;
};

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
  
  Udp.beginPacket((IPAddress) ipaddress_array[ex->remRef], remPort); //remIP, remPort);
  Udp.print((String)ex->light_pin);
  Udp.endPacket();
  
  delay(1000);
  return false;
}

EvtAction doActionsArray[] = 
{
  send_message,             // Action 0 : envoyer un message à un autre arduino
  allumerLed_fred,          // Action 1 : Allumer LED lorsque bouton et led sur le même arduino
  allumerLed_with_sensor,   // Action 2 : allumer LED à partir d'un sensor (LED et sensor sur le même arduino)
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
  Ethernet.begin(mac, (IPAddress) ipaddress_array[this_arduino_ref]);
  
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

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // clear EEPROM
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }

  // Variable to store in EEPROM
  String data = "[{\"p\":2,\"r\":1,\"n\":1,\"c\":[5],\"a\":1,\"d\":1},{\"p\":3,\"r\":1,\"n\":1,\"c\":[8],\"a\":0,\"d\":2},{\"p\":5,\"r\":1,\"n\":2,\"c\":[],\"a\":1,\"d\":1},{\"p\":6,\"r\":1,\"n\":2,\"a\":1,\"d\":1},{\"p\":7,\"r\":1,\"n\":2,\"a\":1,\"d\":1},{\"p\":\"A0\",\"r\":1,\"n\":3,\"c\":[7],\"a\":2,\"f\":{\"tl\":15,\"th\":25},\"d\":1},{\"p\":2,\"r\":2,\"n\":1,\"c\":[6],\"a\":0,\"d\":1},{\"p\":8,\"r\":2,\"n\":2,\"a\":1,\"d\":2}]";  
  int eeAddress = 0;   //Location we want the data to be put.

  //One simple call, with the address first and the object second.
  EEPROM.put(eeAddress, data);

  Serial.println("Written data type!");

  String receivedData = "";
  EEPROM.get(eeAddress, receivedData);
  Serial.print("Read Data:");
  Serial.println(receivedData);
  delay(10);

  StaticJsonBuffer<4096> jsonBuffer;
  JsonArray& root = jsonBuffer.parseArray(receivedData);
 
  // Déclaration INPUT - OUTPUT
  // Walk the JsonArray efficiently
  //for (JsonObject repo : root) { // if C++11 is available
  for (JsonArray::iterator it=root.begin(); it!=root.end(); ++it) { // When C++11 is not available
    JsonObject& repo = *it;
    
    if (repo["n"].as<int>()==1 && repo["p"].as<int>()>0 && repo["r"].as<int>()==this_arduino_ref){ // cas des boutons
      pinMode(repo["p"].as<int>(), INPUT_PULLUP); 
      //digitalWrite(repo["p"].as<int>(), LOW);
      Serial.println("BUTTON : ");
      Serial.println(repo["p"].as<int>());
    }
    
    if (repo["n"].as<int>() == 2 && repo["p"].as<int>()>0 && repo["r"].as<int>()==this_arduino_ref){ // cas des leds
      pinMode(repo["p"].as<int>(), OUTPUT);  
      Serial.println("LED : ");
      Serial.println(repo["p"].as<int>());
    }
  }
  
  mgr.resetContext();
  
  for (JsonArray::iterator it=root.begin(); it!=root.end(); ++it) { // When C++11 is not available
    JsonObject& repo = *it;

    if (repo["n"].as<int>()==1 && repo["p"].as<int>()>0 && repo["r"].as<int>()==this_arduino_ref){ // cas des boutons
      EvtPinListener* evt = new EvtPinListener(repo["p"].as<int>(), 80, LOW, doActionsArray[repo["a"].as<int>()]);
      ExtraData *ex = new ExtraData();
      ex->pinNumber = repo["p"].as<int>();
      ex->light_pin = repo["c"][0].as<int>();
      ex->pin_state = digitalRead(ex->light_pin); // LOW;
      ex->remRef = repo["d"].as<int>();
      evt->extraData = (void *)ex;
      mgr.addListener(evt);  
    }
    
    if (repo["n"].as<int>()==3 && repo["r"].as<int>()==this_arduino_ref){ // cas des sensors
      EvtTimeListener* evt = new EvtTimeListener(250, true, allumerLed_with_sensor); // En fait : doActionsArray[repo["a"].as<int>() ou doActionsArray[(int) root_0["action"]);
                                                                                     
      ExtraData *ex = new ExtraData();
      ex->pinNumber = sensorPins[repo["p"].as<int>()];
      ex->light_pin = repo["c"][0].as<int>();
      ex->temp_min = repo["f"]["tl"].as<float>();
      ex->temp_max = repo["f"]["th"].as<float>();
      evt->extraData = (void *)ex;
      mgr.addListener(evt);  
    }
  }
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
