#define HYDRO_PIN 36  //hydrometer
#define LIGHT_PIN 34 //photoresistor
#define TEMP_PIN 32  //thermoresistor

#include <cstdio>
#include <WiFi.h>
#include <sys/time.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <ctime>

WiFiServer server(80);
WiFiClient client = server.available();

String last;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

int RELAY_PIN = 5; //Pomp relay
int value, hydro, ahydro, bhydro, hydro_com, light, temp;
int test=0;
int i=0;
char timeStringBuff[50];

const char* ssid = ""; //wifi ssid
const char* password = "";  //wifi password
String hostname = "ESP32_Konewka"; //dysplayed name

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  printf("Connecting to WiFi ..\n");
  server.begin();
}

void webpageOutput(){
  client.printf("<!DOCTYPE html>\
  <html>\
  <head>\
  <title>Konewka na ESP32</title>\
  <script>\
  function autoRefresh() {\
  window.location = window.location.href;\
  }\
  setInterval('autoRefresh()', 2000);\
  </script>\
  </head>\
  <body>\
  <h1>Wilgoc: %d %%<br>Swiatlo: %d %%<br>Temp. : %d °C<br><br>Ostatnio Pompa uruchomila sie:<br>%s<br>Wilgoc przed: %d %% , Wilgoc po: %d %%<br><br>Blokada przed ponownym nawodnieniem: %d s</h1>\
  </body>\
  </html>",
  hydro_com, light, temp, last.c_str(), ahydro, bhydro, i);
  delay(1000);
  return;
}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Nie udało się pobrać czasu");
    return;
  }
  char timeStringBuff[50]; // buffer dla wyniku
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  Serial.println(timeStringBuff);
}
    
void setup()
{
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  WiFi.setHostname(hostname.c_str());
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  
  server.begin();
  return;
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    initWiFi();
    Serial.println(WiFi.localIP());
  }
  int analogValue = analogRead(LIGHT_PIN);
  int analogValue2 = analogRead(TEMP_PIN);
  value = analogRead(HYDRO_PIN);
  printLocalTime();
  Serial.print(" Wilgoć : ");
  hydro_com = (abs(value - 4095) * 100) / 2856;
  Serial.print(hydro_com, 0);
  Serial.print("%   ");
  light = (analogValue * 100) / 2000;
  Serial.print("Światło : ");
  Serial.print(light);
  Serial.print("%   ");
  temp = (analogValue2 * 100 / 2946) - 38;
  Serial.print("Temp. : ");
  Serial.print(temp);
  Serial.println("°C");

  if (light < 20 && temp > 5 && hydro_com < 40) {
    if (test == 0) {
      ahydro = hydro_com;
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Pompa ON");
      last = timeStringBuff;
      for (i = 10; i > 0; i--) {
        delay(1000);
        Serial.println(i);
      }
      test = 1;
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Pompa OFF");
      value = analogRead(HYDRO_PIN);
      hydro_com = (abs(value - 4095) * 100) / 2856;
      bhydro = hydro_com;
      i = 14400;
    }
  }
  if (test == 1) {
    if (i > 0) {
      i--;
    } else {
      test = 0;
      i = 0;
    }
  }
  webpageOutput();
}
