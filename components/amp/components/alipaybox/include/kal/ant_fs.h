/**
 * ant_fs.h
 *
 * �ļ�ϵͳ�ӿ�
 */
#ifndef __ANT_FS_H__
#define __ANT_FS_H__

#include "ant_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ROOTDIR                   "C:\\"
#define CUSTOM_AUDIO_DIR          "C:\\"

//�ļ���󳤶�
#define ANT_MAX_FILE_PATH_LEN     256
#define ANT_FS_TRIGGER_FORMAT     1000

// �ļ��򿪺ʹ�����ʽ
#define ANT_FS_READ_WRITE         0x00000000L
#define ANT_FS_READ_ONLY          0x00000100L
#define ANT_FS_CREATE             0x00010000L
#define ANT_FS_CREATE_ALWAYS      0x00020000L
#define ANT_FS_OPEN_SHARED        0x00000200L

// �ļ��ƶ���ʽ
#define ANT_FS_MOVE_COPY          0x00000001     // Move file|folder by copy
#define ANT_FS_MOVE_KILL          0x00000002     // Delete the moved file|folder after moving

/****************************************************************************
 * �ļ�ϵͳ������
 ***************************************************************************/
#define ANT_FS_PARAM_ERROR                    -2    /* User */
#define ANT_FS_INVALID_FILENAME               -3    /* User */
#define ANT_FS_DRIVE_NOT_FOUND                -4    /* User or Fatal */
#define ANT_FS_TOO_MANY_FILES                 -5    /* User or Normal: use over max file handle number or more than 256 files in sort */
#define ANT_FS_NO_MORE_FILES                  -6    /* Normal */
#define ANT_FS_WRONG_MEDIA                    -7    /* Fatal */
#define ANT_FS_INVALID_FILE_SYSTEM            -8    /* Fatal */
#define ANT_FS_FILE_NOT_FOUND                 -9    /* User or Normal */
#define ANT_FS_INVALID_FILE_HANDLE           -10    /* User or Normal */
#define ANT_FS_UNSUPPORTED_DEVICE            -11    /* User */
#define ANT_FS_UNSUPPORTED_DRIVER_FUNCTION   -12    /* User or Fatal */
#define ANT_FS_CORRUPTED_PARTITION_TABLE     -13    /* fatal */
#define ANT_FS_TOO_MANY_DRIVES               -14    /* not use so far */
#define ANT_FS_INVALID_FILE_POS              -15    /* User */
#define ANT_FS_ACCESS_DENIED                 -16    /* User or Normal */
#define ANT_FS_STRING_BUFFER_TOO_SMALL       -17    /* User */
#define ANT_FS_GENERAL_FAILURE               -18    /* Normal */
#define ANT_FS_PATH_NOT_FOUND                -19    /* User */
#define ANT_FS_FAT_ALLOC_ERROR               -20    /* Fatal: disk crash */
#define ANT_FS_ROOT_DIR_FULL                 -21    /* Normal */
#define ANT_FS_DISK_FULL                     -22    /* Normal */
#define ANT_FS_TIMEOUT                       -23    /* Normal: FS_CloseMSDC with nonblock */
#define ANT_FS_BAD_SECTOR                    -24    /* Normal: NAND flash bad block */
#define ANT_FS_DATA_ERROR                    -25    /* Normal: NAND flash bad block */
#define ANT_FS_MEDIA_CHANGED                 -26    /* Normal */
#define ANT_FS_SECTOR_NOT_FOUND              -27    /* Fatal */
#define ANT_FS_ADDRESS_MARK_NOT_FOUND        -28    /* not use so far */
#define ANT_FS_DRIVE_NOT_READY               -29    /* Normal */
#define ANT_FS_WRITE_PROTECTION              -30    /* Normal: only for MSDC */
#define ANT_FS_DMA_OVERRUN                   -31    /* not use so far */
#define ANT_FS_CRC_ERROR                     -32    /* not use so far */
#define ANT_FS_DEVICE_RESOURCE_ERROR         -33    /* Fatal: Device crash */
#define ANT_FS_INVALID_SECTOR_SIZE           -34    /* Fatal */
#define ANT_FS_OUT_OF_BUFFERS                -35    /* Fatal */
#define ANT_FS_FILE_EXISTS                   -36    /* User or Normal */
#define ANT_FS_LONG_FILE_POS                 -37    /* User : FS_Seek new pos over sizeof int */
#define ANT_FS_FILE_TOO_LARGE                -38    /* User: filesize + pos over sizeof int */
#define ANT_FS_BAD_DIR_ENTRY                 -39    /* Fatal */
#define ANT_FS_ATTR_CONFLICT                 -40    /* User: Can't specify FS_PROTECTION_MODE and FS_NONBLOCK_MOD */
#define ANT_FS_CHECKDISK_RETRY               -41    /* System: don't care */
#define ANT_FS_LACK_OF_PROTECTION_SPACE      -42    /* Fatal: Device crash */
#define ANT_FS_SYSTEM_CRASH                  -43    /* Normal */
#define ANT_FS_FAIL_GET_MEM                  -44    /* Normal */
#define ANT_FS_READ_ONLY_ERROR               -45    /* User or Normal */
#define ANT_FS_DEVICE_BUSY                   -46    /* Normal */
#define ANT_FS_ABORTED_ERROR                 -47    /* Normal */
#define ANT_FS_QUOTA_OVER_DISK_SPACE         -48    /* Normal: Configuration Mistake */
#define ANT_FS_PATH_OVER_LEN_ERROR           -49    /* Normal */
#define ANT_FS_APP_QUOTA_FULL                -50    /* Normal */
#define ANT_FS_VF_MAP_ERROR                  -51    /* User or Normal */
#define ANT_FS_DEVICE_EXPORTED_ERROR         -52    /* User or Normal */
#define ANT_FS_DISK_FRAGMENT                 -53    /* Normal */
#define ANT_FS_DIRCACHE_EXPIRED              -54    /* Normal */
#define ANT_FS_QUOTA_USAGE_WARNING           -55    /* Normal or Fatal: System Drive Free Space Not Enought */
#define ANT_FS_ERR_DIRDATA_LOCKED            -56    /* Normal */
#define ANT_FS_INVALID_OPERATION             -57    /* Normal */
#define ANT_FS_ERR_VF_PARENT_CLOSED          -58    /* Virtual file's parent is closed */
#define ANT_FS_ERR_UNSUPPORTED_SERVICE       -59    /* The specified FS service is closed in this project. */
#define ANT_FS_ERR_INVALID_JOB_ID            -81
#define ANT_FS_ERR_ASYNC_JOB_NOT_FOUND       -82
#define ANT_FS_MSDC_MOUNT_ERROR             -100    /* Normal */
#define ANT_FS_MSDC_READ_SECTOR_ERROR       -101    /* Normal */
#define ANT_FS_MSDC_WRITE_SECTOR_ERROR      -102    /* Normal */
#define ANT_FS_MSDC_DISCARD_SECTOR_ERROR    -103    /* Normal */
#define ANT_FS_MSDC_PRESNET_NOT_READY       -104    /* System */
#define ANT_FS_MSDC_NOT_PRESENT             -105    /* Normal */
#define ANT_FS_EXTERNAL_DEVICE_NOT_PRESENT  -106    /* Normal */
#define ANT_FS_HIGH_LEVEL_FORMAT_ERROR      -107    /* Normal */
#define ANT_FS_CARD_BATCHCOUNT_NOT_PRESENT  -110    /* Normal */
#define ANT_FS_FLASH_MOUNT_ERROR            -120    /* Normal */
#define ANT_FS_FLASH_ERASE_BUSY             -121    /* Normal: only for nonblock mode */
#define ANT_FS_NAND_DEVICE_NOT_SUPPORTED    -122    /* Normal: Configuration Mistake */
#define ANT_FS_FLASH_OTP_UNKNOWERR          -123    /* User or Normal */
#define ANT_FS_FLASH_OTP_OVERSCOPE          -124    /* User or Normal */
#define ANT_FS_FLASH_OTP_WRITEFAIL          -125    /* User or Normal */
#define ANT_FS_FDM_VERSION_MISMATCH         -126    /* System */
#define ANT_FS_FLASH_OTP_LOCK_ALREADY       -127    /* User or Normal */
#define ANT_FS_FDM_FORMAT_ERROR             -128    /* The format of the disk content is not correct */
#define ANT_FS_FDM_USER_DRIVE_BROKEN        -129    /* User drive unrecoverable broken*/
#define ANT_FS_FDM_SYS_DRIVE_BROKEN         -130    /* System drive unrecoverable broken*/
#define ANT_FS_FDM_MULTIPLE_BROKEN          -131    /* multiple unrecoverable broken*/
#define ANT_FS_LOCK_MUTEX_FAIL              -141    /* System: don't care */
#define ANT_FS_NO_NONBLOCKMODE              -142    /* User: try to call nonblock mode other than NOR flash */
#define ANT_FS_NO_PROTECTIONMODE            -143    /* User: try to call protection mode other than NOR flash */
/*
 * If disk size exceeds FS_MAX_DISK_SIZE (unit is KB, defined in makefile),
 * FS_TestMSDC(), FS_GetDevStatus(FS_MOUNT_STATE_ENUM) and all access behaviors will
 * get this error code.
 */
#define ANT_FS_DISK_SIZE_TOO_LARGE          (ANT_FS_MSDC_MOUNT_ERROR),     /*Normal*/
#define ANT_FS_MINIMUM_ERROR_CODE           -65536 /* 0xFFFF0000 */

/****************************************************************************
 * �ļ�ɾ����չ��ʽ
 ***************************************************************************/
#define ANT_FS_FILE_TYPE              0x00000004     
#define ANT_FS_DIR_TYPE               0x00000008     
#define ANT_FS_RECURSIVE_TYPE         0x00000010

/****************************************************************************
 * Constants for File Seek
 ***************************************************************************/
typedef enum
{
   ANT_FS_FILE_BEGIN,    // Beginning of file
   ANT_FS_FILE_CURRENT,  // Current position of file pointer
   ANT_FS_FILE_END       // End of file
} ant_fs_seekpos_e;

typedef enum 
{
    ANT_FS_UFS = 1,       // U�̴洢
    ANT_FS_SD  = 2,       // SD���洢
    ANT_FS_RAM = 3,       // RAM�洢
    ANT_FS_FAT = 0xFF     // norflash FAT�洢
} ant_fs_storage_e;

typedef enum {
    ANT_FS_FMT_DONE = -1,
    ANT_FS_FMT_FAIL,
    ANT_FS_FMT_LOW_FMT,
    ANT_FS_FMT_HIGH_FMT 
} ant_fs_format_action_e;

typedef void (* ant_fs_format_cb)(const char *device_name, ant_fs_format_action_e action, ant_u32 total, ant_u32 completed);


typedef struct
{
    ant_s16 fs_index;  ///< internal fs index
    ant_s16 _reserved; ///< reserved
} ant_fs_dir_t;

/**
 * dirent data structure
 */
typedef struct
{
    ant_s32 d_ino;            ///< inode number, file system implementation can use it for any purpose
    ant_u8 d_type; ///< type of file
    ant_char d_name[256];     ///< file name
}ant_fs_dirent_t;

/**
 * ��ȡ�ļ�ϵͳ��·��
 *
 * @return �ǿ��ַ�������ȡ�ɹ��������ļ�ϵͳ��·��
 *               NULL����ȡʧ��
 */
ant_char* ant_fs_get_root_path();


/**
 * �򿪻򴴽�һ���ļ�
 *
 * @param filename �ļ�·��
 * @param flag �ļ��򿪷�ʽ���ο��ļ��򿪺ʹ�����־
 *
 * @return ���ڵ���0�� �򿪻򴴽��ɹ��������ļ����
 *           ����ֵ�� �򿪻򴴽�ʧ�ܣ�ȡֵ�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_open(char* filename, ant_u32 flag);

ant_s32 ant_ext_fs_open(char *filename, ant_u32 flag);

/**
 * �����ڴ��ļ�
 *
 * @param filename �ļ�����
 * @param flag ������־���ο��ļ��򿪺ʹ�����־
 * @param ramfile_size �ڴ��ļ���С
 * @return
 */
ant_s32 ant_fs_open_ramfile(char *filename, ant_u32 flag, ant_u32 ramfile_size);

/**
 * ���ļ��е�ǰλ�ö�ȡ����
 *
 * @param handle �ļ����
 * @param data ���ݻ���
 * @param len ��ȡ���ݵĳ���
 *
 * @return  ����0�� ��ȡ�ɹ������ض�ȡ�����ݳ���
 *         ����ֵ�� ��ȡʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_read(ant_s32 handle, void *data, ant_u32 len);

/**
 * ���ļ���ǰλ��д������
 *
 * @param handle �ļ����
 * @param data д�����ݻ���
 * @param len д�����ݳ���
 *
 * @return ����0�� д��ɹ�������д������ݳ���
 *        ����ֵ�� д��ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_write(ant_s32 handle, const void *data, ant_u32 len);

/**
 * ��λ�ļ���ǰ����λ��
 *
 * @param handle �ļ����
 * @param offset λ��ƫ��
 * @param whence ��ʼλ�ã��μ�ant_fs_seekpos_e
 *
 * @return     0�� ��λ�ɹ�
 *         ����ֵ�� ��λʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_seek(ant_s32 handle, ant_s32 offset, ant_fs_seekpos_e whence);

/**
 * ��ȡ�ļ���ǰ����λ��
 *
 * @param handle �ļ����
 *
 * @return ���ڵ���0�� �����ļ���ǰ����λ��
 *            ����ֵ�� ��ȡʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_tell(ant_s32 handle);

/**
 * �ض��ļ�����Ϊ0
 *
 * @param handle �ļ����
 *
 * @return     0�� �ɹ�
 *         ����ֵ�� ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_truncate(ant_s32 handle);

/**
 * ���ļ�������������д����̣�flash��
 *
 * @param handle �ļ����
 */
void ant_fs_flush(ant_s32 handle);

/**
 * �ر��ļ�
 *
 * @param handle �ļ����
 */
void ant_fs_close(ant_s32 handle);

/**
 * ��ȡ�ļ�����
 *
 * @param filename �ļ�·����
 *
 * @return ���ڵ���0�� �����ļ��ĳ���
 *            ����ֵ�� ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_getsize(char *filename);

ant_s32 ant_ext_fs_getsize(char *filename);

/**
 * ɾ���ļ�
 *
 * @param filename ��·����
 *
 * @return     0�� �ɹ�
 *         ����ֵ�� ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_delete(char *filename);

ant_s32 ant_ext_fs_delete(char *filename);


/**
 * �����ļ��Ƿ����
 *
 * @param filename ��·����
 *
 * @return 1�� �ļ�����
 *         0�� �ļ�������
 */
ant_s32 ant_fs_exist(char *filename);

/**
 * �ж��ļ���ǰλ���Ƿ���ĩβ
 *
 * @param filename ��·����
 *
 * @return 1�� ��ǰλ����ĩβ
 *         0�� ��ǰλ�ò���ĩβ
 */
ant_bool ant_fs_eof(ant_s32 h);

/**
 * �ļ�������
 *
 * @param filename �������ļ�·��
 * @param filename_new �������ļ�·��
 *
 * @return     0�� �ļ��������ɹ�
 *         ����ֵ�� �ļ�������ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_rename(char *filename, char *filename_new);

ant_s32 ant_ext_fs_rename(char *filename, char *filename_new);


/**
 * ����Ŀ¼
 *
 * @param dirname Ŀ¼����
 *
 * @return     0�� ����Ŀ¼�ɹ�
 *         ����ֵ�� ����ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_create_dir(char *dirname);

/**
 * ɾ��Ŀ¼
 *
 * @param dirname Ŀ¼����
 *
 * @return     0�� ɾ��Ŀ¼�ɹ�
 *         ����ֵ�� ɾ��Ŀ¼ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_delete_dir(char *dirname);

/**
 * ��ʼ����Ŀ¼�е��ļ�
 *
 * @param path Ŀ¼·��
 * @param filename �洢�ļ����Ļ���
 * @param filename_len �洢�ļ����Ļ��泤��
 * @param file_size ���ر��������ļ���С
 * @param is_dir  ���ر������ļ��Ƿ�ΪĿ¼��1����ʾĿ¼��0��ʾ�ļ�
 *
 * @return ���ڵ���0�� ��ʼ�����ɹ������ر������
 *           ����ֵ�� ����ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_find_first(char *path, char *filename, ant_u32 filename_len, ant_u32 *file_size, ant_bool *is_dir);

/**
 * ��������Ŀ¼�е��ļ�
 *
 * @param path Ŀ¼·��
 * @param filename �洢�ļ����Ļ���
 * @param filename_len �洢�ļ����Ļ��泤��
 * @param file_size ���ر��������ļ���С
 * @param is_dir  ���ر������ļ��Ƿ�ΪĿ¼��1����ʾĿ¼��0��ʾ�ļ�
 *
 * @return     0�� �����ɹ�
 *         ����ֵ�� ����ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_find_next(ant_s32 handle, char *filename, ant_u32 filename_len, ant_u32 *file_size, ant_bool *is_dir);

/**
 * ����Ŀ¼����
 *
 * @param handle �������
 */
   void ant_fs_find_close(ant_s32 handle);

/**
 * ɾ���ļ���չ
 *
 * @param path Ŀ¼·��
 * @param flag ɾ����ʽ���μ��ļ�ɾ����ʽ
 *
 * @return     0�� ɾ���ɹ�
 *         ����ֵ�� ɾ��ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_delete_ext(char* path, ant_u32 flag);

/**
 * ��ȡ�û��ļ�ϵͳʣ���С
 *
 * @return �ļ�ϵͳʣ���С����λΪ�ֽ�
 */
ant_s32 ant_fs_get_free_space(void);

/**
 * ��ȡ�û��ļ�ϵͳ�ܴ�С
 *
 * @return �ļ�ϵͳ�ܴ�С����λΪ�ֽ�
 */
ant_s32 ant_fs_get_total_space(void);

/**
 * �ļ�ϵͳ��ʽ��
 *
 * @param cb ��ʽ�����̻ص��������μ� ant_fs_format_cb
 *
 * @return 0����ʽ�����óɹ�
 *         ����ֵ�� ��ʽ������ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_format(ant_fs_format_cb cb);

/**
 * ������ʱ�ļ�
 *
 * @return ��ʱ�ļ����
 */
ant_s32 ant_fs_tmpfile(void);

/**
 * �ر���ʱ�ļ�
 *
 * @param handle
 *
 * @return     0�� �رճɹ�
 *         ����ֵ�� �ر�ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_tmpfile_close(ant_s32 handle);

/**
 * ���ļ��Դ�ӡ��ʽ���
 *
 * @param handle �ļ����
 * @param format ��ӡ��ʽ
 * @param ... ��ӡ����
 *
 * @return     0�� ����ɹ�
 *         ����ֵ�� ���ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_printf(ant_s32 handle, const char *format, ...);

/**
 * ���ļ��Դ�ӡ��ʽ����
 *
 * @param handle �ļ����
 * @param format ��ӡ��ʽ
 * @param ... ��ӡ����
 *
 * @return     0�� ����ɹ�
 *         ����ֵ�� ����ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_scanf(ant_s32 handle, const char *format, ...);

/**
 * ���ļ�д��һ��
 *
 * @param str д����ַ���
 * @param h �ļ����
 *
 * @return 0�� д��ɹ�
 *         ����ֵ�� д��ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_puts(char *str, ant_s32 h);

/**
 * ���ļ��л�ȡһ��
 *
 * @param str д����ַ���
 * @param h �ļ����
 *
 * @return 0�� ��ȡ�ɹ�
 *         ����ֵ�� ��ȡʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
char *  ant_fs_gets(char *buf, ant_s32 len, ant_s32 h);

/**
 * ���ļ�д��һ���ַ�
 * 
 * @param c д����ַ�
 * @param h �ļ����
 * 
 * @return 0�� д��ɹ�
 *         ����ֵ�� д��ʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_putc(char c, ant_s32 h);

/**
 * ���ļ���ȡһ���ַ�
 * 
 * @param c ��ȡ���ַ�
 * @param h �ļ����
 * 
 * @return 0�� ��ȡ�ɹ�
 *         ����ֵ�� ��ȡʧ�ܣ����ش����룬�μ��ļ�ϵͳ������
 */
ant_s32 ant_fs_getc(char c, ant_s32 h);


char *ant_fs_get_real_path(char *filename);


ant_fs_dir_t *     ant_fs_open_dir(char * dirname);

ant_fs_dir_t *     ant_fs_open_dir_ext(char * dirname);

ant_fs_dirent_t *  ant_fs_read_dir(ant_fs_dir_t * p_dir );

ant_s32            ant_fs_close_dir(ant_fs_dir_t * p_dir);

ant_s32            ant_ext_fs_exist(char *filename);

ant_s32            ant_ext_fs_get_free_space(void);

ant_s32            ant_ext_fs_get_total_space(void);




#ifdef __cplusplus
} // extern "C"
#endif

#endif

