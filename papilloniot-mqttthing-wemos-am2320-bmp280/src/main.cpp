#include <Arduino.h>
#include "Wire.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"
#include <Adafruit_BMP280.h>

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
const int sleepSeconds = 600;

Adafruit_AM2320 am2320 = Adafruit_AM2320();
Adafruit_BMP280 bmp; // I2C

void setup() {
  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);

  Serial.begin(115200);
  while (!Serial) {
    delay(10); // hang out until serial port opens
  }
  Serial.flush();
  Serial.println();

    // initialize WiFi
  WiFi.mode(WIFI_STA);
//  WiFi.config(ip, gateway, subnet, dns);
  bool connectingWIFI = true;
  int tries = 0;

  while (connectingWIFI) {
    WiFi.begin(ssid, password);
    tries = 0;    
    Serial.println("Trying Main WiFi");
    while ((WiFi.status() != WL_CONNECTED) && (tries < 50)) {
      delay(100);
      Serial.print(".");
      tries++;
    }
    Serial.println();
    if (tries == 50) {
      Serial.println("Too many trials, we'll sleep and try later");
      ESP.deepSleep(sleepSeconds * 1000000);
    } else {
      connectingWIFI = false;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.print("Network: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //connect to MQTT server
  client.setServer(broker, 1883);
  client.connect("DHTSensor3");

  // Config sensors
  if (!am2320.begin()) {
    Serial.println(F("Could not find a valid AM2320 sensor, check wiring or "
                      "try a different address!"));
  }

  if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
//  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
  } else {
    // Default settings from datasheet.
    bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     // Operating Mode. 
                    Adafruit_BMP280::SAMPLING_X2,     // Temp. oversampling 
                    Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling 
                    Adafruit_BMP280::FILTER_X16,      // Filtering. 
                    Adafruit_BMP280::STANDBY_MS_500); // Standby time. 

  }
  // Run duty cycle
  double T,P,H;

  // AM2320
  T = am2320.readTemperature();
  H = am2320.readHumidity();
  Serial.printf("Temperature : %f\n", T);
  Serial.printf("Humidity    : %f\n", H);

  // BMP280
  // must call this to wake sensor up and get new measurement data
  // it blocks until measurement is complete
  if (bmp.takeForcedMeasurement()) {
    // can now print out the new measurements
    P = bmp.readPressure();
    Serial.printf("Pressure   : %f\n", P);
  } else {
    Serial.println("BMP Forced measurement failed!");
  }
  Serial.println();

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

  // Let's go to sleep
  ESP.deepSleep(sleepSeconds * 1000000);
}

void loop() {

}
