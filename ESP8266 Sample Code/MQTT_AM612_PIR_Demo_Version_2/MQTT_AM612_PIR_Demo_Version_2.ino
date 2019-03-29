/*

    Title:        MQTT AM612 PIR Demo
    Date:         28th March 2019
    Version:      1
    Description:  Sample code
    Device:       ESP8266
    By:           David Mottram

    Notes.
    The LED in the PIR lens flashes at power up until the WiFi connection is made, then turn off.
    By default the PIR will send a report every 10 seconds, below is the format. Also on PIR event and clearing of a PIR event.
    Sent via MQTT on topic AM612_Demo/Status
        "Event, A/D light level, Device MAC address, Wifi AP, Signal level, IP address, event ID"

      Events
        Boot - send once after power up
        Heartbeat - send every 10 seconds, confirming the device is available and reports light level.
        Alert - motion is detected
        Clear - PIR has timed out

    The firmware will accept the following commands via MQTT on topic AM612_Demo/Control
        #REBOOT - Reboots the sensor
        /Chirp - Make a chirp sound
        /Beep1 - Make a tone up
        /Beep2 - Make a tone down
        /LED=On - LED on
        /LED=Off - LED off

    The sensor will accept the command if send with either an "all sensor" or device specific MAC preface.
         ALL/LED=Off - will be accepted by all sensors subscribed to the above topic
        82F3EBB377D6/LED=Off - will only be accepted by the device with that MAC address

    Although any MQTT broker could be used a great solution is to install the mosquitto broker on a raspberry pi,
    the pi must have a fixed IP address to be usable. The same raspberry pi could be used for other functions as
    the MQTT broker feature does not use a lot of system resources.

*/


/* ********************************** Compiler settings, un-comment to use ************************** */
//#define Fixed_IP                      // un-comment to use a fixed IP address to speed up development
#define Print_Report_Level_1          // un-comment for option
#define Print_Report_Level_2          // un-comment for option, report received MQTT message
//#define Enable_Watchdog               // un-comment to enable the watchdog option
#define Send_Percent                  // un-comment to send light value as a percent value 0-100%
/* ************************************************************************************************** */

#include <ESP8266WiFi.h>              // needed for EPS8266
#include <WiFiClient.h>               // WiFi client

// custom settings files
#include "Wifi_Settings.h"            // custom Wifi settings
#include "MQTT_Settings.h"            // MQTT broker and topic
#include "Project_Settings.h"         // board specific details.

// ESP8266 tones library https://github.com/Mottramlabs/ESP8266-Tone-Generator
#include <ESP8266_Tones.h>
ESP8266_Tones Mytone(Speaker);

// incude WiFi and MQTT functions
WiFiClient espClient;                 // for ESP8266 boards
#include <PubSubClient.h>             // http://pubsubclient.knolleary.net/api.html
PubSubClient client(espClient);       // ESP pubsub client
#include "WiFi_Functions.h"           // read wifi data
#include "MQTT_Functions.h"           // MQTT Functions


void setup() {


#ifdef Enable_Watchdog
  // watchdog items, comment out if not used
  secondTick.attach(1, ISRwatchdog);
#endif

  // I/O setup and initial state
  pinMode(PIR, INPUT);
  pinMode(LED, OUTPUT);
  LED_Off;

  Mytone.Close_Encounters(Speaker);

  // connect to WiFi access point
  Get_Wifi();

  // connect to the MQTT broker
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // reset heartbeat timer
  LastMsg = millis();

  Event = "Boot"; Report_Request = 1;

} // end of setup


void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  // PIR test
  if (digitalRead(PIR) == HIGH && PIR_Status == LOW) {

    PIR_Status = HIGH;
    Event = "Alert";
    Report_Request = 1;

  } // end of test

  if (digitalRead(PIR) == LOW && PIR_Status == HIGH) {

    PIR_Status = LOW;
    Event = "Clear";
    Report_Request = 1;

  } // end of test


  if (LED_Control == HIGH) {
    LED_On;
  } // end of test
  else {
    LED_Off;
  }


  // headbeat or report requested
  if (millis() - LastMsg > Heatbeat || Report_Request == 1) {

#ifdef Send_Percent
    // convert to a percent 0-100%
    int zz = map(analogRead(A0), 0, 1024, 0, 100);
    Light = String(zz);

#else
    //read A/D, RAW data input
    Light = String(analogRead(A0));
#endif

    LastMsg = millis();
    Report_Request = 0;

    // update event progress counter
    ++Heart_Value;
    if (Heart_Value > Heartbeat_Range) {
      Heart_Value = 1;
    }

    // heartbeat timed out or report message requested

    // get a Report Make and make as an array
    String Report = Status_Report();
    char Report_array[(Report.length() + 1)];
    Report.toCharArray(Report_array, (Report.length() + 1));

#ifdef Print_Report_Level_2
    // display a report when publishing
    Serial.print("Published To topic: "); Serial.print(InStatus); Serial.print("  -  Report Sent: "); Serial.println(Report_array);
#endif

    // send a status report
    client.publish(InStatus, Report_array);

  } // end of heartbeat timer


} // end of loop
