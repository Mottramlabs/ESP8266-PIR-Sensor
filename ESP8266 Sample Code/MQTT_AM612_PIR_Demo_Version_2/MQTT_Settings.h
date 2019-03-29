/*
      MQTT Broker and topic
*/

// MQTT Broker, change to the broker used. Below is a mosquitto broker running on a raspberry pi
const char* mqtt_server = "192.168.1.10";

// MQTT Topics
const char* InStatus = "AM612_Demo/Status";
const char* InControl = "AM612_Demo/Control";

// the ESP8266's MAC address is normally used to send a message to a selected device. 
// below is the address used to broadcast to all devices subscribed to the above topic.
String Broadcast_All = "*ALL";
