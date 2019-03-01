#include <HttpPacket.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <dht.h>
HttpPacketHead packet;
#define myPeriodic 5
dht DHT;
#define DHT11_PIN 2
int L = 0; //LED指示灯引�?int sent = 0;
char OneNetServer[] = "api.heclouds.com";       //不需要修�?
const char ssid[] = "dtsdsy09";     //修改为自己的路由器用户名
const char password[] = ""; //修改为自己的路由器密�?
char device_id[] = "4452619";    //修改为自己的设备ID
char API_KEY[] = "kB=6Muq8vQlwyMOHXWP9OajINno=";    //修改为自己的API_KEY
char sensor_id1[] = "TEMP";
char sensor_id2[] = "HUMI";
void connectWifi()
{  
  Serial.print("Connecting to " + *ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");  
  }
  Serial.println("");
  Serial.println("Connected");
  Serial.println("");
  digitalWrite(0, HIGH);
  delay(1000);
  digitalWrite(0, LOW);
  delay(1000);
  digitalWrite(0, HIGH);
}
void postDataToOneNet(char* API_VALUE_temp, char* device_id_temp, char* sensor_id_temp, float thisData)
{ 
  WiFiClient client;
  StaticJsonBuffer<250> jsonBuffer;
  JsonObject& myJson = jsonBuffer.createObject();
  JsonArray& datastreams= myJson.createNestedArray("datastreams");
  JsonObject& id_datapoints = datastreams.createNestedObject();
  id_datapoints["id"] = sensor_id_temp;
    JsonArray& datapoints = id_datapoints.createNestedArray("datapoints");
  JsonObject& value = datapoints.createNestedObject();
    value["value"] =thisData;
  char p[180];
  myJson.printTo(p, sizeof(p)); 
  packet.setHostAddress(OneNetServer);
  packet.setDevId(device_id_temp);   //device_id
  packet.setAccessKey(API_VALUE_temp);  //API_KEY
  /*create the http message about add datapoint */
  packet.createCmdPacket(POST, TYPE_DATAPOINT, p);
  if (strlen(packet.content))
    Serial.print(packet.content);
  Serial.println(p);
  char cmd[400];
  memset(cmd, 0, 400);  
  strcpy(cmd, packet.content);
  strcat(cmd, p);
  if (client.connect(OneNetServer, 80)) { 
    Serial.println("WiFi Client connected ");
    client.print(cmd);
    delay(1000);
  }//end if
  //  Serial.println(cmd);
  client.stop();
}
void readDHT11()
{
   // READ DATA
  Serial.print("DHT11, \t");
  int chk = DHT.read11(DHT11_PIN);
  switch (chk)
  {
    case DHTLIB_OK:  
    Serial.print("OK,\t"); 
    break;
    case DHTLIB_ERROR_CHECKSUM: 
    Serial.print("Checksum error,\t"); 
    break;
    case DHTLIB_ERROR_TIMEOUT: 
    Serial.print("Time out error,\t"); 
    break;
       
    
  }
  // DISPLAY DATA
  Serial.print(DHT.humidity, 1);
  Serial.print(",\t");
  Serial.println(DHT.temperature, 1);
  delay(2000);
}
void setup() {
  Serial.begin(115200);
  pinMode(0, OUTPUT);
  pinMode(4, INPUT);
  connectWifi();
  Serial.println("setup end!");
   Serial.println("DHT TEST PROGRAM ");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHT_LIB_VERSION);
  Serial.println();
  Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)");
}
void loop() {
  readDHT11();
  int temp =(DHT.temperature); int hum = (DHT.humidity);
  
  postDataToOneNet(API_KEY, device_id, sensor_id1, temp);
  delay(100);
  postDataToOneNet(API_KEY, device_id, sensor_id2, hum);
  Serial.println("N0 " + String(sent) + " Stream: was send");
  sent++;
  int count = myPeriodic;
  while (count--)
    delay(1000);
}