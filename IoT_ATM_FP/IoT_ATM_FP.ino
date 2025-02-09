#include <WiFi.h>
#include <Keypad.h>
#include <Stepper.h>
#include "src/LiquidCrystal_I2C.h"
#include <Adafruit_Fingerprint.h>
#include <HTTPClient.h>

const char SSID[] = "Redmi Note 12";
const char PASS[] = "Swanan016";

const char HOST[] = "http://atmapp.000webhostapp.com/api.php";
const char REQ_TYPE[] = "application/x-www-form-urlencoded";

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {36, 39, 34, 35};
byte colPins[COLS] = {32, 33, 25, 26};

const int buzzerPin = 19;
const int stepperPin1 = 13;
const int stepperPin2 = 12;
const int stepperPin3 = 14;
const int stepperPin4 = 27;
const int servoPin = 4;

LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
Stepper stepper(2038, stepperPin1, stepperPin3, stepperPin2, stepperPin4);
HTTPClient http;

void setup() {
  Serial.begin(9600);

  finger.begin(57600);
  finger.verifyPassword();

  pinMode(buzzerPin, OUTPUT);
  pinMode(servoPin, OUTPUT);
  stepper.setSpeed(15);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 1);
  lcd.print(F("CONNECTING WIFI.."));
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);

  byte x = 0;
  while (WiFi.status() != WL_CONNECTED) {
    lcd.setCursor(x, 2);
    lcd.print(F("."));
    delay(1000);
    x++;
    if (x == 20) x = 0;
  }

  lcdHome(0);
}

void loop() {
  char key = keypad.getKey();
  if (key == 'D') addFinger();

  int id = matchFinger();
  if (id > 0) withdraw(id);
}

void withdraw(int id) {
  lcd.clear();
  lcd.print((String)"MATCHED ID #" + id);

  lcd.setCursor(0, 1);
  lcd.print(F("ENTER AMOUNT:"));
  lcd.setCursor(0, 3);
  lcd.print(F("[A]: NEXT [C]:CANCEL"));

  int withdrawAmount = inputNumber(2, 5);
  if (withdrawAmount < 100 || withdrawAmount % 100 != 0) {
    clearLine(3);
    lcd.setCursor(0, 3);
    lcd.print(F("INVALID AMOUNT!!"));
    lcdHome(1);
    return;
  }

  clearLine(3);
  lcd.setCursor(0, 3);
  lcd.print(F("CHECKING BALANCE.."));

  int currentBalance = checkBalance(id);
  if (currentBalance == -1) {
    lcd.clear();
    lcd.setCursor(0, 2);
    lcd.print(F("SERVER ERROR!!"));
    lcdHome(1);
    return;
  }
  else if (currentBalance < withdrawAmount) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("BALANCE: "));
    lcd.print(currentBalance);
    lcd.setCursor(0, 2);
    lcd.print(F("INSUFICIENT FUND!"));
    lcdHome(1);
    return;
  }

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("WITHDRAWING.."));
  lcd.setCursor(0, 2);
  lcd.print(F("NOTE COUNT: "));
  servoMove(90, 140);
  for (byte n = 1; n <= withdrawAmount / 100; n++) {
    lcd.setCursor(12, 2);
    lcd.print(n);
    stepper.step(2000);
    delay(1000);
  }
  servoMove(140, 90);

  lcd.setCursor(0, 3);
  lcd.print(F("UPDATING SERVER.."));
  makeTransection(id, withdrawAmount);
  lcdHome(0);
}

// POST request
void makeTransection(int fid, int amount) {
  String body = (String)"action=withdraw&fingerId=" + fid + "&amount=" + amount;
  http.begin(HOST);
  http.addHeader("Content-Type", REQ_TYPE);
  http.POST(body);
  String response = http.getString();
  Serial.println(response);
  http.end();
}

// GET request
int checkBalance(int fid) {
  String link = (String)HOST + "?balance&fingerId=" + fid;
  http.begin(link);
  http.GET();
  String payload = http.getString();
  Serial.println(payload);
  http.end();
  if (payload.indexOf("bal") != -1) {
    payload.remove(0, 12);
    return payload.toInt();
  }
  return -1;
}

void addFinger() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("ENTER FINGER ID:"));
  lcd.setCursor(0, 3);
  lcd.print(F("[A]: NEXT [C]:CANCEL"));
  int id = inputNumber(1, 3);
  if (id > 0) {
    lcd.clear();
    lcd.print((String)"ENROLLING ID #" + id);
    lcd.setCursor(0, 1);
    lcd.print(F("PLS PUT FINGER.."));

    lcd.setCursor(0, 3);
    if (fingerEnroll(id)) {
      lcd.print(F("ADDED."));
      lcdHome(0);
    }
    else {
      lcd.print(F("ERROR!"));
      lcdHome(1);
    }
  }
  else {
    lcd.clear();
    lcd.setCursor(0, 2);
    lcd.print(F("!! INVALID ID !!"));
    lcdHome(1);
  }
}

int inputNumber(byte line, byte limit) {
  String idStr;
  char key;
  byte x = 0;
  while (key != 'A') {
    key = keypad.waitForKey();
    if (key >= '0' && key <= '9') {
      lcd.setCursor(x, line);
      lcd.print(key);
      idStr += key;
      x++;
      delay(500);
      if (x == limit) break;
    }
    if (key == 'C') {
      idStr = "0";
      break;
    }
  }

  return idStr.toInt();
}

void clearLine(byte l) {
  for (byte x = 0; x < 20; x++) {
    lcd.setCursor(x, l);
    lcd.print(F(" "));
  }
}

void lcdHome(bool makeBeep) {
  digitalWrite(buzzerPin, makeBeep);
  delay(1500);
  digitalWrite(buzzerPin, 0);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("-XYZ BANK ATM BOOTH-"));
  lcd.setCursor(0, 2);
  lcd.print(F("WAITING FOR FINGER.."));
}

void servoMove(byte from, byte to) {
  const int pulseWidthMin = 500;
  const int pulseWidthMax = 2500;

  if (from < to) {
    for (int i = from; i < to; i++) {
      int pulseWidth = map(i, 0, 180, pulseWidthMin, pulseWidthMax);
      digitalWrite(servoPin, HIGH);
      delayMicroseconds(pulseWidth);
      digitalWrite(servoPin, LOW);
      delayMicroseconds(20000 - pulseWidth);
    }
  }
  else {
    for (int i = from; i > to; i--) {
      int pulseWidth = map(i, 0, 180, pulseWidthMin, pulseWidthMax);
      digitalWrite(servoPin, HIGH);
      delayMicroseconds(pulseWidth);
      digitalWrite(servoPin, LOW);
      delayMicroseconds(20000 - pulseWidth);
    }
  }
}

bool fingerEnroll(int id) {
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) return 0;

  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) return 0;

  p = finger.createModel();
  if (p != FINGERPRINT_OK) return 0;

  p = finger.storeModel(id);
  if (p != FINGERPRINT_OK) return 0;
  return 1;
}

int matchFinger() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -2;

  return finger.fingerID;
}
