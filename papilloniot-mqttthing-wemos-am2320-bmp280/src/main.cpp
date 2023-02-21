#include <Arduino.h>
#include "BMP280.h"
#include "Wire.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"

#define P0 1013.25

IPAddress broker(192,168,2,101);
IPAddress ip(192, 168, 2, 27); 
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8); 

const char* ssid     = "Papillon";
const char* password = "70445312";

// Initialize the Ethernet client object
WiFiClient WIFIclient;

PubSubClient client(WIFIclient);

// sleep for this many seconds
const int sleepSeconds = 300;

Adafruit_AM2320 am2320 = Adafruit_AM2320();
BMP280 bmp;

void setup() {
  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);

  Serial.begin(115200);
  while (!Serial) {
    delay(10); // hang out until serial port opens
  }

    // initialize WiFi
  WiFi.mode(WIFI_STA);
//  WiFi.config(ip, gateway, subnet, dns);
  bool connectingWIFI = true;
  int tries = 0;

  while (connectingWIFI) {
    WiFi.begin(ssid, password);
    tries = 0;    
    Serial.println("Trying Main WiFi");
    while ((WiFi.status() != WL_CONNECTED) && (tries < 10)) {
      delay(500);
      Serial.print(".");
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

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //connect to MQTT server
  client.setServer(broker, 1883);
  client.connect("DHTSensor3");

  // Config sensors
  am2320.begin();
  bmp.begin();  
  bmp.setOversampling(4);

  // Run duty cycle
  double T,P,H;
  char result = bmp.startMeasurment();
 
  if(result!=0){
    delay(result);
    result = bmp.getTemperatureAndPressure(T,P);
    if(result!=0)
      {
        Serial.print("Pressure = \t");
        Serial.print(P,2);
        Serial.println(" mBar");
        //        Serial.print("A = \t");Serial.print(A,2); Serial.println(" m");
        Serial.print("Temperature = \t");
        T = am2320.readTemperature();
        Serial.print(T);
        Serial.println(" C");
        Serial.print("Humidity = \t");
        H = am2320.readHumidity();
        Serial.print(H);
        Serial.println("%");

        String payload = "{\"Temperature\":";
        payload += T;
        payload += ",\"Humidity\":";
        payload += H;
        payload += ",\"Pressure\":";
        payload += P;
        payload += "}";

        //Serial.println(payload);
        client.publish("PapillonIoT/DHTSensor3/data", (char*) payload.c_str());
        delay(500);
      }
      else {
        Serial.println("Error.");
      }
  }
  else {
    Serial.println("Error.");
  }

  // Let's go to sleep
  ESP.deepSleep(sleepSeconds * 1000000);
}

void loop() {

}
