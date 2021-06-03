/**
 * ant_os.h
 *
 * ����ϵͳ��ؽӿ�
 */
#ifndef __ANT_OS_H__
#define __ANT_OS_H__

#include "ant_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TASK_ID_INVALID     -1
#define TASK_APP_PRIO_BASE  24

typedef enum {
    ANT_NO_WAIT = 0,
    ANT_INFINITE_WAIT = -1
} ant_wait_mode_e;

typedef enum {
    ANT_QUE_SEM_TIMEOUT   = 0x00,       /* ��Ϣ���л��ź�����ʱ���� */
    ANT_QUE_SEM_FAILURE   = 0x01,       /* ��Ϣ���л��ź������󷵻� */
    ANT_QUE_SEM_SUCCESS   = 0x02        /* ��Ϣ���л��ź����������� */
} ant_que_sem_status_e;

typedef void* ant_mutex;
typedef void* ant_sem;
typedef void* ant_cond;
typedef void* ant_queue;
typedef ant_u32 ant_task_t;
typedef void (* ant_task_function_t)(void *);

/**
 * �߳�����
 *
 * @param ms ����ʱ�䣬��λΪ����
 */
void ant_task_sleep_ms(ant_u32 ms);

/**
 * ����������
 *
 * @return  �ǿգ������ɹ������ػ�����
 *         NULL: ����ʧ��
 */
ant_mutex ant_mutex_create(void);

/**
 * ����������
 *
 * @param mutex ������
 */
void ant_mutex_lock(ant_mutex mutex);

/**
 * ����������
 *
 * @param mutex ������
 */
void ant_mutex_unlock(ant_mutex mutex);

/**
 * �ͷŻ�����
 *
 * @param mutex
 */
void ant_mutex_free(ant_mutex mutex);

/**
 * �����ź���
 *
 * @return  �ǿգ������ɹ��������ź���
 *         NULL: ����ʧ��
 */
ant_sem ant_sem_create(ant_u32 initial_count);

/**
 * �ȴ��ź���
 *
 * @param sem �ź���
 * @param time_out ��ʱʱ�䣬��λΪ����
 * @return ANT_QUE_SEM_SUCCESS: �ȴ��ɹ�����ȡ���ź���
 *         ANT_QUE_SEM_TIMEOUT�� �ȴ���ʱ��û�л�ȡ���ź���
 *         ANT_QUE_SEM_FAILURE�� �ȴ�ʧ�ܣ�û�л�ȡ���ź���������û�еȵ���ʱʱ��
 */
ant_s32 ant_sem_wait(ant_sem sem, ant_s32 time_out);

/**
 * �����ź���
 *
 * @param sem �ź���
 */
void ant_sem_signal(ant_sem sem);

/**
 * �ͷ��ź�������semָ����ź�����������
 *
 * @param sem Ҫ�ͷŵ��ź�������
 */
void ant_sem_free(ant_sem sem);

/**
 * ����������
 *
 * @return  �ǿգ������ɹ�������������
 *         NULL: ����ʧ��
 */
ant_cond ant_cond_create(void);

/**
 * ����������
 *
 * @param cond ��������
 * @param events ����������ʶ��bit[31:0]ÿ1λΪ1ʱ��ʾһ����������
 */
void ant_cond_signal(ant_cond cond, ant_u32 events);

/**
 * �ȴ�����
 *
 * @param cond ������
 * @param events Ҫ�ȴ���������ʶ��bit[31:0]λΪ1ʱ��ʾ��Ҫ�ȴ�������
 * @param events_got �ȵ���������ʶ��bit[31:0]λΪ1ʱ��ʾ��λָʾ����������
 * @param timeout ��ʱʱ�䣬��λΪ����
 * @return ANT_QUE_SEM_SUCCESS: �ȴ��ɹ������ٵȵ�һ����������
 *         ANT_QUE_SEM_TIMEOUT�� �ȴ���ʱ��û�еȵ��κ���������
 *         ANT_QUE_SEM_FAILURE�� �ȴ�ʧ�ܣ�û�еȵ���������������û�еȵ���ʱʱ��
 */
ant_s32 ant_cond_wait(ant_cond cond, ant_u32 events, ant_u32 *events_got, ant_u32 timeout);

/**
 * ����������
 *
 *  @param cond ������
 */
void ant_cond_destroy(ant_cond cond);

/**
 * ��������
 *
 * @param queue_size ���д�С
 * @return �ǿգ������ɹ������ض���
 *         NULL: ����ʧ��
 */
ant_queue ant_queue_create(int queue_size);

/**
 * ���ٶ���
 *
 * @param queue ���ж���
 */
void ant_queue_destroy(ant_queue queue);

void ant_task_init(void);
/**
 * ��������(�̣߳���ͬʱ������������Ķ���
 *
 * @param entry_function ����ִ�к������
 * @param task_name ��������
 * @param stack_size �����߳�ջ��С
 * @param queue_length ������д�С
 * @param param ����ִ�к����Ĳ���
 * @param priority �������ȼ�
 * @return �ǿգ� �ɹ������񴴽��ɹ�������ִ�У������������̣߳�����
 *         NULL: ʧ��
 */
ant_task_t ant_task_create(ant_task_function_t entry_function,
                           const char * const task_name,
                           ant_u32 stack_size,
                           ant_u32 queue_length,
                           void *param,
                           int priority);

/**
 * ��������
 *
 * @param task �������
 * @return  0�� �ɹ�
 *         -1�� ʧ��
 */
ant_s32 ant_task_destroy(ant_task_t task);

/**
 * ���ص�ǰ�̵߳��������
 *
 * @return  �ǿգ� �ɹ������ص�ǰ�������
 *         NULL: ʧ��
 */
ant_task_t ant_task_self(void);

/**
 * ������ִ��ǰϵͳ��Ҫִ�еĴ��룬�������Ҫִ�У�����ʲôҲ����������0
 */
void ant_task_start(void);

/**
 * ��ȡ��������
 *
 * @param task ����������Ϊ�գ����ص�ǰ�������ƣ�����ǿգ�����taskָ����������ơ�
 * @return
 */
const char * ant_task_name(ant_task_t task);

/**
 * ��ȡ����ջ��С
 *
 * @param task ����������Ϊ�գ����ص�ǰ����ջ��С������ǿգ�����taskָ�������ջ��С��
 * @return ����ջ��С
 */
ant_u32 ant_task_get_stack_size(ant_task_t task);

/**
 * ��ȡ������д�С
 *
 * @param task ����������Ϊ�գ����ص�ǰ������ж�������ǿգ�����taskָ���������ж���
 * @return ����������ж���
 */
ant_queue ant_task_get_event_queue(ant_task_t task);

/**
 * ��ȡϵͳ��������ȼ���ֵ
 *
 * @return ������ȼ���ֵ
 */
ant_s32 ant_task_get_max_priority();

#ifdef __cplusplus
}
#endif

#endif

