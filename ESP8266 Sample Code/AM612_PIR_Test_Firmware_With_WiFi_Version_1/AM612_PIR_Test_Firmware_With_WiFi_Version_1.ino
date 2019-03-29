/*

    Title:        WiFi AM612 PIR Demo
    Date:         27th March 2019
    Version:      1
    Description:  Sample code
    Device:       ESP8266
    By:           David Mottram

*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// defining a static IP address: local & gateway
// default IP in AP mode is 192.168.4.1
IPAddress apIP(192, 168, 1, 42);

// this are the WiFi access point settings
const char *ssid = "ESP8266_PIR";
const char *password = "monkey123";


// Define a web server at port 80 for HTTP
ESP8266WebServer server(80);

// I/O
#define PIR 4
int PIR_Status = LOW;
#define LED 12
int LED_Control = LOW;
const int Speaker =  13;
int LED_Mode = 1;
int Beep_Mode = 1;

// ESP8266 tones library https://github.com/Mottramlabs/ESP8266-Tone-Generator
#include <ESP8266_Tones.h>
ESP8266_Tones Mytone(Speaker);


void handleRoot() {

  // get the received message. https://techtutorialsx.com/2016/10/22/esp8266-webserver-getting-query-parameters/
  String Message_Received = "";

  // make a string of the incoming message
  for (int i = 0; i < server.args(); i++) {
    Message_Received += server.argName(i);
  }

  Serial.print("Received >>"); Serial.print(Message_Received); Serial.println("<<");

  // dynamically generate the mode html text
  char beeperText[80];
  char ledText[80];

  // read the current settings for the devices
  if (Beep_Mode == 1) {
    strcpy(beeperText, "Beeper mode is ON. <a href=\"/?beepoff=0\">Turn it OFF!</a>");
  }
  else {
    strcpy(beeperText, "Beeper mode is OFF. <a href=\"/?beepon=0\">Turn it ON!</a>");
  }

  if (LED_Mode == 1) {
    strcpy(ledText, "LED mode is ON. <a href=\"/?ledoff=0\">Turn it OFF!</a>");
  }
  else {
    strcpy(ledText, "LED mode is OFF. <a href=\"/?ledon=0\">Turn it ON!</a>");
  }


  // test the incoming message for a valid command
  if ((Message_Received.indexOf("beepon") >= 0) ) {
    strcpy(beeperText, "Beeper mode is ON. <a href=\"/?beepoff=0\">Turn it OFF!</a>");
    Beep_Mode = 1;
  } // end test

  if ((Message_Received.indexOf("beepoff") >= 0) ) {
    strcpy(beeperText, "Beeper mode is OFF. <a href=\"/?beepon=0\">Turn it ON!</a>");
    Beep_Mode = 0;
  } // end test


  if ((Message_Received.indexOf("ledon") >= 0) ) {
    strcpy(ledText, "LED mode is ON. <a href=\"/?ledoff=0\">Turn it OFF!</a>");
    LED_Mode = 1;
  } // end test

  if ((Message_Received.indexOf("ledoff") >= 0) ) {
    strcpy(ledText, "LED mode is OFF. <a href=\"/?ledon=0\">Turn it ON!</a>");
    LED_Mode = 0;
  } // end test


  // new uptime value
  char html[1000];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  // read A/D, RAW data input
  int brightness = analogRead(A0);

  // Build an HTML page to display on the web-server root address
  snprintf ( html, 1000,

             "<html>\
  <head>\
    <meta http-equiv='refresh' content='10'/>\
    <title>ESP8266 WiFi Network</title>\
    <style>\
      body { background-color: #bbbbbb; font-family: Arial, Helvetica, Sans-Serif; font-size: 1.7em; Color: #000000; }\
      h1 { Color: #0000AA; }\
    </style>\
  </head>\
  <body>\
    <h1>ESP8266 PIR Demo</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>Brightness A/D Raw Value: %d%</p>\
    <p>%s<p>\
    <p>%s<p>\
    <p>This page refreshes every 10 seconds. Click <a href=\"javascript:window.location.reload();\">here</a> to refresh the page now.</p>\
  </body>\
</html>",

             hr, min % 60, sec % 60,
             brightness,
             beeperText,
             ledText

           );


  server.send ( 200, "text/html", html );

} // end handleRoot





void handleNotFound() {
  // digitalWrite ( LED_BUILTIN, 0 );
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
  // digitalWrite ( LED_BUILTIN, 1 ); //turn the built in LED on pin DO of NodeMCU off
} // end handleNotFound



void setup(void) {

  // I/O setup and initial state
  pinMode(PIR, INPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  // start the serial port
  Serial.begin(115200); Serial.println(""); Serial.println(""); Serial.println("Up and Clackin!");

  // set-up the custom IP address
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   // subnet FF FF FF 00

  // you can remove the password parameter if you want the AP to be open. WiFi.softAP(ssid, password);
  WiFi.softAP(ssid);

  server.on ( "/", handleRoot );
  server.on ( "/ledmode=OFF", handleRoot);
  server.on ( "/ledmode=ON", handleRoot);
  server.on ( "/beepmode=OFF", handleRoot);
  server.on ( "/beepmode=ON", handleRoot);

  server.on ( "/inline", []() {
    server.send ( 200, "text/plain", "this works as well" );
  } );

  server.onNotFound ( handleNotFound );


  // report the AP's IP address
  server.begin();
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Acess Point Started Named: "); Serial.println(ssid);
  Serial.print("HTTP Server Started: http://"); Serial.println(myIP);


  Mytone.Close_Encounters(Speaker);
  digitalWrite(LED, LOW);
  PIR_Status = LOW;


} // End of setup


void loop() {

  server.handleClient();


  // test the PIR input
  
  if (digitalRead(PIR) == HIGH) {

    if (LED_Mode > 0) {
      digitalWrite(LED, HIGH);
    }

    if (PIR_Status == LOW) {

      PIR_Status = HIGH;
      Serial.print("PIR Input = High  ---  A/D Value = "); Serial.println(analogRead(A0));
      if (Beep_Mode > 0) {
        Mytone.Tone_Up(Speaker);
      }

    } // end report

  } // end PIR high


  if (digitalRead(PIR) == LOW) {

    digitalWrite(LED, LOW);
    // wait for LED to go off before reading light level
    delay(100);

    if (PIR_Status == HIGH) {

      PIR_Status = LOW;
      Serial.print("PIR Input = Low   ---  A/D Value = "); Serial.println(analogRead(A0));
      if (Beep_Mode > 0) {
        Mytone.Click(Speaker);
      }

    } // end report

  } // end PIR low


} // End of Loop
