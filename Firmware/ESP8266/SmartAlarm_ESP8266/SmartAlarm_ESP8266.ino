/*
  پروژه: دزدگیر هوشمند - بخش ESP8266 (نسخه‌ی نهایی صنعتی)
  ✅ آستانه‌های نسبی LDR بر اساس ldrBase
  ✅ هیسترزیس تنگ‌تر برای هال
  ✅ StaticJsonDocument به جای Dynamic
  ✅ timeout کوتاه برای HTTP
  ✅ اصلاح شرط arming از وبسایت
  ✅ پردازش سریال با اولویت بالاتر
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

WiFiClient wifiClient;

const char* WIFI_SSID = "Wifi ssid";
const char* WIFI_PASS = "12345";
const char* SERVER_HOST = "192.168.xx.xx";
const uint16_t SERVER_PORT = 5000;

////// ipconfig /////python app.py

#define SERIAL_BAUD 9600
#define POLL_INTERVAL 2000
#define SEND_INTERVAL 2000

// ✅ هیسترزیس تنگ‌تر برای هال
#define HALL_DANGER_MIN 450
#define HALL_DANGER_MAX 550
#define HALL_SAFE_MIN 435    // تنگ‌تر شد
#define HALL_SAFE_MAX 565    // تنگ‌تر شد

// ✅ آستانه‌های نسبی LDR (در کد محاسبه می‌شوند)

enum State { DISARMED, ARMED, SUSPICIOUS, TRIGGERED };
State sysState = DISARMED;
bool alarmCooldown = false;
unsigned long cooldownEnd = 0;

struct Sensors {
  int hall = 512, ldr = 500;
  int hallBase = 512, ldrBase = 500;
  bool calibrated = false;
  unsigned long lastUpdate = 0;
} sensors;

bool wifiConnected = false;
unsigned long lastReconnect = 0, lastAlarm = 0, lastSend = 0;
int reconnectCount = 0;
const unsigned long ALARM_COOLDOWN = 5000;
String serialBuf = "";

bool sirenActive = false;
unsigned long sirenLast = 0;
bool sirenState = false;

bool lastHallDanger = false;
bool lastLdrDanger = false;

void setup() {
  Serial.begin(SERIAL_BAUD); Serial.setTimeout(100); delay(500);
  Serial.println("ESP: Starting...");
  connectWiFi();
  if(wifiConnected) { syncStatus(); sendToATmega("CMD:CALIBRATE"); }
  Serial.println("ESP: Ready");
}

void loop() {
  // ✅ پردازش سریال با اولویت بالاتر
  processSerial();
  
  if (alarmCooldown && millis() >= cooldownEnd) {
    alarmCooldown = false;
  }
  
  if(sirenActive) updateSiren();
  
  if(!wifiConnected) { handleReconnect(); return; }
  
  if(millis() - lastSend >= SEND_INTERVAL && sensors.calibrated) { 
    lastSend = millis(); 
    sendPeriodicData(); 
  }
  
  static unsigned long lastPoll = 0;
  if(millis() - lastPoll >= POLL_INTERVAL) { 
    lastPoll = millis(); 
    syncStatus(); 
  }
  
  delay(10);
}

void updateSiren() {
  if(millis() - sirenLast >= 150) { 
    sirenLast = millis(); sirenState = !sirenState; 
    sendToATmega(sirenState ? "CMD:BUZZER_ON" : "CMD:BUZZER_OFF");
  }
}

void connectWiFi() {
  Serial.print("WiFi: Connecting..."); WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();
  while(WiFi.status() != WL_CONNECTED && millis() - start < 15000) { delay(500); Serial.print("."); }
  if(WiFi.status() == WL_CONNECTED) { wifiConnected = true; reconnectCount = 0; Serial.println("\nWiFi: Connected"); } 
  else { wifiConnected = false; lastReconnect = millis(); Serial.println("\nWiFi: Failed"); }
}

void handleReconnect() {
  unsigned long delayMs = min((unsigned long)(2000 * (1 << reconnectCount)), 30000UL);
  if(millis() - lastReconnect < delayMs) return;
  lastReconnect = millis(); reconnectCount++;
  if(reconnectCount > 10) { ESP.restart(); return; }
  Serial.print("WiFi: Reconnecting..."); WiFi.disconnect(true); delay(100); WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();
  while(WiFi.status() != WL_CONNECTED && millis() - start < 10000) delay(500);
  if(WiFi.status() == WL_CONNECTED) { Serial.println("OK"); wifiConnected = true; reconnectCount = 0; syncStatus(); }
}

void processSerial() {
  while(Serial.available() > 0) {
    char c = Serial.read();
    if(c == '\n') { parseMsg(serialBuf); serialBuf = ""; } 
    else if(c != '\r') { serialBuf += c; if(serialBuf.length() > 128) serialBuf = ""; }
  }
}

void parseMsg(String msg) {
  msg.trim(); if(msg.length() == 0) return;
  if(msg.startsWith("DATA|")) parseData(msg.substring(5));
  else if(msg.startsWith("EVENT:")) handleEvent(msg.substring(6));
  else if(msg.startsWith("CALIB:")) handleCalib(msg.substring(6));
  else if(msg == "INIT|ATMEGA_READY") { if(!sensors.calibrated) sendToATmega("CMD:CALIBRATE"); }
}

void parseData(String data) {
  sensors.hall = extractInt(data, "H:"); 
  sensors.ldr = extractInt(data, "L:");
  sensors.lastUpdate = millis();
  if(sensors.calibrated && sysState != DISARMED && !alarmCooldown) checkIntrusion();
}

// ✅ آستانه‌های نسبی LDR
void checkIntrusion() {
  if (sysState == TRIGGERED) return;

  // محاسبه آستانه‌های نسبی LDR (20% کاهش نسبت به پایه)
  int ldrDangerThreshold = sensors.ldrBase * 0.8;  // 20% کمتر از پایه
  int ldrSafeThreshold = sensors.ldrBase * 0.9;    // 10% کمتر از پایه

  bool hallDanger;
  if (lastHallDanger) {
    hallDanger = (sensors.hall >= HALL_SAFE_MIN && sensors.hall <= HALL_SAFE_MAX);
  } else {
    hallDanger = (sensors.hall >= HALL_DANGER_MIN && sensors.hall <= HALL_DANGER_MAX);
  }

  bool ldrDanger;
  if (lastLdrDanger) {
    ldrDanger = (sensors.ldr < ldrSafeThreshold);
  } else {
    ldrDanger = (sensors.ldr < ldrDangerThreshold);
  }

  lastHallDanger = hallDanger;
  lastLdrDanger = ldrDanger;
  
  if (hallDanger && ldrDanger) {
    if (millis() - lastAlarm >= ALARM_COOLDOWN) {
      sysState = TRIGGERED; 
      alarmCooldown = false; 
      sirenActive = true;
      sendToATmega("STATE:TRIGGERED"); 
      sendAlarm("Intrusion", sensors.hall, sensors.ldr, false); 
      lastAlarm = millis();
      
      Serial.print("ESP: 🚨 TRIGGERED | Hall="); Serial.print(sensors.hall);
      Serial.print(" LDR="); Serial.print(sensors.ldr);
      Serial.print(" (Threshold="); Serial.print(ldrDangerThreshold); Serial.println(")");
    }
  } 
  else if (hallDanger || ldrDanger) {
    if (sysState != SUSPICIOUS) {
      sysState = SUSPICIOUS; 
      sirenActive = false; 
      sendToATmega("STATE:SUSPICIOUS");
      sendToATmega("CMD:BUZZER_OFF");
    }
  } 
  else {
    if (sysState != ARMED) {
      sysState = ARMED; 
      sirenActive = false; 
      sendToATmega("STATE:ARMED");
      sendToATmega("CMD:BUZZER_OFF");
    }
  }
}

void handleEvent(String ev) {
  if(ev == "TOUCH_ARM_SHORT") {
    sysState = ARMED; 
    alarmCooldown = true;
    cooldownEnd = millis() + 5000;
    sirenActive = false; 
    
    sendToATmega("STATE:ARMED"); 
    sendToATmega("CMD:RGB:0,255,0,0");
    sendToATmega("CMD:BUZZER_OFF");
    sendArmToServer();
  }
  else if(ev == "TOUCH_ALARM_3S") {
    sysState = TRIGGERED; 
    alarmCooldown = true; cooldownEnd = millis() + 5000;
    sirenActive = true;
    sendToATmega("STATE:TRIGGERED"); 
    sendAlarm("Touch_3s", sensors.hall, sensors.ldr, true);
  }
}

void handleCalib(String val) {
  int comma = val.indexOf(',');
  if(comma > 0) { 
    sensors.hallBase = val.substring(0, comma).toInt(); 
    sensors.ldrBase = val.substring(comma + 1).toInt(); 
    sensors.calibrated = true; 
  }
}

int extractInt(const String& d, const String& key) {
  int s = d.indexOf(key); if(s == -1) return -1;
  s += key.length(); int e = d.indexOf(',', s); if(e == -1) e = d.length();
  return d.substring(s, e).toInt();
}

void sendPeriodicData() {
  if(!wifiConnected) return;
  HTTPClient http; 
  String url = String("http://") + SERVER_HOST + ":" + SERVER_PORT + "/api/alarm";
  http.begin(wifiClient, url); 
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(500);  // ✅ timeout کوتاه
  
  StaticJsonDocument<256> doc;  // ✅ Static به جای Dynamic
  doc["sensor"] = "periodic_update"; doc["hall"] = sensors.hall; doc["ldr"] = sensors.ldr;
  doc["ip"] = WiFi.localIP().toString(); 
  String payload; serializeJson(doc, payload);
  http.POST(payload); http.end();
}

void sendAlarm(const char* src, int h, int l, bool manual) {
  if(!wifiConnected) return;
  HTTPClient http; 
  String url = String("http://") + SERVER_HOST + ":" + SERVER_PORT + "/api/alarm";
  http.begin(wifiClient, url); 
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(500);  // ✅ timeout کوتاه
  
  StaticJsonDocument<384> doc;  // ✅ Static
  doc["sensor"] = src; doc["hall"] = h; doc["ldr"] = l; doc["manual"] = manual; doc["ip"] = WiFi.localIP().toString();
  String payload; serializeJson(doc, payload); 
  int code = http.POST(payload);
  if(code == 200) { lastAlarm = millis(); }
  http.end();
}

void sendArmToServer() {
  if(!wifiConnected) return;
  HTTPClient http; 
  String url = String("http://") + SERVER_HOST + ":" + SERVER_PORT + "/api/arm";
  http.begin(wifiClient, url); 
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(500);  // ✅ timeout کوتاه
  
  StaticJsonDocument<128> doc;  // ✅ Static
  doc["source"] = "touch_sensor"; doc["ip"] = WiFi.localIP().toString();
  String payload; serializeJson(doc, payload);
  int code = http.POST(payload);
  if(code == 200) Serial.println("ESP: ✅ ARM sent to server");
  http.end();
}

// ✅ اصلاح شرط arming از وبسایت
void syncStatus() {
  if(!wifiConnected) return;
  HTTPClient http; 
  String url = String("http://") + SERVER_HOST + ":" + SERVER_PORT + "/api/status";
  http.begin(wifiClient, url); 
  http.setTimeout(500);  // ✅ timeout کوتاه (به جای 2000)
  
  int code = http.GET();
  if(code == 200) {
    String res = http.getString(); 
    StaticJsonDocument<512> doc;  // ✅ Static
    if(!deserializeJson(doc, res)) {
      const char* st = doc["state"] | "UNKNOWN";
      
      if(strcmp(st, "DISARMED") == 0 && sysState != DISARMED && !alarmCooldown) { 
        sysState = DISARMED; alarmCooldown = true; cooldownEnd = millis() + 10000;
        sirenActive = false; 
        sendToATmega("STATE:DISARMED"); sendToATmega("CMD:BUZZER_OFF");
      }
      // ✅ اصلاح شرط: از هر حالتی قابل ARM باشد
      else if(strcmp(st, "ARMED") == 0 && sysState != ARMED) { 
        sysState = ARMED; alarmCooldown = false;
        sirenActive = false; 
        sendToATmega("STATE:ARMED"); sendToATmega("CMD:RGB:0,255,0,0"); sendToATmega("CMD:BUZZER_OFF");
      }
    }
  }
  http.end();
}

void sendToATmega(String cmd) { 
  Serial.println(cmd); 
  Serial.flush(); 
}