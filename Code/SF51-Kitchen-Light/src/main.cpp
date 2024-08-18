#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

#include "settings.h" //GPIO defines, NodeMCU good-pin table
#include "secret.h"   //Wifi and mqtt server info

const char *ssid = ssidWifi;       // defined on secret.h
const char *password = passWifi;   // defined on secret.h
const char *mqtt_server = mqttURL; // defined on secret.h
const char *deviceName = mqttClient;

WiFiClient espClient;
PubSubClient client(espClient);

boolean ButtonState = false;

StaticJsonDocument<100> doc;
StaticJsonDocument<300> updater;
int device;
int valuejson;

boolean TouchEnable = true;
int lastCount = 0;
int count = 0;

int blinker = 5;
boolean touchState = false;

unsigned long WifiDelayMillis = 0;
const long WifiDelayInterval = 5000; // interval to check wifi and mqtt
unsigned long SwitchDelayMillis = 0;

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname(deviceName); // DHCP Hostname (useful for finding device for static lease)
  if (!WiFi.config(local_IP, gateway, subnet))
  {
    Serial.println("STA Failed to configure");
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(".");
     
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Test if parsing succeeds.
  DeserializationError error = deserializeJson(doc, payload);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  // Print the values to data types
  device = doc["device"].as<unsigned int>();
  valuejson = doc["value"].as<unsigned int>();
  Serial.print("device: ");
  Serial.print(device);
  Serial.print(" valuejson: ");
  Serial.println(valuejson);

  switch (device)
  {

  case 1:
    if (valuejson == 1)
    {
      digitalWrite(KitchenLightRelay, HIGH);
      client.publish("Kitchen-Light", "ON");

    }
    else if (valuejson == 0)
    {
      digitalWrite(KitchenLightRelay, LOW);
      client.publish("Kitchen-Light", "OFF");
    }
    break;

  case 2:
    if (valuejson == 1)
    {
      TouchEnable = true;
      client.publish("Kitchen-Touch", "ON");
    }
    else if (valuejson == 0)
    {    
      TouchEnable = false;
      client.publish("Kitchen-Touch", "OFF");
    }
    break;
  default:
    Serial.print("Err device in case-switch invalid.");
    break;
  }
}

void reconnect()
{
  // Loop until we're reconnected
  if (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqttClient, mqttName, mqttPASS, mqttWillTopic ,0 , true,"offline"))
    {
      // Serial.println("Connected");
      // Once connected, publish an announcement...
      client.publish(mqttWillTopic, "online", true);
      // ... and resubscribe
      client.subscribe(subscribeTopic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      count = count + 1;
    }
  }
}

void setup()
{
  pinMode(KitchenLightRelay, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(KitchenLightTouch, INPUT);

  Serial.begin(115200); // debug print on Serial Monitor
  setup_wifi();
  client.setServer(mqtt_server, mqttPORT);
  client.setCallback(callback);

// button.setEventHandler(handleEvent);

  ArduinoOTA.setHostname(mqttClient);
  ArduinoOTA.setPort(otaPort);
  ArduinoOTA.setPassword(otaPass);
  ArduinoOTA.begin();
}

void toggleSwitch(){
  if(ButtonState){
      digitalWrite(KitchenLightRelay, HIGH);
      ButtonState = false;
      client.publish("Kitchen-Light", "ON");
  }
  else{
      digitalWrite(KitchenLightRelay, LOW);
      client.publish("Kitchen-Light", "OFF");
      ButtonState = true;
    }

}

void loop()
{
  ArduinoOTA.handle();
  unsigned long currentMillis = millis();

  if (currentMillis - WifiDelayMillis >= WifiDelayInterval)
  {
    WifiDelayMillis = currentMillis;
    if (!client.connected())
    {
      Serial.println("reconnecting ...");
      reconnect();
    }
    else if (lastCount != count)
    {
      char buffer[200];
      updater["Disconnected"] = count;
      serializeJson(updater, buffer);
      client.publish(mqttWillTopic, buffer);
      lastCount = count;
    }
  }

  if((!digitalRead(KitchenLightTouch)) && touchState && TouchEnable){
    toggleSwitch();
     touchState = false;
     SwitchDelayMillis = currentMillis;
  }
 
 if (currentMillis - SwitchDelayMillis >= 1500) 
  {
    touchState = true;
  }

  client.loop();
}



 