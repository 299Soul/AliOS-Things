/**
 * ant_mem.h
 *
 * �ڴ����ӿ�
 */
#ifndef __ANT_MEM_H__
#define __ANT_MEM_H__

#include "ant_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * �ڴ�ģ���ʼ��
 * @return  0�� ��ʼ���ɹ�
 *         -1�� ��ʼ��ʧ��
 */
ant_s32 ant_malloc_init(void);

/**
 * �����ڴ�
 *
 * @param size �ڴ��С
 *
 * @return   ��0: �ɹ������ط�����ڴ�ָ��
 *         NULL: ʧ�ܣ����ؿ�ָ��
 */
void *  ant_malloc(ant_u32 size);

/**
 * �����ڴ�鲢��0
 *
 * @param n �ڴ�����
 * @param size �ڴ���С
 * @return   ��0: �ɹ������ط�����ڴ�ָ��
 *         NULL: ʧ�ܣ����ؿ�ָ��
 */
void *  ant_calloc(ant_u32 n, ant_u32 size);

/**
 * ���·����ڴ�
 *
 * @param p �ѷ����ڴ�ָ��
 * @param new_size ���ڴ��С
 *
 * @return   ��0: �ɹ������ط�����ڴ�ָ��
 *         NULL: ʧ�ܣ����ؿ�ָ��
 */
void *  ant_realloc(void *p, ant_u32 new_size);

/**
 * �ͷ��ڴ�
 *
 * @param p ��Ҫ�ͷŵ��ڴ�ָ��
 */
void    ant_free(void *p);

/**
 * ��������ڴ�
 *
 * @param size ������ڴ�Ĵ�С
 * @param align �����ֽ�����������4�ı���
 * @return   ��0: �ɹ������ط�����ڴ�ָ��
 *         NULL: ʧ�ܣ����ؿ�ָ��
 */
void *  ant_malloc_align(ant_u32 size, ant_u32 align);

/**
 * 获取内存状态
 *
 * @param total 返回总内存大小
 * @param avail 返回可用内存大小
 * @param max_block_size 返回可分配最大内存块大小
 *
 * @return 0 : 获取成功
 *        -1 : 获取失败
 */
ant_s32 ant_mem_get_stats(ant_u32 *total, ant_u32 *avail, ant_u32 *max_block_size);

/**
 * 返回内存是否存在泄漏
 *
 *
 * @return ANT_TRUE  : 存在内存泄漏
 *         ANT_FALSE : 不存在内存泄漏
 */
ant_bool ant_mem_is_leak(void);


/**
 * ��ȡϵͳʣ���ڴ�
 *
 * @return ����ϵͳʣ���ڴ�
 */
ant_u32 ant_get_remain_memory_size(void);

ant_u32 ant_get_total_memory_size(void);

#define ALLOC_TYPE(p, type) type *p = (type *)ant_malloc(sizeof(type)); memset(p, 0, sizeof(type))
#define ALLOC_TYPE_ARRAY(p, type, n) type *p = (type *)ant_calloc(sizeof(type), (n))

#ifdef __cplusplus
}
#endif

#endif

