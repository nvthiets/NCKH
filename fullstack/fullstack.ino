#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "time.h"
#include <ESP_Google_Sheet_Client.h>

// ====================== WIFI CREDENTIALS ======================
#define WIFI_SSID "dl"
#define WIFI_PASSWORD "1234567890"

// ====================== GOOGLE SHEET CONFIG ======================
#define PROJECT_ID "datalogging-455408"
#define CLIENT_EMAIL "datalogging@datalogging-455408.iam.gserviceaccount.com"
const char PRIVATE_KEY[] PROGMEM = R"KEY(
-----BEGIN PRIVATE KEY-----
MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDSDueY7sa8NN+T
qcfqmDEqfrRJyEASzPLyprMITmFEOVPDlA3k93ZgqUpcWkFfFx/LvwqXe/mvvXcY
AKdaG8kXgwOVJBAyz8mBdA9i7KjC4qA9aqLgDtyyivc8FgYOSrncbWumEFOywgjp
kX3umPX2W1mwo08eAQ2AoJbeLOCIlyY18yKgSs+wD05LzB4uxkays7fdU838bu+L
R1uY2LSav6Y4OcwDi+oFF+nxmFzfMD3RzDVOQLNRCg4iS/elTqgeW0tzLHVgxGM5
MhHdf4aBXSOha3xqKDC33hyt5G3mVEiKxv+//xQbkkeZTzu0Hk6BLrqiOpYEWaRz
hxb4pLwNAgMBAAECggEAAohaeNNm2FhnhQTBhnhnngRhZm2rKW7eKQMjL8ueIZqd
V24pYrmBwbWY0hc70t/ZACBhX3qHrqZBUYFEg0dy0RKWevhbLPmotiOw7HeVAKXc
hREMPgOqhok6ujc7wyAEhf4DE8FTo66c4+VfWClGTJOaYmZTAGDGibv7Ra991OJp
X8AODMaQ1q4aR/84SnuoCfl2thOJin7uHQmL8NzrCK7rpXC7koA+GAX703Crv9NU
V9NBV0TxtRTvqrqR8wQdXMaebOMYUzQWmjJNLpGOyNGEcO5j+amoi0ot/v1Mb7vp
nuzyyhOVj74os9tE11Ip3U2Va0qgwq4yYGmjTTUq8wKBgQD7lNwLjlvHx4/yj2yV
4yvMWax+s/OBiCCKflxNEv3SAwVaTCE+2hRr07KZLZzH3FfrBE5wlULOWKj5DhUZ
9QZJWT+1xI6b/O864xDYuPLo0tpFRl2we5mG8Pw/l+xJtm4BW3qweD5HRbxmklVF
ZylqVJ+bcPm5zZxTFTM8mMbuCwKBgQDVv1oA0v1banzHbXut+TiS+2V+94KoIW4c
Dpgf1MpXaNGts7VM/eWrkoLmk/r7ERMgKEhPwc8WJzPAkvoB8lq27bg4DJ5trVnI
geYHIS2FUKyvqjbPiEwuCE7mKBIjUAXOLQH+fh135mUmn9nujMeU6s62Xt2Yx5LN
vgFwKIeFRwKBgQCtI9p4fYW+rSHtDjXtxcULwx9Kp6/LEpNKnQIK7SoP0ZJqfYBX
0CBDwRLmBpUimwtKq9EL/D1Y432OwdlV3uJvsmN9RgPbaKx/u2uJq2dJQmuDR5UM
81NKoikH3xd393wnpPx/3JEH3f92G/lhZUkEa67YfFzxOYb/StTTPFWT1wKBgQDF
7DseGkEqPg+u5s0oimZ9i7Yu/GeK4fHAny70AqkeiVvnGUnfMhdSaQc3FX3Ir8Xi
+AoFeHQWklRUlIRV+yFO1A7KL70f1+4UQVxL68fn840D2HyKbS4FTgIpEp/A/R9z
C7AWBJeq57dwLYKyZr+EQG0K6AsPpnVkd/WC8rSSjwKBgQDMJYUQxxUdL7zqp0hP
bKebNmz8S4Shhvn/q6fQgpw00Q0dlbztEJP+UJm73uIe6hXCv9jE7D+HvNtLtKpa
MWpTj5APBgnFv+OucILtm2btOoomYsJiVZs2u2hIBqbzPBe4hSPIk2PIVKn4258C
8p1ePyXnDPJZ9mpgJmRJbB/DiA==
-----END PRIVATE KEY-----
)KEY";
const char SPREADSHEET_ID[] = "1WReDMQ9IWguXkmJnUkRTRVcXLq2YHuRchBmbZqrgKFM";
const char SHEET_NAME[] = "Sheet1";

// ====================== HARDWARE CONFIG ======================
LiquidCrystal_I2C lcd(0x27, 20, 4);
Servo ObjServo1, ObjServo2, ObjServo3, ObjServo4, ObjServo5, ObjServo6;
#define RXD2 16
#define TXD2 17
#define SENSOR1 26
#define SENSOR2 27
#define RELAY 14
const int ServoGPIO1 = 32, ServoGPIO2 = 33, ServoGPIO3 = 25, ServoGPIO4 = 19, ServoGPIO5 = 18, ServoGPIO6 = 5;
const int minPulse = 500, maxPulse = 2400;

// ====================== TIMING CONFIG ======================
#define QUICK_STOP_TIME 100    // Dừng nhanh 250ms để quét QR
#define MOVING_SCAN_TIME 400   // Thời gian đọc khi đang di chuyển  
#define MIN_SCAN_INTERVAL 500  // Khoảng cách tối thiểu giữa các lần quét
#define PICKUP_DELAY 800        // Delay trước khi pickup

// ====================== OBJECTS ======================
ESP_Google_Sheet_Client MySheet;
AsyncWebServer server(80);

// ====================== STATE VARIABLES ======================
bool relayState = true;
unsigned long lastScanTime = 0;

// QR Scanning states
enum ScanState {
  IDLE,           // Không quét
  QUICK_STOP,     // Dừng nhanh để bắt đầu quét
  MOVING_SCAN     // Vừa đi vừa đọc
};

ScanState currentScanState = IDLE;
unsigned long scanStateStartTime = 0;
String scannedId = "";
String scannedName = "";
bool hasValidScan = false;
unsigned long pickupReadyTime = 0;

// Servo states
enum ServoState {
  SERVO_IDLE,
  SERVO_PICKUP,
  SERVO_DROP,
  SERVO_DEFAULT
};

ServoState servoState = SERVO_IDLE;
unsigned long servoStartTime = 0;
int servoStep = 0;

// Product tracking
struct Product {
  String id;
  String name;
  int quantity;
  String date;
  String time;
};
std::vector<Product> productList;
std::vector<Product> pendingUploads;

// ====================== UTILS ======================
void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
    }
  }
}

String getFormattedDate() {
  struct tm timeInfo;
  if (!getLocalTime(&timeInfo)) return "00/00/0000";
  char buffer[11];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y", &timeInfo);
  return String(buffer);
}

String getFormattedTime() {
  struct tm timeInfo;
  if (!getLocalTime(&timeInfo)) return "00:00:00";
  char buffer[9];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeInfo);
  return String(buffer);
}

void updateLCD(String message, int row) {
  static String lastMessages[4];
  if (lastMessages[row] != message) {
    lcd.setCursor(0, row);
    lcd.print("                    "); // Xóa dòng
    lcd.setCursor(0, row);
    lcd.print(message);
    lastMessages[row] = message;
  }
}

// ====================== SERVO FUNCTIONS ======================
void DefaultStep() {
  ObjServo5.write(112);
  delay(200);
  ObjServo4.write(180);
  ObjServo3.write(86); 
  ObjServo2.write(115);
  ObjServo1.write(150);
  ObjServo6.write(0);
}

void pickUpStep1() {
  ObjServo1.write(150);
  ObjServo2.write(110);
  ObjServo3.write(86);
  ObjServo4.write(157);
  ObjServo5.write(74);
  ObjServo6.write(9);
}

void pickUpStep2() {
  ObjServo1.write(95);
}

void pickUpStep3() {
  ObjServo5.write(90);
}

void dropRow1Step1() {
  ObjServo6.write(125);
  delay(300);
  ObjServo2.write(150);
  ObjServo3.write(86);
  ObjServo4.write(150);
  ObjServo5.write(44);
  delay(500);
  ObjServo1.write(150);
}

void dropRow2Step1() {
  ObjServo6.write(110);
  delay(300);
  ObjServo2.write(122);
  ObjServo3.write(86);
  ObjServo4.write(150);
  ObjServo5.write(53);
  delay(500);
  ObjServo1.write(150);
}

void dropRow3Step1() {
  ObjServo6.write(93);
  delay(300);
  ObjServo2.write(122);
  ObjServo3.write(86);
  ObjServo4.write(141);
  ObjServo5.write(46);
  delay(500);
  ObjServo1.write(150);
}

void dropRow4Step1() {
  ObjServo6.write(107);
  delay(300);
  ObjServo2.write(123);
  ObjServo3.write(86);
  ObjServo4.write(180);
  ObjServo5.write(80);
  delay(500);
  ObjServo1.write(150);
}

void dropRow5Step1() {
  ObjServo6.write(130);
  delay(300);
  ObjServo2.write(125);
  ObjServo3.write(86);
  ObjServo4.write(180);
  ObjServo5.write(80);
  delay(500);
  ObjServo1.write(150);
}

void startPickup() {
  servoState = SERVO_PICKUP;
  servoStartTime = millis();
  servoStep = 0;
}

void startDrop(String id) {
  servoState = SERVO_DROP;
  servoStartTime = millis();
  servoStep = 0;
  scannedId = id;
}

void startDefault() {
  servoState = SERVO_DEFAULT;
  servoStartTime = millis();
  servoStep = 0;
}

void updateServos() {
  unsigned long currentTime = millis();
  
  switch (servoState) {
    case SERVO_PICKUP:
      if (servoStep == 0 && currentTime - servoStartTime >= 0) {
        pickUpStep1();
        servoStep++;
        servoStartTime = currentTime;
      } else if (servoStep == 1 && currentTime - servoStartTime >= 1000) {
        pickUpStep2();
        servoStep++;
        servoStartTime = currentTime;
      } else if (servoStep == 2 && currentTime - servoStartTime >= 500) {
        pickUpStep3();
        servoStep = 0;
        servoState = SERVO_IDLE;
      }
      break;
      
    case SERVO_DROP:
      if (servoStep == 0 && currentTime - servoStartTime >= 0) {
        if (scannedId == "001") dropRow1Step1();
        else if (scannedId == "002") dropRow2Step1();
        else if (scannedId == "003") dropRow3Step1();
        else if (scannedId == "004") dropRow4Step1();
        else if (scannedId == "005") dropRow5Step1();
        servoStep++;
        servoStartTime = currentTime;
      } else if (servoStep == 1 && currentTime - servoStartTime >= 1000) {
        ObjServo1.write(scannedId == "001" ? 135 : scannedId == "002" ? 131 : 
                        scannedId == "003" ? 115 : 122);
        servoStep = 0;
        servoState = SERVO_IDLE;
      }
      break;
      
    case SERVO_DEFAULT:
      if (servoStep == 0 && currentTime - servoStartTime >= 0) {
        DefaultStep();
        servoStep++;
        servoStartTime = currentTime;
      } else if (servoStep == 1 && currentTime - servoStartTime >= 1000) {
        servoStep = 0;
        servoState = SERVO_IDLE;
      }
      break;
      
    default:
      break;
  }
}

// ====================== PRODUCT VALIDATION ======================
struct ValidProduct {
  String id;
  String name;
};

ValidProduct validProducts[] = {
  {"001", "VEX 123"},
  {"002", "VEX GO"},
  {"003", "VEX IQ"},
  {"004", "VEX EXP"},
  {"005", "VEX V5"}
};

bool isValidProduct(String id, String name) {
  for (auto& vp : validProducts) {
    if (id == vp.id && name == vp.name) return true;
  }
  return false;
}

// ====================== QR PROCESSING ======================
void tryReadQR() {
  if (!Serial2.available()) return;
  
  String id = "";
  String name = "";
  
  unsigned long startRead = millis();
  while (millis() - startRead < 100 && Serial2.available()) {
    String line = Serial2.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      id = line;
      break;
    }
  }
  
  startRead = millis();
  while (millis() - startRead < 100 && Serial2.available()) {
    String line = Serial2.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      name = line;
      break;
    }
  }
  
  while (Serial2.available()) Serial2.read();
  
  if (id.length() > 0) {
    Serial.println("QR Scanned: " + id + " - " + name);
    
    // Chỉ xử lý nếu mã QR hợp lệ
    if (isValidProduct(id, name)) {
        // Chỉ thêm vào pendingUploads khi hợp lệ
        pendingUploads.push_back({id, name, 1, getFormattedDate(), getFormattedTime()});
        
        scannedId = id;
        scannedName = name;
        hasValidScan = true;
        pickupReadyTime = millis() + PICKUP_DELAY;
        
        Serial.println("✅ Valid QR: " + id + " - " + name);
        updateLCD("Scanned: " + name, 0);
        updateLCD("ID: " + id, 1);
        updateLCD("Moving...", 2);
    } else {
        Serial.println("⚠️ Invalid QR, conveyor continues");
        updateLCD("No valid QR", 0);  // Hiển thị LCD invalid
        // Không thêm vào pendingUploads -> không lên Google Sheets và web
    }
}
}

void processPendingUploads() {
  if (pendingUploads.empty() || !MySheet.ready()) return;
  
  Product p = pendingUploads.front();
  pendingUploads.erase(pendingUploads.begin());
  
  if (productList.size() >= 100) productList.erase(productList.begin());
  productList.push_back(p);
  
  FirebaseJson json;
  json.add("range", String(SHEET_NAME) + "!A1");
  json.add("majorDimension", "ROWS");
  FirebaseJsonArray row;
  row.add(p.id).add(p.name).add(p.quantity).add(p.date).add(p.time);
  FirebaseJsonArray rows;
  rows.add(row);
  json.add("values", rows);
  FirebaseJson response;
  
  const int maxRetries = 3;
  for (int i = 0; i < maxRetries; i++) {
    bool success = MySheet.values.append(&response, SPREADSHEET_ID, String(SHEET_NAME) + "!A1", &json, "USER_ENTERED");
    if (success) {
      Serial.println("✅ Uploaded to Sheet");
      return;
    }
    Serial.println("❌ Upload failed: " + MySheet.errorReason());
    delay(1000 * (i + 1));
  }
  Serial.println("❌ Upload failed, retry later");
  pendingUploads.push_back(p);
}

// ====================== SETUP ======================
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  pinMode(SENSOR1, INPUT);
  pinMode(SENSOR2, INPUT);
  pinMode(RELAY, OUTPUT);

  lcd.init();
  lcd.backlight();
  updateLCD("Connecting WiFi...", 0);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  configTime(0, 0, "pool.ntp.org", "time.google.com");
  while (time(nullptr) < 1609459200) delay(500);
  setenv("TZ", "ICT-7", 1);
  tzset();

  updateLCD("WiFi Connected", 0);
  updateLCD(WiFi.localIP().toString(), 1);

  MySheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
  updateLCD(MySheet.ready() ? "GSheet Ready" : "GSheet FAIL", 2);

  if (!LittleFS.begin(true)) {
    Serial.println("❌ Failed to mount LittleFS");
  }

  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "[";
    for (size_t i = 0; i < productList.size(); i++) {
      if (i > 0) json += ",";
      json += "{\"id\":\"" + productList[i].id + "\",";
      json += "\"name\":\"" + productList[i].name + "\",";
      json += "\"quantity\":" + String(productList[i].quantity) + ",";
      json += "\"date\":\"" + productList[i].date + "\",";
      json += "\"time\":\"" + productList[i].time + "\"}";
    }
    json += "]";
    request->send(200, "application/json", json);
  });
  server.begin();

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  ObjServo1.setPeriodHertz(50); ObjServo1.attach(ServoGPIO1, minPulse, maxPulse);
  ObjServo2.setPeriodHertz(50); ObjServo2.attach(ServoGPIO2, minPulse, maxPulse);
  ObjServo3.setPeriodHertz(50); ObjServo3.attach(ServoGPIO3, minPulse, maxPulse);
  ObjServo4.setPeriodHertz(50); ObjServo4.attach(ServoGPIO4, minPulse, maxPulse);
  ObjServo5.setPeriodHertz(50); ObjServo5.attach(ServoGPIO5, minPulse, maxPulse);
  ObjServo6.setPeriodHertz(50); ObjServo6.attach(ServoGPIO6, minPulse, maxPulse);
  delay(500);

  startDefault();
  while (servoState != SERVO_IDLE) updateServos();
  digitalWrite(RELAY, HIGH);
  
  updateLCD("System Ready", 3);
  delay(2000);
  lcd.clear();
}

// ====================== MAIN LOOP ======================
void loop() {
  checkWiFi();
  
  if (!MySheet.ready()) {
    MySheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
    delay(1000);
    return;
  }

  unsigned long currentTime = millis();
  bool sensor1State = digitalRead(SENSOR1) == LOW;
  bool sensor2State = digitalRead(SENSOR2) == LOW;
  
  // ===== QR SCANNING STATE MACHINE =====
  static bool lastSensor1State = false;
  
  switch (currentScanState) {
    case IDLE:
      if (sensor1State && !lastSensor1State && 
          (currentTime - lastScanTime > MIN_SCAN_INTERVAL)) {
        currentScanState = QUICK_STOP;
        scanStateStartTime = currentTime;
        lastScanTime = currentTime;
        
        relayState = false;
        digitalWrite(RELAY, LOW);
        
        while (Serial2.available()) Serial2.read();
        
        updateLCD("Quick Scan...", 0);
        Serial.println("🔍 Start quick scan");
      }
      break;
      
    case QUICK_STOP:
      tryReadQR();
      
      if (currentTime - scanStateStartTime >= QUICK_STOP_TIME) {
        currentScanState = MOVING_SCAN;
        scanStateStartTime = currentTime;
        
        relayState = true;
        digitalWrite(RELAY, HIGH);
        
        updateLCD("Moving & Scanning", 1);
        Serial.println("🚚 Moving scan mode");
      }
      break;
      
    case MOVING_SCAN:
      tryReadQR();
      
      if (hasValidScan || currentTime - scanStateStartTime >= MOVING_SCAN_TIME) {
        currentScanState = IDLE;
        
        if (!hasValidScan) {
          updateLCD("No valid QR", 0);
          Serial.println("❌ Scan timeout or invalid QR");
          // Đảm bảo băng chuyền chạy lại
          relayState = true;
          digitalWrite(RELAY, HIGH);
        }
      }
      break;
  }
  
  lastSensor1State = sensor1State;
  
  // ===== CONVEYOR CONTROL =====
bool shouldStop = (currentScanState == QUICK_STOP) || (sensor2State && hasValidScan);
  
  if (shouldStop && relayState) {
    relayState = false;
    digitalWrite(RELAY, LOW);
  } else if (!shouldStop && !relayState) {
    relayState = true;
    digitalWrite(RELAY, HIGH);
  }
  
  // ===== PICKUP AND DROP =====
  static bool lastSensor2State = false;
  if (sensor2State && !lastSensor2State && hasValidScan && 
      currentTime >= pickupReadyTime && servoState == SERVO_IDLE) {
    
    digitalWrite(RELAY, LOW);
    relayState = false;
    
    Serial.println("🤖 Starting pickup: " + scannedId);
    
    startPickup();
    while (servoState != SERVO_IDLE) updateServos();
    
    delay(500);
    
    startDrop(scannedId);
    while (servoState != SERVO_IDLE) updateServos();
    
    updateLCD("Dropped: " + scannedName, 0);
    updateLCD("ID: " + scannedId, 1);
    updateLCD("Row: " + String(scannedId == "001" ? 1 : scannedId == "002" ? 2 : 
                              scannedId == "003" ? 3 : scannedId == "004" ? 4 : 5), 2);
    
    Serial.println("✅ Dropped at row " + String(scannedId == "001" ? 1 : scannedId == "002" ? 2 : 
                                                scannedId == "003" ? 3 : scannedId == "004" ? 4 : 5));
    
    startDefault();
    while (servoState != SERVO_IDLE) updateServos();
    
    scannedId = "";
    scannedName = "";
    hasValidScan = false;
    
    delay(1000);
    if (digitalRead(SENSOR2) == HIGH) {
      digitalWrite(RELAY, HIGH);
      relayState = true;
    }
  }
  lastSensor2State = sensor2State;
  
  // ===== STATUS DISPLAY =====
  if (currentScanState == IDLE && !hasValidScan) {
    static unsigned long lastDisplayUpdate = 0;
    if (currentTime - lastDisplayUpdate > 3000) {
      updateLCD("System Ready", 0);
      updateLCD("Conveyor: " + String(relayState ? "ON" : "OFF"), 1);
      updateLCD("S1:" + String(sensor1State ? "ON" : "OFF") + " S2:" + String(sensor2State ? "ON" : "OFF"), 2);
      lastDisplayUpdate = currentTime;
    }
  }
  
  updateServos();
  processPendingUploads();
  
  delay(20);
}