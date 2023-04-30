#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


IPAddress server(192,168,2,101);
IPAddress ip(192, 168, 2, 25); 
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
const int sleepSeconds = 3;

boolean active = false; //Declare intermediate variable 
int movement = 0;
const int sensorPin = D2;

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection.......");
    // Attempt to connect, just a name to identify the client
    if (client.connect("Presence1")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
//      client.publish("Papillon/dev02/status","hello world");
      // ... and resubscribe
//      client.subscribe("presence");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // initialize serial for debugging
  Serial.begin(115200);

  pinMode(D0, WAKEUP_PULLUP);
  
  // initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet, dns);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //connect to MQTT server
  client.setServer(server, 1883);
//  client.setCallback(callback);

  // pinMode(buttonPin, WAKEUP_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Configure sensor port
  pinMode(sensorPin, INPUT); // Set Pin7 as input 

  // We go to sleep
  //ESP.deepSleep(0);
} 

void loop() {
  String payload;

  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  } else {
    movement = digitalRead(sensorPin); // Read Sensor
    digitalWrite(LED_BUILTIN, !movement);
    // a new movement was detected

    if(movement == HIGH && active == false) { 
      active = true;
      Serial.println("Motion detected");
      payload = "{\"Action\":";
      payload += 1;
      payload += "}";
      client.publish("PapillonIoT/Presence1/action", (char*) payload.c_str());
    }
    // no motion after motion is detected
    if(movement == LOW && active == true) {
      active = false;
      Serial.println("No movement");
      payload = "{\"Action\":";
      payload += 0;
      payload += "}";
      client.publish("PapillonIoT/Presence1/action", (char*) payload.c_str());
    }

  }
  client.loop();
  delay(100);
}
