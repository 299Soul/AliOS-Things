/**
 * ant_gpio.h
 *
 * GPIO���ʽӿ�
 */
#ifndef __ANT_GPIO_H__
#define __ANT_GPIO_H__

#include "ant_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif
    
typedef enum {
    GPIO_LED_RED = 0,
    GPIO_LED_GREEN, 
    GPIO_LED_BLUE,
    GPIO_KEY_POWER,
    GPIO_KEY_VOLDN,
    GPIO_KEY_VOLUP,
    GPIO_KEY_FUNC,
    GPIO_PA_CTRL,
    GPIO_POWER_CTRL,
} ant_gpio_name_e;

typedef enum {
    GPIO_IN  = 0,
    GPIO_OUT = 1,   
} ant_gpio_dir_e;

typedef enum {
    GPIO_LOW  = 0,
    GPIO_HIGH = 1,  
} ant_gpio_level_e;

typedef enum {
    GPIO_PULL_DISABLE = 0,
    GPIO_PULL_DOWN    = 1,  
    GPIO_PULL_UP      = 2,
} ant_gpio_pullsel_e;

/**
 * GPIO ģ���ʼ��
 * 
 * @return  0�� ��ʼ���ɹ�
 *         -1: ��ʼ��ʧ��
 */
ant_s32 ant_gpio_init(void);

/**
 * ����GPIO
 * 
 * @param gpio_name GPIO���� 
 * @param level GPIO�ߵ͵�ƽ
 * 
 * @return 0�� ���óɹ�
 *        -1�� ����ʧ��
 */
ant_s32 ant_gpio_set(ant_gpio_name_e gpio_name, ant_gpio_level_e level);

/**
 * ��ȡGPIO��ƽ״̬
 * 
 * @param gpio_name GPIO���� 
 * @param level GPIO�ߵ͵�ƽ
 * 
 * @return ���ڵ���0�� ��ȡ�ɹ�
 *              -1�� ��ȡʧ��
 */
ant_s32 ant_gpio_get(ant_gpio_name_e gpio_name);

/**
 * ����GPIO�����������
 * 
 * @param gpio_name GPIO���� 
 * @param dir �����������
 * 
 * @return ���ڵ���0�� ���óɹ�
 *              -1�� ����ʧ��
 */
ant_s32 ant_gpio_set_dir(ant_gpio_name_e gpio_name, ant_gpio_dir_e dir);

    
#ifdef __cplusplus
}
#endif

#endif

