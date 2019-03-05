#ifndef MQTT_SAMPLE_LAYER_H
#define MQTT_SAMPLE_LAYER_H


#include "mqtt.h"
#include "common.h"
/* Sensor libary head files */
#include <Servo.h>?
#include <ESP8266WiFi.h>
#include <DHT.h>

//MQTT
//#define ESP_SSID "bivl_adsl" // Your network name here
//#define ESP_PASS "0123456789" // Your network password here

#define HTTP_SERVER_ADDR    "183.230.40.33"
#define MQTT_SERVER_ADDR    "183.230.40.39"
#define HTTP_PORT     80                     // 80 = HTTP default port
#define MQTT_PORT     6002   

//#define LED_PIN  13
//#define LED_D 8
//#define SPEAKER_PIN 5
#define PROD_ID     "214410"//产品ID
#define SN          "cscbivl"//设备鉴权信息

#define CMD_GET_ALL_DATA "cmd_get_all_data"
#define CMD_GET_DTH_DATA "cmd_get_dth_data"//example cmd_get_dth_data:
#define CMD_GET_SERVO_POSTION "cmd_get_servo_position"
#define CMD_SET_SERVO_POSITION "cmd_set_servo_position"//example cmd_set_servo_position+90:

//DHT SENSOR
#define DHTPIN D4    // what digital pin DHT connected to
#define DHTTYPE DHT11   // DHT 11

#define SERVOPIN D5

extern DHT dht;
extern Servo myservo;

enum MqttEventType
{
    MQTT_EVENT_TYPE_CONNACK = 1,
    MQTT_EVENT_TYPE_PUBACK,
    MQTT_EVENT_TYPE_PUBREC,
    MQTT_EVENT_TYPE_PUBREL,
    MQTT_EVENT_TYPE_PUBCOMP,
    MQTT_EVENT_TYPE_SUBACK,
    MQTT_EVENT_TYPE_UNSUBACK,
    MQTT_EVENT_TYPE_PINGRESP,
    MQTT_EVENT_TYPE_GETCMD,
};


struct MqttSampleContext
{

    uint32_t sendedbytes;
    struct MqttContext mqttctx[1];
    struct MqttBuffer mqttbuf[1];

    const char *host;
    unsigned short port;

    char *proid;
    char *devid;
    char *apikey;

    int dup;
    enum MqttQosLevel qos;
    int retain;

    uint16_t pkt_to_ack;
    char cmdid[70];

    uint16_t  eventType;

    int keep_alive;

    //generate mqtt package id
    uint16_t pkt_id;

    char cmd[64];
    uint8_t cmd_arg;

};

extern struct MqttSampleContext ctx[1];

int MqttSample_Init();

boolean MqttSample_Setup();

bool MqttSample_keepalive();

void MqttSample_readPacket();

void MqttSample_pollingCmd();

void MqttSample_sendDHTdata();

void MqttSample_sendServoPos();

void MqttSample_setServoPos(int pos);

void die(char *msg);

#endif

