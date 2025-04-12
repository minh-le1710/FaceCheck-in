#include <ESP8266WiFi.h>
#include "AdafruitIO_WiFi.h"
#include <U8g2lib.h>
#include <time.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

//Adafruit.io
#define IO_USERNAME   "nhatminh1710"
#define IO_KEY        "aio_gKzi73E8RHcNP0sxZHA5ITnsRn62"

//Wifi
#define WIFI_SSID     "vivo S17e"
#define WIFI_PASS     "minh1710"

//U8g2 SSD1306
// reset pin = none, SCL = D1, SDA = D2
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0, U8X8_PIN_NONE, /*SCL=*/ D1, /*SDA=*/ D2
);

// ===== Adafruit IO =====
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
AdafruitIO_Feed *chamcong = io.feed("chamcong123");

//pin
const int greenLedPin = D5;
const int redLedPin   = D6;
const int buzzerPin   = D7;

//state
String lastResultText = "";
unsigned long resultShownAt = 0;

//time and temp
char timeStr[9] = "--:--:--";
char tempStr[6] = "--.-";
int lastFetchMinute = -1;

//get time and weather
void updateTimeAndWeather() {
  // NTP GMT+7
  time_t now = time(nullptr);
  struct tm *ti = localtime(&now);
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", ti);

  // fetch 1m/time
  if (ti->tm_min != lastFetchMinute) {
    lastFetchMinute = ti->tm_min;

    WiFiClient client;
    HTTPClient http;
    String url = String("http://api.openweathermap.org/data/2.5/weather?q=Ho%20Chi%20Minh,VN")
               + "&units=metric&appid=bd9223f61e57f198b2fc5dbd0206a40f";
    if (http.begin(client, url)) {
      if (http.GET() == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        if (!deserializeJson(doc, payload)) {
          float t = doc["main"]["temp"];
          snprintf(tempStr, sizeof(tempStr), "%.1f", t);
        }
      }
      http.end();
    }
  }
}

// oled
void drawDisplay() {
  u8g2.clearBuffer();

  // time and temp
  u8g2.setFont(u8g2_font_6x12_tr);
  const int y_small = 12;  // baseline

  u8g2.drawUTF8(0, y_small, timeStr);

  int tempW = u8g2.getUTF8Width(tempStr);
  int Cw    = u8g2.getUTF8Width("C");
  const int circleR = 1, gap = 1;
  int totalW = tempW + gap + 2*circleR + gap + Cw;
  int x0 = 128 - totalW;
  u8g2.drawUTF8(x0, y_small, tempStr);
  int cx = x0 + tempW + gap + circleR;
  int cy = y_small - 6;
  u8g2.drawCircle(cx, cy, circleR);
  int xC = x0 + tempW + gap + 2*circleR + gap;
  u8g2.drawUTF8(xC, y_small, "C");

  // Result 
  if (lastResultText.length()) {
    u8g2.setFont(u8g2_font_5x8_tr);
    int y_result = y_small + 2 * y_small;  // = 36
    u8g2.drawUTF8(0, y_result, lastResultText.c_str());
  }

  u8g2.sendBuffer();
}

//Callback Adafruit IO 
void handleMessage(AdafruitIO_Data *data) {
  String val = data->value();

  // turn off buzzer
  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, LOW);
  digitalWrite(buzzerPin, HIGH);

  if (val == "1") {
    lastResultText = "Cham cong Thanh cong";
    digitalWrite(greenLedPin, HIGH);  // bật LED xanh
    digitalWrite(buzzerPin, LOW);     // LOW = phát âm thanh
    delay(1000);
    digitalWrite(buzzerPin, HIGH);    // tắt buzzer
    digitalWrite(greenLedPin, LOW);
  }
  else if (val == "0") {
    lastResultText = "Cham cong That Bai";
    digitalWrite(redLedPin, HIGH);    // bật LED đỏ
    digitalWrite(buzzerPin, LOW);
    delay(1000);
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(redLedPin, LOW);
  }
  else {
    lastResultText = "Khong xac dinh";
  }

  resultShownAt = millis();
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, LOW);
  digitalWrite(buzzerPin, HIGH);

  u8g2.begin();
  configTime(7*3600, 0, "pool.ntp.org", "time.nist.gov");

  chamcong->onMessage(handleMessage);
  io.connect();
}

void loop() {
  io.run();

  static unsigned long lastMs = 0;
  if (millis() - lastMs >= 1000) {
    lastMs = millis();
    updateTimeAndWeather();

    // clear text after n second
    if (lastResultText.length() && millis() - resultShownAt >= 2000) {
      lastResultText = "";
    }

    drawDisplay();
  }
}
