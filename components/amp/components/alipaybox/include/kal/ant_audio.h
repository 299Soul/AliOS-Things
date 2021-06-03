/**
 * ant_audio.h
 *
 * ��Ƶ���Žӿ�
 */
#ifndef __ANT_AUDIO_H__
#define __ANT_AUDIO_H__
#include "ant_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ANT_AUDIO_MIN_VOLUME 1
#define ANT_AUDIO_MAX_VOLUME 6

typedef enum {
    ANT_AUDIO_FORMAT_UNKNOWN = -1,
    ANT_AUDIO_FORMAT_WAV = 0,
    ANT_AUDIO_FORMAT_MP3 = 1,
    ANT_AUDIO_FORMAT_BUFFER = 2
} ant_audio_format_e;

typedef enum {
    ANT_AUDIO_SUCCESS = 0,          /* 0 */
    ANT_AUDIO_FAIL,                 /* 1 */
    ANT_AUDIO_BUSY,                 /* 2 */
    ANT_AUDIO_DISC_FULL,            /* 3 */
    ANT_AUDIO_OPEN_FILE_FAIL,       /* 4 */
    ANT_AUDIO_END_OF_FILE,          /* 5 */
    ANT_AUDIO_TERMINATED,           /* 6 */
    ANT_AUDIO_BAD_FORMAT,           /* 7 */
    ANT_AUDIO_INVALID_FORMAT,       /* 8 */
    ANT_AUDIO_ERROR,                /* 9 */
    ANT_AUDIO_NO_DISC,              /* 10 */
    ANT_AUDIO_NO_SPACE,             /* 11 */
    ANT_AUDIO_INVALID_HANDLE,       /* 12 */
    ANT_AUDIO_NO_HANDLE,            /* 13 */
    ANT_AUDIO_RESUME,               /* 14 */
    ANT_AUDIO_BLOCKED,              /* 15 */
    ANT_AUDIO_MEM_INSUFFICIENT,     /* 16 */
    ANT_AUDIO_BUFFER_INSUFFICIENT,  /* 17 */
    ANT_AUDIO_FILE_EXIST,           /* 18 */
    ANT_AUDIO_WRITE_PROTECTION,     /* 19 */
    ANT_AUDIO_PARAM_ERROR,          /* 20 */
    ANT_AUDIO_UNSUPPORTED_CHANNEL,  /* 21 */
    ANT_AUDIO_UNSUPPORTED_FREQ,     /* 22 */
    ANT_AUDIO_UNSUPPORTED_TYPE,     /* 23 */
    ANT_AUDIO_UNSUPPORTED_OPERATION,/* 24 */
    ANT_AUDIO_PARSER_ERROR          /* 25 */
} ant_audio_play_result_e;

typedef void (* ant_audio_play_finish_callback_t)(ant_u16 play_id);

void ant_audio_init(void);

/**
 * ע�Ქ����ɻص�����
 *
 * @param cb �������ʱ�ص�cb�������ant_audio_play_file��ant_audio_play_buffer�����play_id
 * @return
 */
ant_s32 ant_register_play_callback(ant_audio_play_finish_callback_t cb);

/**
 * ������Ƶ�ļ���MP3, WAV��ʽ
 *
 * @param filename ��Ƶ�ļ�·��
 * @param play_id  ��Ƶ��������id
 *
 * @return 0 ���ųɹ�
 *         ��0ֵ�� ����ʧ�ܣ�ȡֵ�ο�ant_audio_play_result_e
 */
ant_s32 ant_audio_play_file(char *filename, ant_u16 play_id);

/**
 * ���Ż������ݣ�MP3, WAV��ʽ
 *
 * @param data ��Ƶ���ݻ���
 * @param len ��Ƶ�������ݳ���
 * @param format ��Ƶ��ʽ��ȡֵ�μ�ant_audio_format_e
 * @param play_id ��Ƶ��������id
 * @return 0 ���ųɹ�
 *         ��0ֵ�� ����ʧ�ܣ�ȡֵ�ο�ant_audio_play_result_e
 */
ant_s32 ant_audio_play_buffer(ant_u8 *data, ant_u32 len, int format, ant_u16 play_id);

/**
 * ���ò�������
 *
 * @param volume ������С 0~7
 *
 * @return 0 ���ųɹ�
 *         ��0ֵ�� ����ʧ�ܣ�ȡֵ�ο�ant_audio_play_result_e
 */
ant_s32 ant_audio_set_volume(ant_u8 volume);

/**
 * ��ȡ��������
 *
 * @return ������С����СֵANT_AUDIO_MIN_VOLUME�����ֵANT_AUDIO_MAX_VOLUME
 */
ant_s32 ant_audio_get_volume(void);

/**
 * ���þ���
 *
 * @return   0 �ɹ�
 *          -1 ʧ��
 */
ant_s32 ant_audio_mute(void);

/**
 * ȡ������
 *
 * @return   0 �ɹ�
 *          -1 ʧ��
 */
ant_s32 ant_audio_unmute(void);

/**
 * ��ȡ�ļ�ϵͳ�е���Ƶ�ļ���ʽ������
 *
 * @param filename ��Ƶ�ļ�·��
 * @param aud_len ������Ƶ���ݳ���
 * @param format ������Ƶ���ݸ�ʽ���μ�ant_audio_format_e
 * @return ��Ƶ����flash�����ַ
 */
ant_u8 *ant_audio_get_data(const char *filename, ant_u32 *aud_len, ant_s32 *format);


#ifdef __cplusplus
}
#endif
#endif
