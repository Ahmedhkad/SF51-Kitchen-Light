//Wifi Static IP
IPAddress local_IP(192, 168, 1, 10);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // optional
IPAddress secondaryDNS(8, 8, 4, 4); // optional
//WiFi
#define ssidWifi "WIFInameHere"   //Wifi SSID
#define passWifi "WIFIpassword"         //Wifi Password
//MQTT
#define mqttURL "192.168.1.2"      // MQTT Broker 
#define mqttName "MQTTname" 
#define mqttPASS "MQTTpassword" 
#define mqttClient "thisClientName"      
#define mqttPORT 1883
//Topics
#define subscribeTopic "TopicSub"
#define mqttWillTopic "devLife/willTopic" 
#define mqttDisconnectTopic "devDiscommect/device" 
//OTA
#define otaPass "longPassForOTAupdateOverWIFI"
#define otaPort 1000

 