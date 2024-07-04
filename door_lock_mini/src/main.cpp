#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#define WIFI_SSID "Bundita 2.4G"
#define WIFI_PASS "0816216971"
#define MQTT_SERVER "192.168.1.241"
#define MQTT_USER "mqtt"
#define MQTT_PASSWORD "9696"
#define wifiPin 10
#define lockPin 3

WiFiClient espClient;
PubSubClient client(espClient);

IPAddress local_IP(192, 168, 1, 242);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const char* lockCommandTopic = "home/stairs/doorlock/set";
const char* lockStateTopic = "home/stairs/doorlock";
bool newstart = 0;


// const char* lockAvailabilityTopic = "home/stairs/doorlock/available";
const unsigned long interval = 3UL * 60UL * 60UL * 1000UL;
// const unsigned long interval = 60UL * 1000UL;
unsigned long previousMillis = 0;


void setup_wifi();
void reconnect();
void hard_restart();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  newstart = 1;
  pinMode(wifiPin, OUTPUT);
  pinMode(lockPin, OUTPUT);
  digitalWrite(lockPin, LOW);
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  // Serial.println("Connecting to WiFi");
}

void hard_restart(){
  esp_task_wdt_init(1,true);
  esp_task_wdt_add(NULL);
  while(true);

}

void setup_wifi(){
  delay(10);
  Serial.print("Connecting");
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  
  for (int k = 0; k<25; k++){
    if(WiFi.status()!=WL_CONNECTED){
      int wifidelay = 0;
      wifidelay = wifidelay + 250;
      WiFi.disconnect();
      delay(wifidelay);
      Serial.println("...");
      WiFi.mode(WIFI_STA);
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      delay(wifidelay);
      delay(wifidelay);
    }
  }

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    hard_restart();
    delay(2000);
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(wifiPin, HIGH);
  }

}


void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX); // Generate a random client ID
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      client.subscribe(lockCommandTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}





void callback(char* topic, byte* payload, unsigned int length){
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.print("] ");

  String messageTemp;
  for(unsigned int i = 0; i < length; i++){
    messageTemp += (char)payload[i];
  }
  // Serial.println(messageTemp);

  if(String(topic) == lockCommandTopic){
    if(messageTemp == "ON"){
      digitalWrite(lockPin, HIGH);
      client.publish(lockStateTopic, "ON");
    } else if(messageTemp == "OFF"){
      digitalWrite(lockPin,HIGH);
      client.publish(lockStateTopic,"ON");
      delay(3000);
      digitalWrite(lockPin, LOW);
      client.publish(lockStateTopic, "OFF");
    }
  }
}

void loop() {
  newstart = 0;
  if(!client.connected()){
    reconnect();
  }
  client.loop();
  delay(500);
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    Serial.println("Rebooting...");
    hard_restart();
    
  }
}