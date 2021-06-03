/**
 * ant_flash.h
 *
 * NOR/SPI FLASH�ӿ�
 */
#ifndef __ANT_FLASH_H__
#define __ANT_FLASH_H__
#include "ant_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/**
 * ��spi flash��ַд������
 * 
 * @param addr spiflash��ַ
 * @param data ���ݻ���ָ��
 * @param len ���ݻ��泤��
 * 
 * @return  0�� д��ɹ�
 *         -1�� д��ʧ��
 */
ant_s32 ant_flash_write(ant_u32 addr, ant_u8 *data, ant_u32 len);

/**
 * ��spi flash��ַ��ȡ����
 * 
 * @param addr spiflash��ַ
 * @param data ���ݻ���ָ��
 * @param len ���ݻ��泤��
 * 
 * @return  0�� ��ȡ�ɹ�
 *         -1�� ��ȡʧ��
 */
ant_s32 ant_flash_read (ant_u32 addr, ant_u8 *data, ant_u32 len);

/**
 * ����spi flash��
 * 
 * @param block_index spiflash�����
 * 
 * @return  0�� �����ɹ�
 *         -1�� ����ʧ��
 */
ant_s32 ant_flash_erase_block(ant_u32 block_index);

/**
 * ����spi flash�û�����
 * 
 * @return  0�� �����ɹ�
 *         -1�� ����ʧ��
 */
ant_s32 ant_flash_erase(void);

/**
 * ��ȡspi flash��ַ��Ӧ�Ŀ����
 * 
 * @param addr spiflash��ַ
 * 
 * @return  ��Ӧ @addr ���ڵ�spiflash�����
 */
ant_u32 ant_flash_get_block_index(ant_u32 addr);

/**
 * ��ȡspi flash�û��������ʼ��ַ
 * 
 * @return  0�� �����ɹ�
 *         -1�� ����ʧ��
 */
ant_u32 ant_flash_get_start_addr(void);

ant_u32 ant_flash_get_total_size(void);

   
#ifdef __cplusplus
} // extern "C"
#endif

#endif

