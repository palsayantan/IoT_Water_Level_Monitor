#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>

#define TRIG 5       //trig pin of the HC SR-04 sensor
#define ECHO 4       //echo pin of the HC SR-04 sensor
long duration, cm;   //variables for ultrasonic sensor

#define STATUS_LED 12      //Status LED
#define MOTOR_PIN 2     //connects to the Relay
bool motorState;    //true when motor is ON false when OFF

#define INTERVAL 10000    //sleep time in ms
#define TIMEOUT 90        //AP timeout in sec

String ul = "";
String ll = "";
String mod = "";
String sw = "";
int ut;
int lt;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(STATUS_LED, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);

  digitalWrite(MOTOR_PIN, LOW);
  digitalWrite(STATUS_LED, LOW);

  WiFiManager wifiManager;                                                  //WiFiManager
  //wifiManager.resetSettings();                                            //reset saved settings
  wifiManager.setTimeout(TIMEOUT);                                          //sets timeout until configuration portal gets turned off in seconds
  //wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 1, 1), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));   //set custom ip for portal
  if (!wifiManager.autoConnect("SensorYou Wifi Manager", "EspConfig")) {    //fetches ssid and pass from eeprom and tries to connect
    Serial.println("failed to connect and hit timeout");                    //if it does not connect it starts an access point with the specified name
    delay(1000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
  }
  Serial.println("Connected to Wifi ");                                     //if you get here you have connected to the WiFi
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  digitalWrite(STATUS_LED, HIGH);
}

//  link for APP to hit
//  https://customprojects.000webhostapp.com/user1/index.php?uth=20&lth=80&mod=auto&sw=off

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void distance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  duration = pulseIn(ECHO, HIGH);
  cm = (duration / 2) / 29.1;
  Serial.println(cm);
  delay(100);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting..");
  }
  action();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void action() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url0 = ("http://customprojects.000webhostapp.com/user1/data.php"); //your example link in this format
    String url = (url0 + "?lvl=" + cm + "&ms=" + motorState);
    Serial.println(url);
    http.begin(url);
    int httpCode = http.GET();
    String response = "";
    if (httpCode > 0) {
      // start of fetching get process
      response = http.getString();
      Serial.println(response);
    }
    else {
      Serial.println(httpCode);
      delay(1000);
      action();
    }
    http.end();
    delay(500);
    upperlimit();
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void upperlimit() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url1 = ("http://customprojects.000webhostapp.com/user1/upper_limit"); //your example link in this format
    Serial.println(url1);
    http.begin(url1);
    int httpCode = http.GET();

    if (httpCode > 0) {
      // start of fetching get process
      ul = http.getString();
      ut = ul.toInt();
      Serial.println(ut);
    }
    else {
      Serial.println(httpCode);
      delay(1000);
      upperlimit();
    }
    http.end();
    delay(500);
    lowerlimit();
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void lowerlimit() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url2 = ("http://customprojects.000webhostapp.com/user1/lower_limit"); //your example link in this format
    Serial.println(url2);
    http.begin(url2);
    int httpCode = http.GET();

    if (httpCode > 0) {
      // start of fetching get process
      ll = http.getString();
      lt = ll.toInt();
      Serial.println(lt);
    }
    else {
      Serial.println(httpCode);
      delay(1000);
      lowerlimit();
    }
    http.end();
    delay(500);
    moode();
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void moode() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url3 = ("http://customprojects.000webhostapp.com/user1/mode"); //your example link in this format
    Serial.println(url3);
    http.begin(url3);
    int httpCode = http.GET();

    if (httpCode > 0) {
      // start of fetching get process
      mod = http.getString();
      Serial.println(mod);
    }
    else {
      Serial.println(httpCode);
      delay(1000);
      moode();
    }
    http.end();
    delay(500);
    button();
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void button() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url4 = ("http://customprojects.000webhostapp.com/user1/switch"); //your example link in this format
    Serial.println(url4);
    http.begin(url4);
    int httpCode = http.GET();

    if (httpCode > 0) {
      // start of fetching get process
      sw = http.getString();
      Serial.println(sw);
    }
    else {
      Serial.println(httpCode);
      delay(1000);
      button();
    }
    http.end();
    delay(500);
    process();
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void process() {
  if (cm > lt && mod == "auto") {
    digitalWrite(MOTOR_PIN, HIGH);
    Serial.println("Motor ON1");
    motorState = true;
  }
  if (cm < lt && cm > ut && mod == "auto") {
    digitalWrite(MOTOR_PIN, LOW);
    Serial.println("Motor OFF1");
    motorState = false;
  }
  //if (mod == "manual" && sw == "on") {    //uncomment this line for force manual ON
  if (mod == "manual" && sw == "on" && cm > ut) {
    digitalWrite(MOTOR_PIN, HIGH);
    Serial.println("Motor ON2");
    motorState = true;
  }
  if ((mod == "manual" && sw == "off") || (cm < ut)) {
    digitalWrite(MOTOR_PIN, LOW);
    Serial.println("Motor OFF2");
    motorState = false;
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String urla = ("http://customprojects.000webhostapp.com/user1/data.php"); //your example link in this format
    String urlb = (urla + "?lvl=" + cm + "&ms=" + motorState);
    Serial.println(urlb);
    http.begin(urlb);
    int httpCode = http.GET();
    String response1 = "";
    if (httpCode > 0) {
      // start of fetching get process
      response1 = http.getString();
      Serial.println(response1);
    }
    else {
      Serial.println(httpCode);
      delay(1000);
      Serial.println(urlb);
      http.begin(urlb);
    }
    http.end();
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Serial.println(" ");
  delay(INTERVAL);
}


void loop() {
  distance();
}
