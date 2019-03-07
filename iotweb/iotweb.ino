// +----------------------------------------------------------------------
// | AdminIOT
// +----------------------------------------------------------------------
// | Copyright (c) 2017 https://www.adminiot.com.cn All rights reserved.
// +----------------------------------------------------------------------
// | Licensed (Without the authorship of the author, the code can not be
// | transmitted two times or used for other business practices)
// +----------------------------------------------------------------------
// | Author: Robert <78320701@qq.com> Date:2017/12
// +----------------------------------------------------------------------

//#include <Adafruit_ESP8266.h>
//#include <ESP8266WiFi.h>
//#include <stdio.h>
//#include "cJSON.h"
//#include "log.h"
#include "mqtt_sample_layer.h"


const char* ssid = "bivl_adsl";
const char* password = "0123456789";


char devid_temp[] = "";
char API_KEY[] = "";


boolean setupWIFI()
{
    // Connect to WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    //WiFi no need to define at D1 Wemos
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());

    return true;
}

//TODO change to use ESP8266 s
boolean loadDeviceInfo()
{
    //TODO get following info by http request OneNet server
    //TODO end
    
   ctx->devid = (char *)malloc(strlen(devid_temp)+1);
   if(NULL != ctx->devid)
      strncpy(ctx->devid, devid_temp, strlen(devid_temp)+1);
   else
   	  goto malloc_fail;
   
   ctx->apikey = (char *)malloc(strlen(API_KEY)+1);
   if(NULL != ctx->apikey)
   	  strncpy(ctx->apikey, API_KEY, strlen(API_KEY)+1);
   else
   	  goto malloc_fail;
   
   return 1;

   malloc_fail: return 0; 
}

static void sensorInit()
{
    //DHT setup
    dht.begin();

	//Servo setiup
	myservo.attach(SERVOPIN);
}

static void MqttProtocolInit()
{
	//load cloud device info
    MqttSample_Init();
    
    //TODO get device info from OneNet use HTTP
    if(!loadDeviceInfo()) die("load device info fail\n");

    //MQTT setup
    if(!MqttSample_Setup()) die("MqttSample_Setupfail\n");
}

static void MqttDataRecvLoop()
{
	//if ping fail , next will try 3 times , if still fail, just reconnect to server.
	if(!MqttSample_keepalive())
		   return;
    //recieve data from sever 
    MqttSample_readPacket();

	//take action for server
    MqttSample_pollingCmd();
   // MqttSample_pollingtopic();
}

static void MqttDataPublish(uint32_t cnt)
{
	if(0 == cnt % 2)
	    MqttSample_sendDHTdata();

	/*Add other sensor publish here*/
	
	/*Add end*/
}

void setup()
{
    Serial.begin(9600);
    delay(10);

    Serial.println("board boot up , hello iot!\n");

	//wifi setup
    if(!setupWIFI()) die("setup WIFI fail\n");

	//All sensor init
	sensorInit();
    
    //Mqtt init
    MqttProtocolInit();

    //Delay enough time for mqtt init done
	delay(5000);
}

uint32_t cnt =0;
void loop()
{
    // Wait a few seconds between measurements.
    delay(1000);

	MqttDataRecvLoop();//Board receive cmd or publish from server then reply and excute

	MqttDataPublish(cnt);//Board publish data to server every few seconds.

    cnt++;
    if( cnt > 1000 )
    {
        cnt = 0;
    }    
}

