#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>

#define DHTPIN D4     // what pin we're connected to

//#define DEBUG

DHTesp dht;

IPAddress broker(192,168,2,101);

const char* ssid     = "Papillon";
const char* password = "70445312";

int status = WL_IDLE_STATUS;   // the Wifi radio's status

// sleep for this many seconds
const int sleepSeconds = 600;

// Initialize the Ethernet client object
WiFiClient WIFIclient;

PubSubClient client(WIFIclient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
#ifdef DEBUG
    Serial.print("Attempting MQTT connection...");
#endif
    // Attempt to connect, just a name to identify the client
    if (client.connect("DHT22Sensor1")) {
#ifdef DEBUG
      Serial.println("connected");
#endif
      // Once connected, publish an announcement...
//      client.publish("Papillon/dev02/status","hello world");
      // ... and resubscribe
//      client.subscribe("presence");
    } else {
#ifdef DEBUG
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
#endif
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // SETUP START

  // initialize digital pin LED_BUILTIN as an output.
//  pinMode(LED_BUILTIN, OUTPUT);
//  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)

  // initialize serial for debugging
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println();
  Serial.println("******* SETUP START *******");
#endif

  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);

  // initialize WiFi
  WiFi.mode(WIFI_STA);

  bool connectingWIFI = true;
  int tries = 0;

  while (connectingWIFI) {
    WiFi.begin(ssid, password);
    tries = 0;    
#ifdef DEBUG
    Serial.println("Trying Main WiFi");
#endif
    while ((WiFi.status() != WL_CONNECTED) && (tries < 10)) {
      delay(500);
#ifdef DEBUG
      Serial.print(".");
#endif
      tries++;
    }
#ifdef DEBUG
    Serial.println(); 
#endif
    if (tries == 10) {
#ifdef DEBUG
      Serial.println("Too many trials, we'll sleep and try later");
#endif
      ESP.deepSleep(sleepSeconds * 1000000);
    } else {
      connectingWIFI = false;
    }
  }
#ifdef DEBUG
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
#endif

  //connect to MQTT server
  client.setServer(broker, 1883);

  dht.setup(DHTPIN, DHTesp::DHT22); // data pin 4

  delay(1000); // Stabilize Sensors

  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  }

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature() * 0.95;

  String payload = "{\"Temperature\":";
  payload += temperature;
  payload += ",\"Humidity\":";
  payload += humidity;
  payload += "}";

#ifdef DEBUG
  Serial.println(payload);
#endif
  
  client.publish("PapillonIoT/DHT22Sensor1/data", (char*) payload.c_str());
  delay(1000);  
#ifdef DEBUG
  Serial.println("Deep Sleep");
#endif
  // Let's go to sleep
  ESP.deepSleep(sleepSeconds * 1000000);

}

void loop() {
}

