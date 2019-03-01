#ifndef ONENET_MQTT_H
#define ONENET_MQTT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#include <stdint.h>
#include <time.h>
#include "mqtt_buffer.h"

struct iovec {
    void *iov_base;
    size_t iov_len;
};



#define MQTT_DEFAULT_ALIGNMENT sizeof(int)

#define MQTT_HOST	"183.230.40.39"
#define MQTT_PORT	6002

#define HTTP_HOST	"183.230.40.33"
#define HTTP_PORT	80


/** MQTT������ */
enum MqttError {
    MQTTERR_NOERROR                  = 0,  /**< �ɹ����޴���*/
    MQTTERR_OUTOFMEMORY              = -1, /**< �ڴ治�� */
    MQTTERR_ENDOFFILE                = -2, /**< ������ʧ�ܣ��ѵ��ļ���β*/
    MQTTERR_IO                       = -3, /**< I/O���� */
    MQTTERR_ILLEGAL_PKT              = -4, /**< �Ƿ������ݰ� */
    MQTTERR_ILLEGAL_CHARACTER        = -5, /**< �Ƿ������� */
    MQTTERR_NOT_UTF8                 = -6, /**< �ַ����벻��UTF-8 */
    MQTTERR_INVALID_PARAMETER        = -7, /**< �������� */
    MQTTERR_PKT_TOO_LARGE            = -8, /**< ���ݰ����� */
    MQTTERR_BUF_OVERFLOW             = -9, /**< ��������� */
    MQTTERR_EMPTY_CALLBACK           = -10,/**< �ص�����Ϊ�� */
    MQTTERR_INTERNAL                 = -11,/**< ϵͳ�ڲ����� */
    MQTTERR_NOT_IN_SUBOBJECT         = -12,/**< ����Mqtt_AppendDPFinishObject����û��ƥ���Mqtt_AppendDPStartObject */
    MQTTERR_INCOMPLETE_SUBOBJECT     = -13,/**< ����Mqtt_PackDataPointFinishʱ�������������ݽṹ����` */
    MQTTERR_FAILED_SEND_RESPONSE     = -14 /**< ����publishϵ����Ϣ�󣬷�����Ӧ��ʧ�� */
};

/** MQTT���ݰ����] */
enum MqttPacketType {
    MQTT_PKT_CONNECT = 1, /**< �������������� */
    MQTT_PKT_CONNACK,     /**< ����ȷ�������� */
    MQTT_PKT_PUBLISH,     /**< �������������� */
    MQTT_PKT_PUBACK,      /**< ����ȷ�������� */
    MQTT_PKT_PUBREC,      /**< ���������ѽ������ݰ���Qos 2ʱ���ظ�MQTT_PKT_PUBLISH */
    MQTT_PKT_PUBREL,      /**< ���������ͷ����ݰ��� Qos 2ʱ���ظ�MQTT_PKT_PUBREC */
    MQTT_PKT_PUBCOMP,     /**< ����������ݰ��� Qos 2ʱ���ظ�MQTT_PKT_PUBREL */
    MQTT_PKT_SUBSCRIBE,   /**< ���������� */
    MQTT_PKT_SUBACK,      /**< ����ȷ�������� */
    MQTT_PKT_UNSUBSCRIBE, /**< ȡ������������ */
    MQTT_PKT_UNSUBACK,    /**< ȡ������ȷ�������� */
    MQTT_PKT_PINGREQ,     /**< ping ������ */
    MQTT_PKT_PINGRESP,    /**< ping ��Ӧ������ */
    MQTT_PKT_DISCONNECT   /**< �Ͽ����������� */
};

/** MQTT QOS�ȼ� */
enum MqttQosLevel {
    MQTT_QOS_LEVEL0,  /**< �ʶ෢��һ�� */
    MQTT_QOS_LEVEL1,  /**< ���ٷ���һ��  */
    MQTT_QOS_LEVEL2   /**< ֻ����һ�� */
};

/** MQTT ���������־λ���ڲ�ʹ�� */
enum MqttConnectFlag {
    MQTT_CONNECT_CLEAN_SESSION  = 0x02,
    MQTT_CONNECT_WILL_FLAG      = 0x04,
    MQTT_CONNECT_WILL_QOS0      = 0x00,
    MQTT_CONNECT_WILL_QOS1      = 0x08,
    MQTT_CONNECT_WILL_QOS2      = 0x10,
    MQTT_CONNECT_WILL_RETAIN    = 0x20,
    MQTT_CONNECT_PASSORD        = 0x40,
    MQTT_CONNECT_USER_NAME      = 0x80
};

/** ����ȷ�ϱ�־�� */
enum MqttConnAckFlag {
    MQTT_CONNACK_SP = 0x01 /**< �Ự���� */
};

/** MQTT ������ */
enum MqttRetCode {
    MQTT_CONNACK_ACCEPTED                  = 0, /**< �����ѽ��� */
    MQTT_CONNACK_UNACCEPTABLE_PRO_VERSION  = 1, /**< ��������֧�ָð汾��MQTTЭ��*/
    MQTT_CONNACK_IDENTIFIER_REJECTED       = 2, /**< ������Ŀͻ���ID */
    MQTT_CONNACK_SERVER_UNAVAILABLE        = 3, /**< ������������ */
    MQTT_CONNACK_BAD_USER_NAME_OR_PASSWORD = 4, /**< �û��������벻�ϛT */
    MQTT_CONNACK_NOT_AUTHORIZED            = 5, /**< ��Ȩʧ�� */

    MQTT_SUBACK_QOS0    = 0x00,  /**< ����ȷ��? QoS�ȼ�0 */
    MQTT_SUBACK_QOS1    = 0x01,  /**< ����ȷ��? QoS�ȼ�1 */
    MQTT_SUBACK_QOS2    = 0x02,  /**< ����ȷ��? QoS�ȼ�2 */
    MQTT_SUBACK_FAILUER = 0x80   /**< ����ʧ�� */
};

/** ���ݵ����ͣ��ڲ�ʹ�� */
enum MqttDataPointType {
    MQTT_DPTYPE_TRIPLE = 2,  /**< ��������������?ʱ��������ݵ�� */
    MQTT_DPTYPE_BINARY = 4,   /**< �������������ݵ����ݞ� */
    MQTT_DPTYPE_FLOAT = 7
};

/** MQTT ����ʱ������ */
struct MqttContext {
    char *bgn;
    char *end;
    char *pos;

    void *read_func_arg; /**< read_func�Ĺ����Δ` */
    int (*read_func)(void *arg, void *buf, uint32_t count);
        /**< ��ȡ���ݻص�������argΪ�ص����������Ĳ�����bufΪ��������
             ��Ż�������countΪbuf���ֽ��������ض�ȡ�����ݵ��ֽ���?
             ���ʧ�ܷ���-1����ȡ���ļ���β����0. */

    void *writev_func_arg; /**< writev_func�Ĺ����Δ` */
    int (*writev_func)(void *arg, const struct iovec *iov, int iovcnt);
        /**<  �������ݵĻص�����������Ϊ����د unix�е�writev?
              arg�ǻص������Ĺ���������iovcntΪiov����ĸ�����iovec��������?
              struct iovec {
                  void *iov_base;
                  size_t iov_len;
              }
			  ���ط��̵��ֽ��������ʧ�ܷ���-1.
        */

    void *handle_ping_resp_arg; /**< ����ping��Ӧ�Ļص������Ĺ������� */
    int (*handle_ping_resp)(void *arg); /**< ����ping��Ӧ�Ļص��������ɹ��򷵻طǸ��� */

    void *handle_conn_ack_arg; /**< ����������Ӧ�Ļص������Ĺ������� */
    int (*handle_conn_ack)(void *arg, char flags, char ret_code);
        /**< ����������Ӧ�Ļص������� flagsȡ?@see MqttConnAckFlag?
             ret_code��?Ϊ @see MqttRetCode? �ɹ��򷵻طǸ���
         */

    void *handle_publish_arg; /**< ���������ݵĻص������Ĺ������� */
    int (*handle_publish)(void *arg, uint16_t pkt_id, const char *topic,
                          char *payload, uint32_t payloadsize,
                          int dup, enum MqttQosLevel qos);
        /**< ���������ݵĻص������� pkt_idΪ���ݰ���ID��topic?
             ����ܲ����Topic? payloadΪ���ݵ���ʼ��ַ? payloadsize?
             payload���ֽ���? dupΪ�Ƿ��ط�״̬�� qosΪQoS�ȼ����ɹ����طǸ���?
			 SDK�����Զ����̶�Ӧ����Ӧ��?
         */

    void *handle_pub_ack_arg; /**< ����������ȷ�ϵĻص������Ĺ������� */
    int (*handle_pub_ack)(void *arg, uint16_t pkt_id);
        /**< ����������ȷ�ϵĻص���pkt_idΪ��ȷ�ϵķ����������ݰ���ID���ɹ��򷵻طǸ��` */

    void *handle_pub_rec_arg; /**< �����������ѽ��յĻص������Ĺ����Δ` */
    int (*handle_pub_rec)(void *arg, uint16_t pkt_id);
        /**< �����������ѽ��յĻص�������pkt_idΪ�������ѽ������ݰ���ID���ɹ��򷵻طǸ��` */

    void *handle_pub_rel_arg; /**< �������������ͷŵĻص������Ĺ����Δ` */
    int (*handle_pub_rel)(void *arg, uint16_t pkt_id);
        /**< �������������ͷŵĻص�����? pkt_idΪ�����������ͷ����ݰ���ID���ɹ��򷵻طǸ��` */

    void *handle_pub_comp_arg; /**< ��������������ɵĻص������Ĺ����Δ` */
    int (*handle_pub_comp)(void *arg, uint16_t pkt_id);
        /**< ��������������ɵĻص�������pkt_idΪ����������������ݰ���ID���ɹ��򷵻طǸ��` */

    void *handle_sub_ack_arg; /**< ������ȷ�ϵĻص����ݵĹ������� */
    int (*handle_sub_ack)(void *arg, uint16_t pkt_id,
                          const char *codes, uint32_t count);
        /**< ������ȷ�ϵĻص����ݣ� pkt_idΪ�������ݰ���ID?
             codesΪ@see MqttRetCode����˳���Ӧ�������ݰ��е�Topic?
             countΪcodes�ĸ������ɹ��򷵻طǸ���
         */

    void *handle_unsub_ack_arg; /**< ����ȡ������ȷ�ϵĻص������Ĺ������� */
    int (*handle_unsub_ack)(void *arg, uint16_t packet_id);
        /**< ����ȡ������ȷ�ϵĻص����`, pkt_idΪȡ���������ݰ���ID���ɹ��򷵻طǸ��` */

    void *handle_cmd_arg; /**< ��������Ļص������Ĺ������� */
    int (*handle_cmd)(void *arg, uint16_t pkt_id, const char *cmdid,
                      int64_t timestamp, const char *desc, char *cmdarg,
                      uint32_t cmdarg_len, int dup, enum MqttQosLevel qos);
        /**< ��������Ļص����`, cmdid Ϊ����ID? timestampΪ����ʱ�����Ϊ0��ʾ��ʱ���?
		     descΪ���������� cmdargΪ���������cmdarg_lenΪ����������ȣ�
             dupΪ�����Ƿ�Ϊ�ط�״�R�� qosΪQoS�ȼ�
			 �ɹ��򷵻طǸ���
         */
};

/**
 * ��ʼ��MQTT����ʱ������
 * @param ctx ��Ҫ����ʼ����MQTT����ʱ������
 * @param buf_size �������ݻ������Ĵ�С���ֽ���?
 * @return �ɹ��򷵇� @see MQTTERR_NOERROR
 * @remark ctx��ʼ���ɹ��󣬲���ʹ��ʱ����@see Mqtt_DestroyContext��{
 */
int Mqtt_InitContext(struct MqttContext *ctx, uint32_t buf_size);
/**
 * ���MQTT����ʱ������
 * @param ctx ��Ҫ�����ٵ�MQTT����ʱ������
 */
void Mqtt_DestroyContext(struct MqttContext *ctx);

void Mqtt_clearBuffer(struct MqttContext *ctx);

/**
 * �������ݰ���������ctx����Ӧ�����ݴ�����
 * @param ctx MQTT����ʱ������
 * @return �ɹ��򷵻�MQTTERR_NOERROR
 */
int Mqtt_RecvPkt(struct MqttContext *ctx);
/**
 * �������ݰ�
 * @param buf ���潫Ҫ�������ݰ��Ļ���������
 * @param offset �ӻ�������offset�ֽڴ���ʼ���{
 * @return �ɹ��򷵻�MQTTERR_NOERROR
 */
int Mqtt_SendPkt(struct MqttContext *ctx, const struct MqttBuffer *buf, uint32_t offset);


/**
 * ��װ��������������
 * @param buf �洢���ݰ��Ļ���������
 * @param keep_alive ����ʱ��������λ��
 * @param id �ͻ��˱�ʶ��
 * @param clean_session ?0ʱ������ʹ����һ�εĻỰ�������ϴλỰ�򴴽��µĻỰ?
 *        ?1ʱ��ɾ����һ�εĻỰ���������µĻỰ
 * @param will_topic "����"��Ϣ���̵���topic
 * @param will_msg "����"��Ϣ���������������豸����ʱ���Զ�������Ϣ���͵�will_topic
 * @param msg_len "����"��Ϣ�ĳ���
 * @param qos Qos�ȼ�
 * @param will_retain ?0ʱ������������will_msg�󣬽�ɾ��will_msg�����򽫱���will_msg
 * @param user �û���
 * @param password ����
 * @param pswd_len ���볤�ȣ��ֽ���?
 * @return �ɹ��򷵻�MQTTERR_NOERROR
 */
int Mqtt_PackConnectPkt(struct MqttBuffer *buf, uint16_t keep_alive, const char *id,
                        int clean_session, const char *will_topic,
                        const char *will_msg, uint16_t msg_len,
                        enum MqttQosLevel qos, int will_retain, const char *user,
                        const char *password, uint16_t pswd_len);

/**
 * ��װ��������������
 * @param buf �洢���ݰ��Ļ���������
 * @param pkt_id ���ݰ�ID����0
 * @param topic ���ݷ��̵��ĸ�topic
 * @param payload ��Ҫ�����������ݿ����ʼ��ַ
 * @param size ���ݿ��С���ֽ�����
 * @param qos QoS�ȼ�
 * @param retain �0ʱ������������publish��Ϣ���浽topic�£����滻���е�publish��Ϣ
 * @param own �0ʱ������payload��������
 * @return �ɹ��򷵻�MQTTERR_NOERROR
 * @remark ��own?0ʱ��payload������buf�����ٻ�����ǰ�����Д`
 */
int Mqtt_PackPublishPkt(struct MqttBuffer *buf, uint16_t pkt_id, const char *topic,
                        const char *payload, uint32_t size,
                        enum MqttQosLevel qos, int retain, int own);
/**
 * ���÷����������ݰ�Ϊ�ط��ķ����������ݰ�
 * @param buf �洢��PUBLISH���ݰ��Ļ�����
 * @return �ɹ��򷵻�MQTTERR_NOERROR
 */
int Mqtt_SetPktDup(struct MqttBuffer *buf);

/**
 * ��װ����������
 * @param buf �洢���ݰ��Ļ���������
 * @param pkt_id ���ݰ�ID? �0
 * @param qos QoS�ȼ�
 * @param topics ���ĵ�topic
 * @param topics_len  ���ĵ�topic ����
 * @return �ɹ�����MQTTERR_NOERROR
 */
int Mqtt_PackSubscribePkt(struct MqttBuffer *buf, uint16_t pkt_id,
                          enum MqttQosLevel qos, const char *topics[], int topics_len);
/**
 * ����cҪ���ĵ�Topic�����еĶ������ݰ���
 * @param buf �洢�������ݰ��Ļ���������
 * @param topic  ���ĵ�Topic
 * @param qos QoS�ȼ�
 * @return �ɹ�����MQTTERR_NOERROR
 */
int Mqtt_AppendSubscribeTopic(struct MqttBuffer *buf, const char *topic, enum MqttQosLevel qos);
/**
 * ��װȡ������������
 * @param buf �洢���ݰ��Ļ���������
 * @param pkt_id ���ݰ�ID
 * @param topics ��Ҫȡ�����ĵ�Topic�����ܰ���'#'��'+'
 * @param topics_len
 * @return �ɹ�����MQTTERR_NOERROR
 */
int Mqtt_PackUnsubscribePkt(struct MqttBuffer *buf, uint16_t pkt_id, const char *topics[], int topics_len);
/**
 * ����cҪȡ�����ĵ�Topic�����е�ȡ���������ݰ���
 * @param buf �洢ȡ���������ݰ��Ļ���������
 * @param topic �cҪȡ����Topic
 * @return �ɹ�����MQTTERR_NOERROR
 */
int Mqtt_AppendUnsubscribeTopic(struct MqttBuffer *buf, const char *topic);
/**
 * ��װping������
 * @param buf �洢���ݰ��Ļ���������
 * @return �ɹ�����MQTTERR_NOERROR
 */
int Mqtt_PackPingReqPkt(struct MqttBuffer *buf);
/**
 * ��װ�Ͽ�����������
 * @param buf �洢���ݰ��Ļ���������
 * @return �ɹ�����MQTTERR_NOERROR
 */
int Mqtt_PackDisconnectPkt(struct MqttBuffer *buf);

/**
 * ��װ�����������(OneNet��չ)
 * @param buf �洢���ݰ��Ļ���������
 * @param pkt_id ���ݰ�ID����0
 * @param cmdid �������ݶ�Ӧ������ID
 * @param ret ����������ʼ��ַ
 * @param ret_len ���������ֽڔ`
 * @param own �0ʱ������ret��������
 * @return �ɹ�����MQTTERR_NOERROR
 * @remark ��own?0ʱ��ret����ȷ����buf�����ٻ�����ǰһֱ�Д`
 */
int Mqtt_PackCmdRetPkt(struct MqttBuffer *buf, uint16_t pkt_id, const char *cmdid,
                       const char *ret, uint32_t ret_len,  enum MqttQosLevel qos, int own);
/**
 * ��ʼ��װ@see MQTT_PKT_TRIPLE���͵����ݵ㣨OneNet��չ?
 * @param buf ���ڴ洢���ݰ��Ļ���������
 * @param pkt_id ���ݰ�ID����0
 * @param qos QoS�ȼ�
 * @param retain �0ʱ������������publish��Ϣ���浽topic�£����滻���е�publish��Ϣ
 * @param topic �0ʱ��$dp����� $crsp
 * @return �ɹ�����MQTTERR_NOERROR
 * @remark ��������ݵ��������Ӻ��c����@see Mqtt_PackDataPointFinish?
 *         �ط�ʱ������@see Mqtt_SetPktDup �������ݰ�Ϊ�ط�״ʹ
 */
int Mqtt_PackDataPointStart(struct MqttBuffer *buf, uint16_t pkt_id,
                            enum MqttQosLevel qos, int retain, int topic);
/**
 * ��ӿ�?���ݵ㣬�������retain������
 * @param buf �洢MQTT_DPTYPE_TRIPLE���͵����ݰ��Ļ���������
 * @param dsid ������id
 * @return �ɹ�����MQTTERR_NOERROR
 * @remark ����Mqtt_PackDataPointStart��ʼ��װ���ݵ�ʱ��retain�c����?0?
 *         Ȼ��ͨ����ӿ�?���ݵ�ķ�ʽ���������retain������
 */
int Mqtt_AppendDPNull(struct MqttBuffer *buf, const char *dsid);
/**
 * ��������������ݵ�@see MQTT_PKT_TRIPLE���͵����ݰ�?
 * @param buf �洢MQTT_DPTYPE_TRIPLE���͵����ݰ��Ļ���������
 * @param dsid ������ID
 * @param ts  ��������ʱ�䣬��1970-01-01T00:00:00.000��ʼ�ĺ���ʱ�����
 *            ?0����ʱ��ϵͳȡĬ��ʱ��
 * @param value ���ݵ�ı�
 * @return �ɹ�����MQTTERR_NOERROR
 */
int Mqtt_AppendDPInt(struct MqttBuffer *buf, const char *dsid, int64_t ts, int value);
/**
 * ����ַ����������ݵ�@see MQTT_PKT_TRIPLE���͵����ݰ�?
 * @param buf �洢MQTT_DPTYPE_TRIPLE���͵����ݰ��Ļ���������
 * @param dsid ������ID
 * @param ts  ��������ʱ�䣬��1970-01-01T00:00:00.000��ʼ�ĺ���ʱ�����
 *            ?0����ʱ��ϵͳȡĬ��ʱ��
 * @param value ���ݵ�ı�
 * @return �ɹ�����MQTTERR_NOERROR
 */
int Mqtt_AppendDPString(struct MqttBuffer *buf, const char *dsid, int64_t ts, const char *value);
/**
 * ��Ӹ������������ݵ�@see MQTT_PKT_TRIPLE���͵����ݰ�?
 * @param buf �洢MQTT_DPTYPE_TRIPLE���͵����ݰ��Ļ���������
 * @param dsid ������ID
 * @param ts  ��������ʱ�䣬��1970-01-01T00:00:00.000��ʼ�ĺ���ʱ�����
 *            ?0����ʱ��ϵͳȡĬ��ʱ��
 * @param value ���ݵ�ı�
 * @return �ɹ�����MQTTERR_NOERROR
 */
int Mqtt_AppendDPDouble(struct MqttBuffer *buf, const char *dsid, int64_t ts, double value);
/**
 * ��Ӹ����������ݵ�@see MQTT_PKT_TRIPLE���͵����ݰ�?
 * @param buf �洢MQTT_DPTYPE_TRIPLE���͵����ݰ��Ļ���������
 * @param dsid ������ID
 * @param ts  ��������ʱ�䣬��1970-01-01T00:00:00.000��ʼ�ĺ���ʱ�����
 *            ?0����ʱ��ϵͳȡĬ��ʱ��
 * @return �ɹ�����MQTTERR_NOERROR
 * @remark ��ɸ��϶����������Ӻ������ @see Mqtt_AppendDPFinishObject
 */
int Mqtt_AppendDPStartObject(struct MqttBuffer *buf, const char *dsid, int64_t ts);
/**
 * ��������������ݶε����ݵ�ĸ������Ͷ���?
 * @param buf �洢MQTT_DPTYPE_TRIPLE���͵����ݰ��Ļ���������
 * @param name ���ݶε�����
 * @param value ���ݶεı�
 * @return �ɹ�����MQTTERR_NOERROR
 */
int Mqtt_AppendDPSubvalueInt(struct MqttBuffer *buf, const char *name, int value);
/**
 * ����ַ����������ݶε����ݵ�ĸ������Ͷ�����
 * @param buf �洢MQTT_DPTYPE_TRIPLE���͵����ݰ��Ļ���������
 * @param name ���ݶε�����
 * @param value ���ݶεı�
 * @return �ɹ�����MQTTERR_NOERROR
 */
int Mqtt_AppendDPSubvalueString(struct MqttBuffer *buf, const char *name, const char *value);
/**
 * ��Ӹ������������ݶε����ݵ�ĸ������Ͷ�����
 * @param buf �洢MQTT_DPTYPE_TRIPLE���͵����ݰ��Ļ���������
 * @param name ���ݶε�����
 * @param value ���ݶεı�
 * @return �ɹ�����MQTTERR_NOERROR
 */
int Mqtt_AppendDPSubvalueDouble(struct MqttBuffer *buf, const char *name, double value);
/**
 * ��ʼ����Ӹ������͵�����
 * @param buf �洢MQTT_DPTYPE_TRIPLE���͵����ݰ��Ļ���������
 * @param name �Ӹ������͵�����
 * @return �ɹ�����MQTTERR_NOERROR
 * @remark ����Ӹ����������ݵ���Ӻ��c���� @see  Mqtt_AppendDPFinishSubobject
 */
int Mqtt_AppendDPStartSubobject(struct MqttBuffer *buf, const char *name);
/**
 * ����Ӹ����������ݵ���ӣ�@see Mqtt_AppendDPStartSubobject
 * @param buf �洢MQTT_DPTYPE_TRIPLE���͵����ݰ��Ļ���������
 * @return �ɹ�����MQTTERR_NOERROR
 */
int Mqtt_AppendDPFinishSubobject(struct MqttBuffer *buf);
/**
 * ��ɸ����������ݵ���ӣ� @see Mqtt_AppendDPStartObject
 * @param buf �洢MQTT_DPTYPE_TRIPLE���͵����ݰ��Ļ���������
 * @return �ɹ�����MQTTERR_NOERROR
 */
int Mqtt_AppendDPFinishObject(struct MqttBuffer *buf);
/**
 * ������װ���ݞ�
 * @param buf �洢@see MqttDataPointType�������ݰ��Ļ���������
 * @return �ɹ�����MQTTERR_NOERROR
 */
int Mqtt_PackDataPointFinish(struct MqttBuffer *buf);
/**
 * ��װ�������������ݵ㣨OneNet��չ?
 * @param buf �洢���ݰ��Ļ���������
 * @param pkt_id ���ݰ�ID����0
 * @param dsid ������ID
 * @param desc ���ݵ��������Ϣ
 * @param time ��������ʱ�䣬��1970-01-01T00:00:00.000��ʼ�ĺ���ʱ�����
 *             ?0����ʱ��ϵͳȡĬ��ʱ��
 * @param bin ���������ݵ���ʼ��ַ
 * @param size ���������ݵ��ֽڔ`
 * @param qos QoS�ȼ�
 * @param retain �0ʱ������������publish��Ϣ���浽topic�£����滻���е�publish��Ϣ
 * @param own �0ʱ������bin��������
 * @param save �0ʱ�����������洢���ݣ����򲻋�
 * @return �ɹ�����MQTTERR_NOERROR
 * @remark ��own?0ʱ��bin������buf�����ٻ�����ǰ�����Д`
 */
int Mqtt_PackDataPointByBinary(struct MqttBuffer *buf, uint16_t pkt_id, const char *dsid,
                               const char *desc, int64_t time, const char *bin,
                               uint32_t size, enum MqttQosLevel qos, int retain, int own, int save);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // ONENET_MQTT_H

