#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "TUT ROG 2";
const char* password = "password";

const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);

#define PUMP_PIN 5
#define FAN_PIN 18
#define LIGHT_PIN 19

String apiKey = "1234";


bool pumpState = false;
bool fanState = false;
bool lightState = false;


int temp = 25;
int moist = 50;
int light = 2000;


void handleCommand(String cmd) {
  Serial.println("Command: " + cmd);

  if (cmd == "pump_on") {
    digitalWrite(PUMP_PIN, HIGH);
    pumpState = true;
  }
  else if (cmd == "pump_off") {
    digitalWrite(PUMP_PIN, LOW);
    pumpState = false;
  }

  else if (cmd == "fan_on") {
    digitalWrite(FAN_PIN, HIGH);
    fanState = true;
  }
  else if (cmd == "fan_off") {
    digitalWrite(FAN_PIN, LOW);
    fanState = false;
  }

  else if (cmd == "light_on") {
    digitalWrite(LIGHT_PIN, HIGH);
    lightState = true;
  }
  else if (cmd == "light_off") {
    digitalWrite(LIGHT_PIN, LOW);
    lightState = false;
  }

  sendStatus(); 
}


void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Received: ");
  Serial.println(message);

  if (message.startsWith(apiKey + ":")) {
    message.replace(apiKey + ":", "");
    handleCommand(message);
  } else {
    Serial.println("Unauthorized");
  }
}


void sendStatus() {
  String payload = "{";
  payload += "\"temp\":" + String(temp) + ",";
  payload += "\"moist\":" + String(moist) + ",";
  payload += "\"light\":" + String(light) + ",";
  payload += "\"pump\":\"" + String(pumpState ? "ON" : "OFF") + "\",";
  payload += "\"fan\":\"" + String(fanState ? "ON" : "OFF") + "\",";
  payload += "\"lightState\":\"" + String(lightState ? "ON" : "OFF") + "\",";
  payload += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  payload += "\"status\":\"ok\"";
  payload += "}";

  client.publish("kevin123/home/status", payload.c_str());

  Serial.println("Sent: " + payload);
}


void reconnect() {
  if (client.connected()) return;

  Serial.println("Connecting to MQTT...");

  String clientId = "ESP32-" + String(random(1000, 9999));

  if (client.connect(clientId.c_str())) {
    Serial.println(" MQTT Connected");
    client.subscribe("kevin123/home/commands");

    sendStatus(); 
  } else {
    Serial.print(" Failed rc=");
    Serial.println(client.state());
  }

  delay(3000);
}

void setup() {
  Serial.begin(115200);

  pinMode(PUMP_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);

  Serial.println("Connecting WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("...");
  }

  Serial.println(" WiFi Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  delay(2000);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  client.setKeepAlive(60);
  client.setSocketTimeout(10);
}


unsigned long lastSend = 0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  if (millis() - lastSend > 5000) {
    lastSend = millis();

    // simulate changing sensor values
    temp = random(20, 35);
    moist = random(40, 70);
    light = random(1500, 3000);

    sendStatus();
  }
}