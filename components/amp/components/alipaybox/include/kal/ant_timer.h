/**
 * ant_timer.h
 *
 * ��ʱ���ӿ�
 */
#ifndef __ANT_TIMER_H__
#define __ANT_TIMER_H__

#include "ant_typedef.h"
#include "ant_os.h"

#ifdef __cplusplus
extern "C" {
#endif
    
#define TIME_SYNC_TIMER       0
#define LED_BLINK_TIMER       1
#define TIMER_ID_MAX 5

typedef void (*callback_on_timer)(void *param);
typedef void * ant_timer;

/**
 * ��ʱ��ģ���ʼ����ϵͳ��ʼ��ʱ��Ҫ���õĽӿ�
 *
 * @return  0: �ɹ�
 *         -1: ʧ��
 */
ant_s32   ant_timer_init(void);

/**
 * ������ʱ��
 *
 * @return �ɹ�: ���ض�ʱ������
 *         ʧ��: ����NULL
 */
ant_timer ant_timer_create(void);

/**
 * ������ʱ��
 *
 * @param timer ��ʱ������
 * @param callback ʱ�䵽��ʱ�Ļص�����
 * @param param �ص���������
 * @param interval ��ʱʱ��
 * @param resched_interval ��ʱʱ�䷢��ʱ���Զ����¿�����ʱ��ʱ�䣬�������0����ʱ���Զ�ֹͣ
 */
void      ant_timer_start(ant_timer timer, callback_on_timer callback, void *param, ant_u32 interval, ant_u32 resched_interval);

/**
 * ֹͣ��ʱ������
 *
 * @param timer ��ʱ������
 */
void      ant_timer_stop(ant_timer timer);

/**
 * ��ȡ��ǰ��ʱ��ʣ��ʱ��
 *
 * @param timer ��ʱ������
 * @return ʣ��ʱ�䣬��λΪ����
 */
ant_u32   ant_timer_get_remaining_time(ant_timer timer);

/**
 * ���ٶ�ʱ��
 *
 * @param timer ��ʱ������
 */
void      ant_timer_destroy(ant_timer timer);

#ifdef __cplusplus
}
#endif

#endif

