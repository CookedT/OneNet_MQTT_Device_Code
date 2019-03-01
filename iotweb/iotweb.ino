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

#define OFFICE_ADSL
#ifdef OFFICE_ADSL
const char* ssid = "Raiden";
const char* password = "chenjian";
#else
const char* ssid = "HUAWEI-WODEJIA";
const char* password = "lixiadian99";
#endif

char devid_temp[] = "517772285";
char API_KEY[] = "bHYoTlqzhICGHCH=3rZWTsUkAqY=";


//ESP8266
// Must declare output stream before Adafruit_ESP8266 constructor; can be
// a SoftwareSerial stream, or Serial/Serial1/etc. for UART.
//Adafruit_ESP8266 wifi(&Serial1, &Serial,-1);
//Adafruit_ESP8266 wifi(&Serial1, NULL,-1);

// Must call begin() on the stream(s) before using Adafruit_ESP8266 object.

/*Used for load device info with HTTP request OneNet server <<By Raiden 19/02/28>>*/
#if 0
#define REG_CODE    "RxoVewnsdmcaGjp5"
#define API_ADDR    "api.heclouds.com"
#define DEVICE_NAME     "arduino_87EF68"

#define REG_PKT_HEAD    "POST http://"API_ADDR"/register_de?register_code="REG_CODE" HTTP/1.1\r\n"\
                        "Host: "API_ADDR"\r\n"\
                        "Content-Length: "

#define REG_PKT_BODY    "{\"title\":\""DEVICE_NAME"\",\"sn\":\""SN"\"}"
#endif

//#define STRLEN 64
//char g_cmdid[STRLEN];

//printf support
static int serial_putchar(char c, FILE *stream)
{

    if (c == '\n')
        Serial.print('\r');

    Serial.print(c);
    return 0;
}

/*
boolean sendHttpPkt(char *phead, char *pbody)
{
    String content(phead);
    content = content+strlen(pbody)+"\r\n\r\n"+pbody;

    //String cmd("AT+CIPSEND=");
   // cmd = cmd + content.length()+ "\r\n";

    //wifi.print(cmd);
    //if(wifi.find(F("> ")))   // Wait for prompt
    {
        wifi.print(content); //send data
        return wifi.find(); // Gets 'SEND OK' line
    }
    return false;
}
*/

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

void setup()
{
    Serial.begin(9600);
    delay(10);

    Serial.println("board boot up , hello iot!\n");

    //DHT setup
    dht.begin();

    Serial.println("Start to setup Wifi!\n");
    //wifi setup
    if(!setupWIFI()) die("setup WIFI fail\n");

	Serial.println("Start to init Mqtt!\n");
    //load cloud device info
    MqttSample_Init();
    
    //TODO get device info from OneNet use HTTP
    if(!loadDeviceInfo()) die("load device info fail\n");

    //MQTT setup
    if(!MqttSample_Setup()) die("MqttSample_Setupfail\n");
	delay(5000);
	Serial.println("Exit Setup!");
}

uint32_t cnt =0;
void loop()
{
    // Wait a few seconds between measurements.
    Serial.println("Loop in!");
    delay(1000);

	
	//if ping fail , next will try 3 times , if still fail, just reconnect to server.
	 if(!MqttSample_keepalive())
		   return;
    Serial.println("Trying to read response from OneNet...");
    //recieve data from sever 
    MqttSample_readPacket();

	Serial.println("Polling Cmd...");
	//take action for server
    MqttSample_pollingCmd();
   // MqttSample_pollingtopic();


    //every 2s , device send data  to server
    cnt++;
    Serial.print("cnt =");
    Serial.println(cnt);
    if( 0 == cnt % 2 )
    {
        MqttSample_senddata();
       // MqttSample_pulish("sub_topic","hello", 5,MQTT_QOS_LEVEL1);
    }    
}

