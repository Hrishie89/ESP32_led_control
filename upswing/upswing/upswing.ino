
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#define LED_PIN 4
// Wi-Fi Credentials
const char* ssid = "Io_world";
const char* password = "Iotech@2023";
// MQTT Broker Details
String device_id = "led";
const char* mqtt_server = "91.121.93.94";    //mosquitto local server ip address
const int mqtt_port = 1883;
const char* mqtt_user = "led1";
const char* mqtt_password = "led1";
const char* mqtt_clientId = "Deivce_Device0001";
const char* topic_subscribe = "upswing";            //topic to subscribe

WiFiClient esp_client;
void callback(char* topic, byte* payload, unsigned int length);
PubSubClient mqtt_client(mqtt_server, mqtt_port, callback, esp_client);

// Data Sending Time
unsigned long CurrentMillis, PreviousMillis, DataSendingTime = (unsigned long) 1000 * 10;

// Variable
byte lightStatus;
void mqtt_connect();
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  delay(5000);
  setup_wifi();
  mqtt_connect();
}

void loop() {
  static int st=0;
  st++;
  if( st == 1)
  {
    Serial.println("Light: " + String(lightStatus) + " " + String(lightStatus == 1 ? ": ON" : ": OFF"));
    delay(1000);
  }
    // Devices State Sync Request
    CurrentMillis = millis();
    if (CurrentMillis - PreviousMillis > DataSendingTime) {
      PreviousMillis = CurrentMillis;
    }
  if (!mqtt_client.loop())
    mqtt_connect();
}

void setup_wifi() {                                      //wifi setup
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println("\"" + String(ssid) + "\"");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqtt_connect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    //    if (mqtt_client.connect(mqtt_clientId, mqtt_user, mqtt_password)) {
    if (mqtt_client.connect(mqtt_clientId)) {
      Serial.println("MQTT Client Connected");
      // Subscribe
      mqtt_subscribe(topic_subscribe);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqtt_subscribe(const char * topic) {
  if (mqtt_client.subscribe(topic))
    Serial.println("Subscribe \"" + String(topic) + "\" ok");
  else
    Serial.println("Subscribe \"" + String(topic) + "\" failed");
}

void callback(char* topic, byte* payload, unsigned int length) {         // to receive JSON 
  String command;
  Serial.print("\n\nMessage arrived [");
  Serial.print(topic);
  Serial.println("] ");
  for (int i = 0; i < length; i++)
    command += (char)payload[i];

  if (command.length() > 0)
    Serial.println("Command receive is : " + command);
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, command);
  JsonObject obj = doc.as<JsonObject>();
  String id = obj[String("device_id")];
  String type = obj[String("type")];
  String value = obj[String("value")];
  Serial.println("\n  device_id is : " + id);
  Serial.println("  status is : " + type);
  Serial.println("  value is : " + value);

  if (id == device_id && type == "light") {
    if (value == "1") {
      lightStatus = 1;
      digitalWrite(LED_PIN, HIGH);
      Serial.println("\nLED ON by Application");   
    } else {
      lightStatus = 0;
      digitalWrite(LED_PIN, LOW);
      Serial.println("\nLED OFF by Application");
    }   
  }
}