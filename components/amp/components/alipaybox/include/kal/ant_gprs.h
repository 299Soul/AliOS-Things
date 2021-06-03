#ifndef __ANT_GPRS_H__
#define __ANT_GPRS_H__
#include "ant_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_GPRS_USER_NAME_LEN 32
#define MAX_GPRS_PASSWORD_LEN  32
#define MAX_GPRS_APN_LEN       100

#define GPRS_SIGNAL_WEAK       (50)
#define GPRS_SIGNAL_NORMAL     (80)


#define MAX_GPRS_USER_NAME_LEN 32
#define MAX_GPRS_PASSWORD_LEN  32
#define MAX_GPRS_APN_LEN       100

#define GPRS_SIGNAL_WEAK       (50)
#define GPRS_SIGNAL_NORMAL     (80)

typedef enum{
    ANT_TYPE_GPRS =0,
    ANT_TYPE_LTE,
    ANT_TYPE_UNKONWN
}ant_network_type_e;

typedef enum {
    ANT_SIM_ID_NONE = -1,
    ANT_SIM_ID_SIM1 = 0,  /* sim card one */
    ANT_SIM_ID_SIM2,	  /* sim card two */
    ANT_SIM_ID_SIM3,	  /* sim card there */
    ANT_SIM_ID_SIM4,	  /* sim card four */
    ANT_SIM_ID_NUM		  /* total sim card number */
} ant_sim_id_e;

/**
 * GPRS
 */
typedef enum {
    ANT_GPRS_SA_SEARCHING,       /* Searching network */
    ANT_GPRS_SA_NO_SERVICE,      /* No service */
    ANT_GPRS_SA_LIMITED_SERVICE, /* Limited service, emergency call only */
    ANT_GPRS_SA_FULL_SERVICE,    /* Full service */
} ant_gprs_sa_e;

typedef enum {
    ANT_GPRS_BAND_NONE        = 0x00,     /* No network */
    ANT_GPRS_BAND_GSM900      = 0x02,     /* GSM900 */
    ANT_GPRS_BAND_DCS1800     = 0x08,     /* DCS1800 */
    ANT_GPRS_BAND_PCS1900     = 0x10,     /* PCS1900 */
    ANT_GPRS_BAND_GSM450      = 0x20,     /* GSM450 */
    ANT_GPRS_BAND_GSM480      = 0x40,     /* GSM480 */
    ANT_GPRS_BAND_GSM850      = 0x80,     /* GSM850 */

    ANT_GPRS_BAND_SCDMA       = 0x0101,   /* �й��ƶ�TD-SCDMA, up: 2010-2025MHz, down: 2010-2025MHz */
    ANT_GPRS_BAND_LTE1880     = 0x0102,   /* �й��ƶ�TD-LTE,   up: 1880-1890MHz, down: 1880-1890MHz */
    ANT_GPRS_BAND_LTE2320     = 0x0103,   /* �й��ƶ�TD-LTE,   up: 2320-2370MHz, down: 2320-2370MHz */
    ANT_GPRS_BAND_LTE2575     = 0x0104,   /* �й��ƶ�TD-LTE,   up: 2575-2635MHz, down: 2575-2635MHz */
    ANT_GPRS_BAND_WCDMA       = 0x0201,   /* �й���ͨWCDMA,    up: 1940-1955MHz, down: 1940-1955MHz */
    ANT_GPRS_BAND_LTE2300     = 0x0202,   /* �й���ͨTD-LTE,   up: 2300-2320MHz, down: 2300-2320MHz */
    ANT_GPRS_BAND_LTE2555     = 0x0203,   /* �й���ͨTD-LTE,   up: 2555-2575MHz, down: 2555-2575MHz */
    ANT_GPRS_BAND_LTE1850     = 0x0204,   /* �й���ͨTD-LTE,   up: 1755-1765MHz, down: 1850-1860MHz */
    ANT_GPRS_BAND_CDMA2000    = 0x0301,   /* �й�����CDMA2000, up: 1920-1935MHz, down: 2110-2125MHz */
    ANT_GPRS_BAND_LTE2370     = 0x0302,   /* �й�����TD-LTE,   up: 2370-2390MHz, down: 2370-2390MHz */
    ANT_GPRS_BAND_LTE2635     = 0x0303,   /* �й�����TD-LTE,   up: 2635-2655MHz, down: 2635-2655MHz */
    ANT_GPRS_BAND_LTE1860     = 0x0304,   /* �й�����FDD-LTE,  up: 1765-1780MHz, down: 1860-1875MHz */
    ANT_GPRS_BAND_OTHERS      = 0xf000,   /* Other band not defined above. */
} ant_gprs_band_e;

typedef enum {
    ANT_GPRS_DATA_STATUS_NONE             = 0x00,   /* No coverage */
    ANT_GPRS_DATA_STATUS_COVERAGE         = 0x01,   /* In coverage */
    ANT_GPRS_DATA_STATUS_ATTACHED         = 0x02,   /* Attached */
    ANT_GPRS_DATA_STATUS_PDP_ACTIVATED    = 0x04,   /* PDP activated */
    ANT_GPRS_DATA_STATUS_UPLINK           = 0x08,   /* DN uplink */
    ANT_GPRS_DATA_STATUS_DOWNLINK         = 0x10,   /* DN downlink */
} ant_gprs_data_status_e;

typedef enum {
    ANT_GPRS_EVENT_NETWORK_UNREGISTER = 0,
    ANT_GPRS_EVENT_NETWORK_REGISTER = 1,
    ANT_GPRS_EVENT_PDP_ACTIVE = 2,
} ant_gprs_event_e;



typedef void (*ant_gprs_callback_t)(ant_gprs_event_e event, void *param);

/**
 * GPRS(2G)��PDP(4G/CAT1)������·����
 *
 * @param sim_id SIM��ID
 *
 * @return = 0: ��ʼ���ɹ���������·�ɹ�����
 *         < 0: ������·��ʼ��ʧ��
 */
ant_s32 ant_gprs_init(ant_sim_id_e sim_id);

void ant_gprs_term(void);

/**
 * ע��GPRS(2G)��PDP(4G/CAT1)������·��ʱ��ص�
 *
 * @param callback �ص�����
 * @param param �û��ص��������ڻص�����ִ��ʱ����
 *
 * @return  = 0: ע��ɹ�
 *          < 0: ע��ʧ��
 */
ant_s32 ant_gprs_register_callback(ant_gprs_callback_t callback, void *param);

/**
 * ��ȡ����IPv4��ַ
 *
 * @param ipaddr IPv4��ַ���ڲ���
 *
 * @return  = 0: ��ȡ�ɹ�
 *          < 0: ��ȡʧ�ܣ�����ЧIPv4��ַ
 */
ant_s32 ant_gprs_getlocalipaddr(ant_u32 *ipaddr);

/**
 * ��ȡDNS������IP��ַ
 *
 * @param addr1 ����DNS1 IP��ַ
 * @param addr2 ����DNS2 IP��ַ
 *
 * @return  = 0: ��ȡ�ɹ�
 *          < 0: ��ȡʧ��
 */
ant_s32 ant_gprs_getdnsaddr(ant_u32 *addr1, ant_u32 *addr2);

/**
 * ����DNS������IP��ַ
 *
 * @param addr1 DNS1 IP��ַ
 * @param addr2 DNS2 IP��ַ
 *
 * @return  = 0: ���óɹ�
 *          < 0: ����ʧ��
 */
ant_s32 ant_gprs_setdnsaddr(ant_u32 addr1, ant_u32 addr2);

/**
 * ��ȡ��ǰ�ź�ǿ��
 *
 * @return �ź�ǿ�Ȱٷֱ�: 0~100
 */
ant_s32 ant_gprs_get_signal_level(void);

/**
 * ��ȡ��ǰ�źŵ�RSSI
 *
 * @return ������db�����������ź�ǿ��
 */
ant_s32 ant_gprs_get_signal_rssi(void);

/**
 * ��ȡ��ǰ���������˺�
 *
 * @return ��ǰ���������˺�
 */
ant_u32 ant_gprs_get_accountid(void);

/**
 * ��ȡ��ǰ����״̬
 *
 * @return ���ص�ǰ����״̬��ȡֵ��Χ�μ�ant_gprs_sa_e
 */
ant_s32 ant_gprs_get_service_availablity(void);

/**
 * ��ȡ��ǰƵ��
 *
 * @return ���ص�ǰ����Ƶ�Σ�ȡֵ��Χ�μ�ant_gprs_band_e
 */
ant_s32 ant_gprs_get_band(void);

/**
 * ��ȡ��ǰ���ӻ�վ�ĵ���λ��
 *
 * @param lac ���ص�ǰ���ӻ�վ��lac
 * @param cell_id ���ص�ǰ���ӻ�վ��cell_id
 *
 * @return
 */
ant_s32 ant_gprs_get_location(ant_u32 *lac, ant_u32 *cell_id);

/**
 * ��ȡ��ǰ��������״̬
 *
 * @return ��λ��ķ�ʽ��ǰ��������״̬��λ����μ� ant_gprs_data_status_e
 */
ant_s32 ant_gprs_get_data_status(void);

/**
 * ���ص�ǰ�Ƿ��Ѳ忨
 *
 * @return 1�� �Ѳ忨
 *         0�� δ�忨
 */
ant_s32 ant_has_sim_card(void);

/**
 * ��ȡ����IMEI��
 *
 * @param imei ����IMEI��
 * @param buffer_size IMEI���泤��
 *
 * @return
 */
ant_s32 ant_get_imei(ant_char *imei, ant_u32 buffer_size);

/**
 * ��ȡ����IMSI��
 *
 * @param imsi ����IMSI��
 * @param buffer_size IMSI���泤��
 * @return = 0: ��ȡ�ɹ�
 *         < 0: ��ȡʧ��
 */
ant_s32 ant_get_imsi(ant_char *imsi, ant_u32 buffer_size);

/**
 * ��ȡ����SIM����ICCID
 *
 * @param iccid ����ICCID
 * @param size ICCID���泤��
 * @return = 0: ��ȡ�ɹ�
 *         < 0: ��ȡʧ��
 */
ant_s32 ant_get_iccid(char *iccid, ant_u32 size);

/**
 * ���÷���ģʽ
 *
 * @param enable 1���������ģ�ͣ� 0���˳�����ģʽ
 */
void    ant_set_flight_mode(int enable);

/**
 * ��ȡ��ǰ����ģʽ״̬
 *
 * @return 1������ģ�ͣ� 0������ģʽ
 */
int     ant_get_flight_mode(void);


ant_s32 ant_get_cpuid(char *cpuid, ant_u32 size);

    
ant_s32 ant_get_sn(ant_char * sn, ant_s32 * size);


ant_network_type_e ant_gprs_get_network_type();

ant_s32 ant_gprs_get_signal_quality(ant_s32 * rsrq,ant_s32 * rsrp,ant_s32 *rsnr);

ant_s32 ant_gprs_get_drx_cycle(void);

ant_s32 ant_gprs_get_rrc_cycle(void);

#ifdef __cplusplus
}
#endif

#endif

