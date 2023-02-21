#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"

#define DHTPIN 2     // what pin we're connected to

DHTesp dht;

IPAddress server(192,168,2,101);
IPAddress ip(192, 168, 2, 26); 
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8); 

const char* ssid     = "Papillon";
const char* password = "70445312";

int status = WL_IDLE_STATUS;   // the Wifi radio's status

// Initialize the Ethernet client object
WiFiClient WIFIclient;

PubSubClient client(WIFIclient);

// sleep for this many seconds
const int sleepSeconds = 1800;

void setup() {
  // initialize serial for debugging
  Serial.begin(115200);
  Serial.println();
//  Serial.println("******* SETUP START *******");

  // Connect D0 to RST to wake up
  pinMode(0, WAKEUP_PULLUP);
  // initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet, dns);
  bool connectingWIFI = true;
  int tries = 0;

  while (connectingWIFI) {
    WiFi.begin(ssid, password);
    tries = 0;    
//    Serial.println("Trying Main WiFi");
    while ((WiFi.status() != WL_CONNECTED) && (tries < 10)) {
      delay(500);
//      Serial.print(".");
      tries++;
    }
    Serial.println();
    if (tries == 10) {
      Serial.println("Too many trials, we'll sleep and try later");
      ESP.deepSleep(sleepSeconds * 1000000);
    } else {
      connectingWIFI = false;
    }
  }

//  Serial.println("");
//  Serial.println("WiFi connected");  
//  Serial.println("IP address: ");
//  Serial.println(WiFi.localIP());

  //connect to MQTT server
  client.setServer(server, 1883);

  dht.setup(DHTPIN, DHTesp::DHT11);
  delay(500); // Stabilize Sensors

//  if (!client.connected()) {
//    reconnect();
//  }

  client.connect("DHTSensor2");
//  Serial.println("connected");
  // Once connected, publish an announcement...
//  client.publish("PapillonIoT/DHTSensor2/status","UP");
  float humidity = dht.getHumidity() * 1.3;
  if (humidity > 100) {
    humidity = 100;
  } 
  float temperature = dht.getTemperature() * 0.9;

  String payload = "{\"Temperature\":";
  payload += temperature;
  payload += ",\"Humidity\":";
  payload += humidity;
  payload += "}";

  //Serial.println(payload);
  client.publish("PapillonIoT/DHTSensor2/data", (char*) payload.c_str());
  delay(500);
  // Let's go to sleep
  ESP.deepSleep(sleepSeconds * 1000000);

}

void loop() {
}

