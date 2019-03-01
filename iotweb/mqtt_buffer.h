#ifndef ONENET_MQTT_BUFFER_H
#define ONENET_MQTT_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "config.h"

struct MqttExtent {
    uint32_t len;
    char *payload;

    struct MqttExtent *next;
};

struct MqttBuffer {
    struct MqttExtent *first_ext;
    struct MqttExtent *last_ext;
    uint32_t available_bytes;

    char **allocations;
    char *first_available;
    uint32_t alloc_count;
    uint32_t alloc_max_count;
    uint32_t buffered_bytes;
};

/**
 * ��ʼ����������������������ʹ����󣬱���v @see MqttBuffer_Destroy��{
 * @param buf ����ʼ���Ļ���������
 */
void MqttBuffer_Init(struct MqttBuffer *buf);
/**
 * ��ٻ���������
 * @param buf �����ٵĻ���������
 */
void MqttBuffer_Destroy(struct MqttBuffer *buf);
/**
 * ����(���)����������
 * @param buf ������(���)�Ļ���������
 */
void MqttBuffer_Reset(struct MqttBuffer *buf);
/**
 * ����?���������ڴ�
 * @param buf ���ڷ��������������Ļ���������
 * @param size ��Ҫ����Ļ�������С���ֽ���?
 */
struct MqttExtent *MqttBuffer_AllocExtent(struct MqttBuffer *buf, uint32_t size);
/**
 * ��һ�����ݿ���ӵ���������ĩ��
 * @param buf �洢���ݿ�Ļ���������
 * @param payload ���ݿ���׵���
 * @param size ���ݿ�Ĵ�С���ֽ���?
 * @param own �0ʱ�����ݿ齫����������������?0ʱ�������������������v
 * @return �ɹ��򷵇� MQTTERR_NOERROR
 * @remark ��own?0ʱ�����뱣֤payload��bufδ�����ǰ?ֱ�Д`
 */
int MqttBuffer_Append(struct MqttBuffer *buf, char *payload, uint32_t size, int own);
/**
 * ��һ���������ڴ���ӵ���������ĩ��
 * @param buf �洢���ݿ�Ļ���������
 * @param ext ��Ҫ���뻺������������
 * @remark ��ext����buf����ģ����豣֤ext��buf������ǰ?ֱ�Д`
 */
void MqttBuffer_AppendExtent(struct MqttBuffer *buf, struct MqttExtent *ext);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ONENET_MQTT_BUFFER_H

