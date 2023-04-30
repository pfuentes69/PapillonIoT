#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const int buttonPin = D3;

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

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

// sleep for this many seconds
const int sleepSeconds = 3;

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection.......");
    // Attempt to connect, just a name to identify the client
    if (client.connect("Button1")) {
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

  // Configure button port
  pinMode(buttonPin,  INPUT_PULLUP);
  // pinMode(buttonPin, WAKEUP_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(50);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);
  delay(50);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);
  delay(50);
  digitalWrite(LED_BUILTIN, HIGH);

  // We go to sleep
  //ESP.deepSleep(0);
} 

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  } else {
    int reading = digitalRead(buttonPin);
    if (reading == LOW) {
      Serial.println("PUSH!");

      String payload = "{\"Action\":";
        payload += 1;
        payload += "}";

      client.publish("PapillonIoT/Button1/action", (char*) payload.c_str());

//      client.publish("PapillonIoT/Button1/state", (char *)"{1}");
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
  client.loop();
  delay(100);
}
