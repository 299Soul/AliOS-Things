/**
 * ant_event.h
 *
 * ϵͳ�¼��ӿ�
 */
#ifndef __ANT_EVENT_H__
#define __ANT_EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ant_typedef.h"
#include "ant_os.h"

#define ANT_MSG_ID_IDX_BASE 0
/**
 * �¼�����
 */
typedef enum {
    ANT_EVENT_POWERON = ANT_MSG_ID_IDX_BASE,
    ANT_EVENT_SIMCARD_READY,                  // 1
    ANT_EVENT_KEY,                            // 2
    ANT_EVENT_KEY_FUNC_LONG_PRESSED,          // 3
    ANT_EVENT_KEY_COMB_FUNC_LONG_PRESSED,     // 4
    ANT_EVENT_AUDIO_PLAY_REQUEST,             // 5
    ANT_EVENT_AUDIO_PLAY_FINISHED,            // 6
    ANT_EVENT_NETWORK,                        // 7
    ANT_EVENT_NETWORK_SERVICE,                // 8  
    ANT_EVENT_NVRAM_REQ,                      // 9
    ANT_EVENT_NVRAM_RSP,                      // 10
    ANT_EVENT_DOWNLOAD_AUDIO,                 // 11
    ANT_EVENT_DOWNLOAD_ORDER,                 // 12
    ANT_EVENT_TIMESYNC,                       // 13
    ANT_EVENT_TIMESYNC_FINISH,                // 14
    ANT_EVENT_OTA,                            // 15
    ANT_EVENT_STAT_CHECK,                     // 16
    ANT_EVENT_IOT_SDK_INIT_REQ,               // 17
    ANT_EVENT_IOT_SDK_EVENT,                  // 18
    ANT_EVENT_ALIPAY_IOT_EVENT,               // 19
    // battery&charger management
    ANT_EVENT_BATTERY_CHARGE_IN,              // 20
    ANT_EVENT_BATTERY_CHARGE_OUT,             // 21
    ANT_EVENT_BATTERY_CHARGE_COMPLETE,        // 22
    ANT_EVENT_BATTERY_CHARGE_LOW_CURRENT,     // 23
    ANT_EVENT_BATTERY_CHARGE_OVER_CURRENT,    // 24
    ANT_EVENT_BATTERY_CHARGE_LOW_TEMPERATURE, // 25
    ANT_EVENT_BATTERY_CHARGE_OVER_TEMPERAURE, // 26
    ANT_EVENT_BATTERY_CHARGE_OVER_VOLTAGE,    // 27
    ANT_EVENT_BATTERY_CHARGE_STATUS,          // 28
    ANT_EVENT_USB_INSERT_STATUS,			  // 29
    // exception event
    ANT_EVENT_FS_CRASH,                       // 29
    ANT_EVENT_MEM_FAIL,                       // 30
    ANT_EVENT_MOS,                            // 31
    ANT_EVENT_LED_BLINK,                      // 32
    ANT_EVENT_FACTORY_RESET,                 // 33
    ANT_EVENT_BREAK_OFF_SHUTDOWN,             // 34
    ANT_EVENT_IDLE_EVENT,
    ANT_EVENT_VOICE_PAC_CLEAN,
    ANT_EVENT_END
} ant_event_e;


/**
 * ������ֵö��
 */
typedef enum {
    ANT_KEY_POWER  = 0,
    ANT_KEY_FUNC,
    ANT_KEY_VOLDN,
    ANT_KEY_VOLUP,
    ANT_KEY_RESET,
    ANT_KEY_UNDEF
} ant_keycode_e;

/**
 * ��������ö��
 */
typedef enum {
    ANT_KEYTYPE_PRESS       = 0,
    ANT_KEYTYPE_RELEASE,
    ANT_KEYTYPE_LONG_PRESS,
    ANT_KEYTYPE_REPEAT,
    ANT_KEYTYPE_UNDEF
} ant_keytype_e;

/**
 * �����ṹ�嶨��
 */
typedef struct {
    ant_keycode_e code;
    ant_keytype_e type;
} ant_key_event;


typedef struct{
    ant_u32 start;
    ant_u32 cnt;
    ant_u32 mode;
}ant_box_idle_t;

/**
 * ��Ϣ�¼��ṹ�嶨��
 */
typedef struct {
    ant_u32 message;         // ant message id
    ant_ptr param1;          // user defined value
    ant_ptr param2;          // user defined value
    ant_ptr param3;          // user defined value
    ant_ptr param4;          // user defined value
    ant_task_t src_task_id;  // task which send message
} ant_msg;

ant_bool ant_event_init(void);

/**
 * �����¼�
 *
 * @param msg_id �����¼���ȡֵ�μ�ant_event_e
 * @return  0�� ���ĳɹ�
 *         -1�� ����ʧ��
 */
ant_s32 ant_event_subscribe(ant_u32 msg_id);

/**
 * ȡ�������¼�
 *
 * @param msg_id �����¼���ȡֵ�μ�ant_event_e
 * @return  0�� ȡ�����ĳɹ�
 *         -1�� ȡ������ʧ��
 */
ant_s32 ant_event_unsubscribe(ant_u32 msg_id);

/**
 * �㲥�¼����������¼�id��ģ�飨���񣩶�����յ����¼�
 *
 * @param msg �㲥�¼�����
 */
void    ant_event_publish(ant_msg *msg);

/**
 * �����¼������յ��¼�֮ǰһֱ������
 *
 * @param msg ���ؽ��յ����¼�
 * @return  0�� �����¼��ɹ�
 *         -1�� �����¼�ʧ��
 */
ant_s32 ant_event_recv_msg(ant_msg *msg);

/**
 * ��ָ���������¼�
 * 
 * @param dst_task_id ����ID����ant_task_t)
 * @param msg �����¼���ָ��
 * @return  0�� �����¼��ɹ�
 *         -1�� �����¼�ʧ��
 */
ant_s32 ant_event_send_msg(ant_u32 dst_task_id, ant_msg *msg);

ant_s32 ant_event_from_system(ant_u32 msg_id);
void    ant_event_wait_bootup_completed(void);
const char *ant_event_description(ant_event_e event);

#ifdef __cplusplus
}
#endif

#endif

