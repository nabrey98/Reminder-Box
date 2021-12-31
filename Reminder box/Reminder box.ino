#define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG    //Macros are usually in all capital letters.
#define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
#define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
#define DPRINT(...)     //now defines a blank line
#define DPRINTLN(...)   //now defines a blank line
#endif

#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


//Connection Settings
const char* host = "script.google.com";
const char* googleRedirHost = "script.googleusercontent.com";
const int httpsPort = 443;

unsigned long entryCalender, entryPrintStatus;

//Fetch Google Calendar events
String url = String("/macros/s/") + "AKfycbxX5pi-SkwH-bkruGmLgAzQ8p0WVwYFjcxQR_Str0q3O1eY7hA" + "/exec";

#define UPDATETIME 1000


#ifdef CREDENTIALS
const char* ssid = mySSID;
const char* password = myPASSWORD;
#else
const char* ssid = "padmono"; //replace with you ssid
const char* password = "Reyhan1904"; //replace with your password
#endif


#define NBR_EVENTS 1
String  possibleEvents[NBR_EVENTS] = {"Absen"};
byte  LEDpins[NBR_EVENTS]    = {D4};
byte  switchPins[NBR_EVENTS] = {D5};
byte  alarm[NBR_EVENTS] = {D7};

enum taskStatus {
  none,
  due,
  done
};

taskStatus taskStatus[NBR_EVENTS];

String calendarData = "";

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;
int PB = 14;

// set LCD address, number of columns and rows
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();


  connectToWifi();
  for (int i = 0; i < NBR_EVENTS; i++) {
    Serial.print("nyambung");
    pinMode(LEDpins[i], OUTPUT);
    pinMode(alarm[i], OUTPUT);
    taskStatus[i] = none;  // Reset all LEDs
    pinMode(switchPins[i], INPUT_PULLUP);
  }

  lcd.setCursor(0,0);
  lcd.print("Nabiha Reyhan P");
  lcd.setCursor(3,1);
  lcd.print("Reminder Box");  
  delay(3000);
  lcd.clear();
  
  
}

void loop() {
  
  if (millis() > entryCalender + UPDATETIME) {
    getCalendar();
    entryCalender = millis();
  }
  manageStatus();
  setActivePins();
  if (millis() > entryPrintStatus + 2000) {
    printStatus();
    entryPrintStatus = millis();
  }
  
}

//Turn active pins from array on
void setActivePins() {
  for (int i = 0; i < NBR_EVENTS; i++) {
    if (taskStatus[i] == due) Serial.println("adaevent");
  /*  {
      digitalWrite(LEDpins[i], HIGH);
      digitalWrite(alarm[i], HIGH);
      lcd.setCursor(0,0);
      lcd.print(calendarData);
      lcd.scrollDisplayLeft();

    }*/
    
    else Serial.println("noevent"); 
   /* {
      lcd.clear();
      digitalWrite(LEDpins[i], LOW);
      digitalWrite(alarm[i], LOW);
      lcd.setCursor(3,0);
      lcd.print("No Event");
    }*/
    
  }
}

void printStatus() {
  for (int i = 0; i < NBR_EVENTS; i++) {
    Serial.print("Task ");
    Serial.print(i);
    Serial.print(" Status ");
    Serial.println(taskStatus[i]);
  }
  Serial.println("----------");
}

//Connect to wifi
void connectToWifi() {
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Use HTTPSRedirect class to create TLS connection
  HTTPSRedirect client(httpsPort);

  Serial.print("Connecting to ");
  Serial.println(host);
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client.connect(host, httpsPort);
    if (retval == 1) {
      flag = true;
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    return;
  }
}

//Get calendar entries from google
void getCalendar() {
  Serial.println("Start Request");
  HTTPSRedirect client(httpsPort);
  if (!client.connected()) client.connect(host, httpsPort);
  calendarData = client.getData(url, host, googleRedirHost);
  Serial.print("Calender Data ");
  Serial.println(calendarData);
}

void manageStatus() {
  for (int i = 0; i < NBR_EVENTS; i++) {
    switch (taskStatus[i]) {
      case none:
        if (taskHere(i)) taskStatus[i] = due;
        break;
      case due:
        if (digitalRead(switchPins[i]) == false) taskStatus[i] = done;
        break;
      case done:
        if (taskHere(i) == false) taskStatus[i] = none;
        break;
      default:
        break;
    }
  }
  yield();
}

bool taskHere(int task) {
  if (calendarData.indexOf(possibleEvents[task]) >= 0 ) {
    //    Serial.print("Task found ");
    //    Serial.println(task);
    return true;
  } else {
    //   Serial.print("Task not found ");
    //   Serial.println(task);
    return false;
  }
}
