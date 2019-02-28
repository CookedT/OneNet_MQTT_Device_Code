/*
  Raiden add 20190221
  For move mqtt function here, remove from ino files
 */

#include "mqtt_sample_layer.h"

WiFiClient client;

#define SETTIMEOUT_MQTT(duration) \
	client.setTimeout(duration)

//Global
DHT dht(DHTPIN, DHTTYPE);
struct MqttSampleContext ctx[1];

static void MqttSample_printbuffer(char *buf, int size)
{

    for(int i = 0; i < size; ++i)
    {
        if(0 == i % 64)
        {
            printf("\n        ");
        }
        const char c = buf[i];
        printf("%c",c);
    }
    printf("\n");
}


static int MqttSample_RecvPkt(void *arg, void *buf, uint32_t count)
{

   uint32_t previousMillis = millis();
   while(!client.available()) 
   {
     //yield();
     uint32_t currentMillis = millis();
     if(currentMillis - previousMillis >= ((int32_t) 2 * 1000))
	 {
	   printf("Recv time out.\n");
       return false;
     }
   }
   int read_size = 0;
   if(client.available())
   {
      read_size = client.read((uint8_t *)buf, count); 
   }
   return read_size;
}

static int MqttSample_SendPkt(void *arg, const struct iovec *iov, int iovcnt)
{
    int bytes;
    int i=0,j=0;

    //get length to send
    int length = 0;
	//printf("Send count = %d\n", iovcnt);
    for(i=0; i<iovcnt; ++i)
    {
        length += iov[i].iov_len;
    }
    
	for(i=0; i<iovcnt; ++i)
	{
	    int send_size = client.write((char*)iov[i].iov_base, iov[i].iov_len);
		//printf("The %d time size is : %d\n", i, send_size);	    
	}

    return length;

}

//------------------------------- packet handlers -------------------------------------------
static int MqttSample_HandleConnAck(void *arg, char flags, char ret_code)
{
    printf("Success to connect to the server, flags(%0x), code(%d).\n",
           flags, ret_code);
    bitSet(ctx->eventType,MQTT_EVENT_TYPE_CONNACK);
    return 0;
}



static int MqttSample_HandlePingResp(void *arg)
{
    printf("Recv the ping response.\n");
    bitSet(ctx->eventType,MQTT_EVENT_TYPE_PINGRESP);
    return 0;
}

static int MqttSample_HandlePublish(void *arg, uint16_t pkt_id, const char *topic,
                                    char *payload, uint32_t payloadsize,
                                    int dup, enum MqttQosLevel qos)
{
    struct MqttSampleContext *ctx = (struct MqttSampleContext*)arg;
    ctx->pkt_to_ack = pkt_id;
    ctx->dup = dup;
    ctx->qos = qos;


    //dump data
    printf("dup: %d, qos: %d, id: %d topic: %s payloadsize: %d ",
           dup, qos, pkt_id, topic, payloadsize);

    printf("payload:");
    MqttSample_printbuffer(payload, payloadsize);

    /*fix me : add response ?*/

    //get cmdid
    return 0;
}

static int MqttSample_HandlePubAck(void *arg, uint16_t pkt_id)
{
    printf("Recv the publish ack, packet id is %d.\n", pkt_id);
    bitSet(ctx->eventType,MQTT_EVENT_TYPE_PUBACK);
    return 0;
}



static int MqttSample_HandlePubRec(void *arg, uint16_t pkt_id)
{
    struct MqttSampleContext *ctx = (struct MqttSampleContext*)arg;
    ctx->pkt_to_ack = pkt_id;
    printf("Recv the publish rec, packet id is %d.\n", pkt_id);
    return 0;
}

static int MqttSample_HandlePubRel(void *arg, uint16_t pkt_id)
{
    struct MqttSampleContext *ctx = (struct MqttSampleContext*)arg;
    ctx->pkt_to_ack = pkt_id;
    printf("Recv the publish rel, packet id is %d.\n", pkt_id);
    return 0;
}

static int MqttSample_HandlePubComp(void *arg, uint16_t pkt_id)
{
    printf("Recv the publish comp, packet id is %d.\n", pkt_id);
    return 0;
}

static int MqttSample_HandleSubAck(void *arg, uint16_t pkt_id, const char *codes, uint32_t count)
{
    uint32_t i;
    printf("Recv the subscribe ack, packet id is %d, return code count is %d:.\n", pkt_id, count);
    bitSet(ctx->eventType,MQTT_EVENT_TYPE_SUBACK);
    return 0;
}


static int MqttSample_HandleUnsubAck(void *arg, uint16_t pkt_id)
{
    printf("Recv the unsubscribe ack, packet id is %d.\n", pkt_id);
    bitSet(ctx->eventType,MQTT_EVENT_TYPE_UNSUBACK);
    return 0;
}

static int readStringUntil( char* dst,char *src, int length,char terminator)
{
    if (length < 1) return 0;
    int index = 0;
    while (index < length)
    {
        char c = src[index];
        if ( c == terminator)
        {
            return index; // return number of characters, not including null terminator
        }
        *dst++ = (char)c;
        index++;
    }
    return -1; // not found  terminator
}

//simple cmd format, command_name:command_arg;
static int MqttSample_HandleCmd(void *arg, uint16_t pkt_id, const char *cmdid,
                                int64_t timestamp, const char *desc, char *cmdarg,
                                uint32_t cmdarg_len, int dup, enum MqttQosLevel qos)
{
    uint32_t i;
    struct MqttSampleContext *ctx = (struct MqttSampleContext*)arg;
    ctx->pkt_to_ack = pkt_id;
    strcpy(ctx->cmdid, cmdid);
	
	printf("-----\n\n\n");
    printf("CMD PACK: id is %d, cmduuid is %s, qos=%d, dup=%d.\n",
           pkt_id, cmdid, qos, dup);

    printf("Data length  %d, data:", cmdarg_len);
    MqttSample_printbuffer(cmdarg, cmdarg_len);

    //get cmd
    memset(ctx->cmd,0,cmdarg_len);
    int num = readStringUntil(ctx->cmd,cmdarg,cmdarg_len,':');
    if(num < 0 )
        return 0; //unkown cmd format, just ignore it

    //get arg
    ctx->cmd_arg = cmdarg[num+1] - '0'; // 1: on, 0:off

    bitSet(ctx->eventType,MQTT_EVENT_TYPE_GETCMD);

    return 0;
}

//--------------------------------------mqtt action ----------------------


static boolean MqttSample_ConnectServer()
{
    //MQTT CONNECT
    int i = 0;
	while((i++) < 3)
    {
      Serial.println("Trying to connect OneNet Mqtt Server!");
      if(client.connect(MQTT_HOST, MQTT_PORT))//0 = false
          return true;
	}
    return false;
}

static void MqttSample_disConnectServer()
{
    //MQTT CONNECT
    client.stop();
    printf("MQTT DISCONNECT!\n");

}


static boolean MqttSample_Connect(struct MqttSampleContext *ctx, char *proid\
                           , char *auth_info, const char *devid, int keep_alive, int clean_session)
{
    int err, flags;

    printf("mqtt connect -- product id: %s sn: %s deviceid: %s keepalive: %d cleansession: %d QoS: %d\n",
           proid, auth_info, devid, keep_alive, clean_session, MQTT_QOS_LEVEL0);

    err = Mqtt_PackConnectPkt(ctx->mqttbuf, keep_alive, devid,
                              clean_session, NULL,
                              NULL, 0,
                              MQTT_QOS_LEVEL0, 0, proid,
                              auth_info, strlen(auth_info));

    if(MQTTERR_NOERROR != err)
    {
        printf("Critical bug: failed to pack the connect packet.\n");
        return false;
    }

    int bytesend = Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
    MqttBuffer_Reset(ctx->mqttbuf);
    if(bytesend <= 0 )
    {
        LOG(ERROR,"Mqtt_SendPkt error! may need to retry send\n");

        return false;
    }

    if(Mqtt_RecvPkt(ctx->mqttctx) < 0)
        return false;
    //ctx->eventType is set by MqttSample_HandleConnAck
    if(bitRead(ctx->eventType,MQTT_EVENT_TYPE_CONNACK))
    {
        ctx->keep_alive = keep_alive;
        bitClear(ctx->eventType,MQTT_EVENT_TYPE_CONNACK);
        return true;
    }
    else
        return false;

}


static bool MqttSample_Subscribe(struct MqttSampleContext *ctx, const char **topic, int num)
{
    int err;
    uint16_t pkt_id = ctx->pkt_id;

    //sprintf(topic, "%s/%s/45523/test-1", ctx->proid, ctx->apikey);
    err = Mqtt_PackSubscribePkt(ctx->mqttbuf, 2, MQTT_QOS_LEVEL0, topic, num);
    if(err != MQTTERR_NOERROR)
    {
        printf("Critical bug: failed to pack the subscribe packet.\n");
        return false;
    }

    int bytesend = Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
    MqttBuffer_Reset(ctx->mqttbuf);
    if(bytesend <= 0 )
    {
        LOG(ERROR,"Mqtt_SendPkt error! may need to retry send\n");
        return false;
    }



    if(Mqtt_RecvPkt(ctx->mqttctx) < 0)
        return false;

    if(bitRead(ctx->eventType,MQTT_EVENT_TYPE_SUBACK))
    {
        //success , so can reuse current package id . ctx->pkt_id
        bitClear(ctx->eventType,MQTT_EVENT_TYPE_SUBACK);
        return true;
    }
    else
    {
        //fail, renew a next package id index
        ctx->pkt_id++;
        if (ctx->pkt_id == 0)
            ctx->pkt_id = 1;

        return false;
    }

}



static int MqttSample_Unsubscribe(struct MqttSampleContext *ctx, const char **topics, int num)
{
    int err;
    uint16_t pkt_id = ctx->pkt_id;

    err = Mqtt_PackUnsubscribePkt(ctx->mqttbuf, 3, topics, num);
    if(err != MQTTERR_NOERROR)
    {
        printf("Critical bug: failed to pack the unsubscribe packet.\n");
        return -1;
    }

    int bytesend = Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
    MqttBuffer_Reset(ctx->mqttbuf);
    if(bytesend <= 0 )
    {
        LOG(ERROR,"Mqtt_SendPkt error! may need to retry send\n");
        return false;
    }

    if(Mqtt_RecvPkt(ctx->mqttctx) < 0)
        return false;

    if(bitRead(ctx->eventType,MQTT_EVENT_TYPE_UNSUBACK))
    {
        //next package id index
        ctx->pkt_id++;
        if (ctx->pkt_id == 0)
            ctx->pkt_id = 1;
        bitClear(ctx->eventType,MQTT_EVENT_TYPE_UNSUBACK);
        return true;
    }
    else
    {
        //fail, renew a next package id index
        ctx->pkt_id++;
        if (ctx->pkt_id == 0)
            ctx->pkt_id = 1;

        return false;

    }

}

static bool MqttSample_pulish(const char *topic,const char *payload, uint32_t payload_len, enum MqttQosLevel qos)
{

    int err;
    uint16_t pkt_id = ctx->pkt_id;


    LOG(DEBUG,"topic %s \n",topic);
    err = Mqtt_PackPublishPkt(ctx->mqttbuf, pkt_id, topic, payload, payload_len, qos, 0, 1);
    if(err != MQTTERR_NOERROR)
    {
        printf("Critical bug: failed to pack the publish packet.\n");
        return false;
    }

    int bytesend = Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
    MqttBuffer_Reset(ctx->mqttbuf);
    if(bytesend <= 0 )
    {
        LOG(ERROR,"Mqtt_SendPkt error! may need to retry send\n");
        return false;
    }

    //curent support QOS0 ,QOS1

    if(qos == MQTT_QOS_LEVEL0)
        return true;

    if(qos == MQTT_QOS_LEVEL1)
    {
        if(Mqtt_RecvPkt(ctx->mqttctx) < 0)
            return false;

        if(bitRead(ctx->eventType,MQTT_EVENT_TYPE_PUBACK))
        {
            bitClear(ctx->eventType,MQTT_EVENT_TYPE_PUBACK);
            return true;
        }
        else
        {
            //next package id index
            ctx->pkt_id++;
            if (ctx->pkt_id == 0)
                ctx->pkt_id = 1;

            return false;
        }
    }


}

static bool MqttSample_pulish_SensorData(float humidity,float temperature)
{
    enum MqttQosLevel qos = MQTT_QOS_LEVEL1;
    int payload_len;
    char *payload;

    //build type =3 json format .
    String data = "{\"temperature\":\""+String(temperature)+"\","+"\"humidity\":\""+String(humidity)+"\"}";

    //send pkt
    payload_len = 1 + 2 + data.length();
    payload = (char *)malloc(payload_len);
    if(payload == NULL)
    {
        printf("<%s>: t_payload malloc error\r\n", __FUNCTION__);
        return false;
    }

    //type 3 means JSON format
    payload[0] = 3;

    //length 2 Bytes
    payload[1] = (data.length() & 0xFF00) >> 8;
    payload[2] = data.length() & 0xFF;

    //json content
    memcpy(payload+3, data.begin(), data.length());
    
    printf("Topic: %s QoS: %d Payload: %s\n",
           "$dp",qos, data.begin());
    //$dp means upload cmd
    bool ret = MqttSample_pulish("$dp",payload, payload_len,qos );
    free(payload);
    if(ret)
        return true;
    else
        LOG(ERROR,"MqttSample_pulish $dp error! \n");
    return false;

}

static void MqttSample_deInit()
{
    // reclaim the resource
    MqttBuffer_Destroy(ctx->mqttbuf);
    Mqtt_DestroyContext(ctx->mqttctx);

}

static  bool MqttSample_Ping()
{
    int err = Mqtt_PackPingReqPkt(ctx->mqttbuf);
    if(err != MQTTERR_NOERROR)
    {
        printf("Critical bug: failed to pack the ping packet.\n");
        return false;
    }

    int bytesend = Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
    MqttBuffer_Reset(ctx->mqttbuf);
    if(bytesend <= 0 )
    {
        LOG(ERROR,"Mqtt_SendPkt error! may need to retry send\n");
        return false;
    }

    if(Mqtt_RecvPkt(ctx->mqttctx) < 0)
        return false;

    if(bitRead(ctx->eventType,MQTT_EVENT_TYPE_PINGRESP))
    {
        bitClear(ctx->eventType,MQTT_EVENT_TYPE_PINGRESP);
        return true;
    }
    else
        return false;

}



static void MqttSample_pollingtopic()
{


}

static bool MqttSample_respcommand(char *resp)
{
    int err;
    enum MqttQosLevel Qos=MQTT_QOS_LEVEL1;
    int i = 0;
    int trycnt = 4;


    if(MQTT_QOS_LEVEL0==Qos)
    {
        err = Mqtt_PackCmdRetPkt(ctx->mqttbuf, 1, ctx->cmdid,
                                 resp, strlen(resp), MQTT_QOS_LEVEL0, 1);
    }
    else if(MQTT_QOS_LEVEL1==Qos)
    {
        err = Mqtt_PackCmdRetPkt(ctx->mqttbuf, ctx->pkt_id, ctx->cmdid,
                                 resp, strlen(resp), MQTT_QOS_LEVEL1, 1);
    }

    if(MQTTERR_NOERROR != err)
    {
        printf("Critical bug: failed to pack the cmd ret packet.\n");
        return false;
    }

    while(trycnt > 0)
    {
        trycnt--;


        int bytesend = Mqtt_SendPkt(ctx->mqttctx, ctx->mqttbuf, 0);
        if(bytesend <= 0 )
        {
            LOG(ERROR,"Mqtt_SendPkt error! may need to retry send\n");
            delay(100);
            continue; //retry
        }

        //MQTT_QOS_LEVEL0 , server will not send ack
        if(MQTT_QOS_LEVEL0==Qos)
        {
            MqttBuffer_Reset(ctx->mqttbuf);
            return true;
        }

        if(MQTT_QOS_LEVEL1==Qos)
        {
            LOG(DEBUG,"wait server ack response\n");
            if(Mqtt_RecvPkt(ctx->mqttctx) < 0)
            {
                delay(100);
                continue; //retry
            }

            MqttBuffer_Reset(ctx->mqttbuf);
            if(bitRead(ctx->eventType,MQTT_EVENT_TYPE_PUBACK))
            {
                bitClear(ctx->eventType,MQTT_EVENT_TYPE_PUBACK);
                return true;
            }
            else
            {
                //fail ,need to renew a new packge id
                if(Qos > MQTT_QOS_LEVEL0)
                {
                    //next package id index
                    ctx->pkt_id++;
                    if (ctx->pkt_id == 0)
                        ctx->pkt_id = 1;
                }

                return false;
            }
        }

    }

    return false;
}


void MqttSample_pollingCmd()
{

    if(bitRead(ctx->eventType,MQTT_EVENT_TYPE_GETCMD))
    {
        bitClear(ctx->eventType,MQTT_EVENT_TYPE_GETCMD);
        char rsp[]="ok";

        if(!MqttSample_respcommand(rsp))
        {
            LOG(ERROR,"CMD response error, resend it!\n");
        }
        else
        {
            //do something
            if(strcmp(ctx->cmd,"cmd_get_all_data") == 0 )//OneNet request to get all data from Arduino
            {
                MqttSample_senddata();
            }

            //do something
            if(strcmp(ctx->cmd,"led")== 0 )
            {
               
            }

        }

    }


}




//static boolean MqttSample_Setup();
//static void die(char *msg);

bool MqttSample_keepalive()
{
    static char pingfailcnt = 0;
    static unsigned long timestart = millis();

    if((millis() - timestart) > 10000 )
    {
       // LOG(DEBUG,"it is time to send ping to server, keep alive with server\n");
        timestart = millis();
        if(!MqttSample_Ping())
        {
            LOG(ERROR,"ping fail, server may inactive!!!!!!!\n");
            pingfailcnt++;
            if(pingfailcnt == 3 )
            {
                pingfailcnt = 0;
                //reconnect
                MqttSample_disConnectServer();
                //try to reconect
                if(!MqttSample_Setup())
                    die("MqttSample_Setupfail");
            }
            return false;
        }
        else
            pingfailcnt = 0 ; //success ping ,so reset and restart count
        return true;
    }

}


void MqttSample_senddata()
{
    //capture sensor data ...
    printf("<%s>: come in\r\n", __FUNCTION__);
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t))
    {
        LOG(ERROR,"Failed to read from DHT sensor!");
        return;
    }

    //report data to oneNet
  //  LOG(DEBUG,"publish sensor data:\n");
    if(!MqttSample_pulish_SensorData(h,t))
    {
        LOG(ERROR,"publish sensor data fail\n");
    }

}


int MqttSample_Init()
{

    int err;

    ctx->host = MQTT_HOST;
    ctx->port = MQTT_PORT;
    ctx->sendedbytes = -1;
    ctx->eventType = 0;
    ctx->keep_alive = 120;
    ctx->pkt_id = 1;
    ctx->devid = NULL;
    ctx->cmdid[0] = '\0';


    err = Mqtt_InitContext(ctx->mqttctx, 1024);
    if(MQTTERR_NOERROR != err)
    {
        printf("Failed to init MQTT context errcode is %d", err);
        return -1;
    }

    ctx->mqttctx->read_func = MqttSample_RecvPkt;
    ctx->mqttctx->writev_func = MqttSample_SendPkt;

    ctx->mqttctx->handle_conn_ack = MqttSample_HandleConnAck;
    ctx->mqttctx->handle_conn_ack_arg = ctx;
    ctx->mqttctx->handle_ping_resp = MqttSample_HandlePingResp;
    ctx->mqttctx->handle_ping_resp_arg = ctx;
    ctx->mqttctx->handle_publish = MqttSample_HandlePublish;
    ctx->mqttctx->handle_publish_arg = ctx;
    ctx->mqttctx->handle_pub_ack = MqttSample_HandlePubAck;
    ctx->mqttctx->handle_pub_ack_arg = ctx;
    ctx->mqttctx->handle_pub_rec = MqttSample_HandlePubRec;
    ctx->mqttctx->handle_pub_rec_arg = ctx;
    ctx->mqttctx->handle_pub_rel = MqttSample_HandlePubRel;
    ctx->mqttctx->handle_pub_rel_arg = ctx;
    ctx->mqttctx->handle_pub_comp = MqttSample_HandlePubComp;
    ctx->mqttctx->handle_pub_comp_arg = ctx;
    ctx->mqttctx->handle_sub_ack = MqttSample_HandleSubAck;
    ctx->mqttctx->handle_sub_ack_arg = ctx;
    ctx->mqttctx->handle_unsub_ack = MqttSample_HandleUnsubAck;
    ctx->mqttctx->handle_unsub_ack_arg = ctx;
    ctx->mqttctx->handle_cmd = MqttSample_HandleCmd;
    ctx->mqttctx->handle_cmd_arg = ctx;

    MqttBuffer_Init(ctx->mqttbuf);

    return 0;
}



boolean MqttSample_Setup()
{
    const char *topics[] = {"test_topic", "test_topic2"};
    int keep_alive = 120;
    int clean_session = 0;


	//make sure to clear mqtt recieve buffer  
	Mqtt_clearBuffer(ctx->mqttctx);

    //MQTT CONNECT
    if(MqttSample_ConnectServer())
    {
        printf("MQTT CONNECT!\n");
    
        if(!MqttSample_Connect(ctx, PROD_ID, SN, ctx->devid, ctx->keep_alive, clean_session))
        {
            printf("mqtt connect fail\n");
            return false;
        }

        //subscribe no need now <<By Raiden 2019/02/28>>
        /*
        if(!MqttSample_Subscribe(ctx, topics, 1)) 
        {
            printf("mqtt Subscribe fail\n");
        }
        */

        return true;
    }
    else
    {
        printf("MQTT CONNECT SERVER FAIL!\n");
        return false;
    }

}


void die(char *msg)
{
    LOG(ERROR," %s\n",msg);

    for(;;);
}

void MqttSample_readPacket()
{
    Mqtt_RecvPkt(ctx->mqttctx);
}



