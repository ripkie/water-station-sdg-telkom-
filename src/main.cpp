#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoHttpClient.h>

#include "secrets.h"

const char *ssid = WIFI_SSID;
const char *wifiPass = WIFI_PASS;

const char *server = API_SERVER;
const char *resource = API_RESOURCE;
const int port = API_PORT;

const String deviceId = "cf7bcb3d-5456-4b83-a1d5-a6e1f9f81768";

WiFiClientSecure wifiClient;

const int irColdPin = 17; // Air Dingin
const int irHotPin = 18;  // Air Panas

// Tangki 1
const int trigPin1 = 4;
const int echoPin1 = 5;

// parameter galon
const float H = 16.3;
const float BLIND_ZONE = 1.7;
const float AREA = 961.0; // 31 * 31

// GLOBAL IR SENSOR
int counterCold = 0;
int counterHot = 0;
unsigned long lastDetectCold = 0;
unsigned long lastDetectHot = 0;
const unsigned long debounceDelay = 30000;
bool lastStateCold = HIGH;
bool lastStateHot = HIGH;

// GLOBAL ULTRASONIC SENSOR
unsigned long lastUltrasonicRead = 0;
const unsigned long ultrasonicInterval = 2000;

float dist1 = 0.0, vol1 = 0.0;
String status1 = "INIT";

void connectWiFi();
void sendCounterOnly(int counterVal);
float getDistanceSingle(int trig, int echo);
void updateGallonData(int tankNum, int trig, int echo, float &dist, float &vol, String &status);

void setup()
{
  Serial.begin(115200);

  while (!Serial && millis() < 3000)
  {
    delay(10);
  }

  Serial.println("\n--- SYSTEM BOOT ---");

  pinMode(irColdPin, INPUT);
  pinMode(irHotPin, INPUT);

  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  digitalWrite(trigPin1, LOW);

  wifiClient.setInsecure();
  connectWiFi();
}

void loop()
{
  unsigned long currentTime = millis();

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi terputus, reconnect...");
    connectWiFi();
  }

  if (currentTime - lastUltrasonicRead >= ultrasonicInterval)
  {
    lastUltrasonicRead = currentTime;

    updateGallonData(1, trigPin1, echoPin1, dist1, vol1, status1);

    Serial.print("[MONITOR TANGKI 1] Jarak: ");
    Serial.print(dist1);
    Serial.print(" cm | Vol: ");
    Serial.print(vol1);
    Serial.print(" L | Status: ");
    Serial.println(status1);
  }

  // IR AIR DINGIN (PIN 17)
  int stateCold = digitalRead(irColdPin);
  if (stateCold == LOW && lastStateCold == HIGH)
  {
    if (currentTime - lastDetectCold > debounceDelay)
    {
      counterCold++;
      lastDetectCold = currentTime;
      Serial.print(">>> IR AIR DINGIN DETECTED! Counter: ");
      Serial.println(counterCold);
      sendCounterOnly(counterCold);
    }
    else
    {
      Serial.println(">>> IR AIR DINGIN DEBOUNCE WAIT!");
    }
  }
  lastStateCold = stateCold;

  // IR AIR PANAS (PIN 18)
  int stateHot = digitalRead(irHotPin);
  if (stateHot == LOW && lastStateHot == HIGH)
  {
    if (currentTime - lastDetectHot > debounceDelay)
    {
      counterHot++;
      lastDetectHot = currentTime;
      Serial.print(">>> IR AIR PANAS DETECTED! Counter: ");
      Serial.println(counterHot);
      sendCounterOnly(counterHot);
    }
    else
    {
      Serial.println(">>> IR AIR PANAS DEBOUNCE WAIT!");
    }
  }
  lastStateHot = stateHot;

  delay(5);
}

float getDistanceSingle(int trig, int echo)
{
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 10000);
  if (duration == 0 || duration <= 120 || duration >= 30000)
    return 999.0;
  return (duration * 0.0343) / 2.0;
}

void updateGallonData(int tankNum, int trig, int echo, float &dist, float &vol, String &status)
{
  float newDist = getDistanceSingle(trig, echo);

  if (newDist >= 999.0)
  {
    status = "SENSOR_ERROR";
    vol = 0.0;
    dist = 999.0;
    return;
  }

  dist = newDist;

  if (dist <= BLIND_ZONE)
  {
    vol = 16.34;
    status = "FULL";
  }
  else if (dist > BLIND_ZONE && dist <= H)
  {
    float waterHeight = H - dist;
    vol = (waterHeight * AREA) / 1000.0;
    if (vol > 16.34)
      vol = 16.34;
    if (vol < 0.0)
      vol = 0.0;
    status = "OK";
  }
  else
  {
    vol = 0.0;
    status = "EMPTY_OR_REMOVED";
  }
}

void connectWiFi()
{
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wifiPass);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 20000)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("\nWiFi GAGAL connect!");
  }
}

void sendCounterOnly(int counterVal)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
    if (WiFi.status() != WL_CONNECTED)
      return;
  }

  HttpClient http(wifiClient, server, port);

  String payload = "{\"device_id\":\"" + deviceId +
                   "\",\"counter\":" + String(counterVal) +
                   ",\"L\":" + String(vol1, 2) + "}";

  Serial.println("Sending Payload: " + payload);

  http.beginRequest();
  http.post(resource);
  http.sendHeader("Content-Type", "application/json");
  http.sendHeader("Content-Length", payload.length());
  http.beginBody();
  http.print(payload);
  http.endRequest();

  int httpCode = http.responseStatusCode();
  String res = http.responseBody();

  Serial.print("API Code: ");
  Serial.println(httpCode);
  Serial.println("Response: " + res);

  http.stop();
}