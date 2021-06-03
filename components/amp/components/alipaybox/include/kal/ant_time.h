/**
 * ant_time.h
 *
 * ʱ�ӽӿ�
 */
#ifndef __ANT_TIME_H__
#define __ANT_TIME_H__

#include "ant_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ant_s32 year;      // [0~127]   
    ant_s32 month;     // [1~12]
    ant_s32 wday;
    ant_s32 day;
    ant_s32 hour;
    ant_s32 minute;
    ant_s32 second;
    ant_s32 isdst;  // one digit expresses a quarter of an hour, for example: 22 indicates "+5:30"
} ant_time;

/**
 * ��ȡϵͳ��ǰ����
 *
 * @param date_time ����ϵͳ��ǰ����
 * @return  0�� ��ȡ�ɹ�
 *         -1�� ��ȡʧ��
 */
ant_s32 ant_get_localtime(ant_time* date_time);

/**
 * ���õ�ǰUTCʱ��
 *
 * @param utc_seconds ����1970.1.1������
 */
void    ant_set_utc_seconds(ant_s64 utc_seconds);

/**
 * ��ȡ��ǰUTCʱ��
 *
 * @return ����1970.1.1������
 */
ant_s64 ant_get_utc_seconds(void);

/**
 * ��ȡϵͳ����������ʱ��
 *
 * @return ϵͳ����������ʱ�䣬��λΪ����
 */
ant_u32 ant_get_system_minsecs(void);

ant_s32  ant_striptime(ant_char * buf, ant_char * format, ant_time  * time_out);

#ifdef __cplusplus
}
#endif

#endif

