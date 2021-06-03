/**
 * ant_uart.h
 *
 * ���ڶ�д�Ϳ��ƽӿ�
 */
#ifndef __ANT_UART_H__
#define __ANT_UART_H__

#include "ant_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UART_PORT1 = 0,
    UART_PORT2,
    UART_PORT3,
    UART_PORT4
} uart_port_enum;

typedef enum {
    DB_5BIT = 5,
    DB_6BIT,
    DB_7BIT,
    DB_8BIT
} uart_data_bits_enum;

typedef enum {
    SB_ONE = 1,
    SB_TWO,
    SB_ONE_HALF
} uart_stop_bits_enum;

typedef enum {
    PB_NONE = 0,
    PB_ODD,
    PB_EVEN,
    PB_SPACE,
    PB_MARK
} uart_parity_bits_enum;

typedef enum {
    UART_FC_NONE = 1,
    UART_FC_HW,
    UART_FC_SW
} uart_flow_ctrl;

typedef enum {
    UART_EVENT_READY_TO_READ  = 0,
    UART_EVENT_READY_TO_WRITE,
    UART_EVENT_FE_IND,
    UART_EVENT_RI_IND,
    UART_EVENT_DCD_IND,
    UART_EVENT_DTR_IND
} uart_event_type_enum;

typedef struct {
    ant_u32               baudrate;
    uart_data_bits_enum   data_bits;
    uart_stop_bits_enum   stop_bits;
    uart_parity_bits_enum parity_bits;
    uart_flow_ctrl        flow_ctrl;
} uart_ctrl_param_t;

typedef void (* uart_rx_callback)(uart_port_enum port);
/**
 * ע�ᴮ�ڽ������ݵĻص�����
 *
 * @param port ���ڶ˿ڣ�ȡֵ�μ�uart_port_enum
 * @param cb �������ݵĻص�����
 * @return  0: ע��ɹ�
 *         -1: ע��ʧ��
 */
ant_s32 ant_uart_register_rx_callback(uart_port_enum port, uart_rx_callback cb);

/**
 * ����ģ���ʼ������ϵͳ��ʼ��ʱ���ã��������Ҫ��ʼ�����ӿڷ���0
 * @return  0: ��ʼ���ɹ�
 *         -1: ��ʼ��ʧ��
 */
ant_s32 ant_uart_init(void);

/**
 * �򿪲���ռ���ڡ���д������Ҫִ�д˺�����
 * @param port ���ڶ˿ڣ�ȡֵ�μ� uart_port_enum
 * @return  0: �򿪳ɹ�
 *         -1: ��ʧ��
 */
ant_s32 ant_uart_open(uart_port_enum port);

/**
 * �رղ��ͷŴ��ڡ�
 * @param port ���ڶ˿ڣ�ȡֵ�μ� uart_port_enum
 * @return  0: �رճɹ�
 *         -1: �ر�ʧ��
 */
ant_s32 ant_uart_close(uart_port_enum port);

/**
 * �򴮿�д����
 *
 * @param port ���ڶ˿ڣ�ȡֵ�μ� uart_port_enum
 * @param data д�����ݵĻ���ָ��
 * @param len д�����ݵĳ���
 * @return ���ڵ���0: д��ɹ�������д�����ݵĳ���
 *            С��0: д��ʧ�ܣ�ȡֵ�μ�ANT_ERROR_XXX�궨��
 */
ant_s32 ant_uart_write(uart_port_enum port, ant_u8 *data, ant_u32 len);

/**
 * �Ӵ��ڶ�����
 *
 * @param port ���ڶ˿ڣ�ȡֵ�μ� uart_port_enum
 * @param data ��ȡ���ݵĻ���ָ��
 * @param len ��ȡ���ݵĻ��泤��
 * @return ���ڵ���0: ��ȡ�ɹ������ض�ȡ���ݵĳ���
 *            С��0: ��ȡʧ�ܣ�ȡֵ�μ�ANT_ERROR_XXX�궨��
 */
ant_s32 ant_uart_read(uart_port_enum port, ant_u8 *data, ant_u32 len);

/**
 * ���ô��ڲ���
 *
 * @param port  ���ڶ˿ڣ�ȡֵ�μ� uart_port_enum
 * @param param ���ڲ������μ�uart_ctrl_param_t���Ͷ���
 * @return    0: ���óɹ�
 *         С��0: ����ʧ�ܣ�ȡֵ�μ�ANT_ERROR_XXX�궨��
 */
ant_s32 ant_uart_set_ctrl_param(uart_port_enum port, uart_ctrl_param_t *param);

/**
 * ������ڽ��ջ���
 *
 * @param port ���ڶ˿ڣ�ȡֵ�μ� uart_port_enum
 * @return    0: ����ɹ�
 *         С��0: ���ʧ�ܣ�ȡֵ�μ�ANT_ERROR_XXX�궨��
 */
ant_s32 ant_uart_clear_rx_buffer(uart_port_enum port);

/**
 * ������ڷ��ͻ���
 *
 * @param port ���ڶ˿ڣ�ȡֵ�μ� uart_port_enum
 * @return    0: ����ɹ�
 *         С��0: ���ʧ�ܣ�ȡֵ�μ�ANT_ERROR_XXX�궨��
 */
ant_s32 ant_uart_clear_tx_buffer(uart_port_enum port);

/**
 * ��ȡ���ڷ��ͻ���ʣ���С
 *
 * @param port ���ڶ˿ڣ�ȡֵ�μ� uart_port_enum
 * @return ���ڵ���0: ��ȡ�ɹ������ش��ڷ��ͻ���ʣ���С
 *            С��0: ��ȡʧ�ܣ�ȡֵ�μ�ANT_ERROR_XXX�궨��
 */
ant_u16 ant_uart_get_tx_roomleft(uart_port_enum port);

/**
 * ��ȡ���ڿɽ���ʣ�����ݳ���
 *
 * @param port ���ڶ˿ڣ�ȡֵ�μ� uart_port_enum
 * @return ���ڵ���0: ��ȡ�ɹ������ش��ڿɽ���ʣ�����ݳ���
 *            С��0: ��ȡʧ�ܣ�ȡֵ�μ�ANT_ERROR_XXX�궨��
 */
ant_s32 ant_uart_get_rx_avail_bytes(uart_port_enum port);

/**
 * �Ӵ��ڻ�ȡһ���ֽ�
 *
 * @param port ���ڶ˿ڣ�ȡֵ�μ� uart_port_enum
 * @return ���ػ�ȡ���ֽ�
 */
ant_u8  ant_uart_get_byte(uart_port_enum port);

#ifdef __cplusplus
}
#endif

#endif

