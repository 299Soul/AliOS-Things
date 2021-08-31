/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>

#include "aos/kernel.h"
#include "fatfs_diskio.h"
#include "ff.h"

#include "vfs_types.h"
#include "vfs_api.h"

#if FF_USE_LFN == 0
#define MAX_NAME_LEN 12
#else
#define MAX_NAME_LEN FF_MAX_LFN
#endif

typedef struct _fsid_map_t {
    int         index; /* Device index */
    const char *root;  /* The mount point of the physical device */
    const char *id;    /* The partition id of the device in fatfs */
} fsid_map_t;

typedef struct _fat_dir_t {
    vfs_dir_t    dir;
    FF_DIR       ffdir;
    FILINFO      filinfo;
    vfs_dirent_t cur_dirent;
} fat_dir_t;

static fsid_map_t g_fsid[] = {
    { DEV_MMC, MMC_MOUNTPOINT, MMC_PARTITION_ID },
    { DEV_USB, USB_MOUNTPOINT, USB_PARTITION_ID },
    { DEV_RAM, RAM_MOUNTPOINT, RAM_PARTITION_ID }
};

static FATFS *g_fatfs[FF_VOLUMES] = { 0 };

static int32_t get_disk_index(int32_t pdrv)
{
    int32_t index;
    for (index = 0; index < FF_VOLUMES; index++) {
        if (g_fsid[index].index == pdrv) {
            return index;
        }
    }

    return -1;
}

static char *translate_relative_path(const char *path)
{
    int32_t len, prefix_len;
    char *relpath, *p;
    BYTE  pdrv;

    if (!path) {
        return NULL;
    }

    len = strlen(path);
    for (pdrv = 0; pdrv < FF_VOLUMES; pdrv++) {
        prefix_len = strlen(g_fsid[pdrv].root);
        if (strncmp(g_fsid[pdrv].root, path, prefix_len) == 0) {
            break;
        }
    }

    if (pdrv == FF_VOLUMES) {
        return NULL;
    }

    relpath = (char *)aos_malloc(len + 1);
    if (!relpath) {
        return NULL;
    }

    memset(relpath, 0, len + 1);
    memcpy(relpath, g_fsid[pdrv].id, strlen(g_fsid[pdrv].id));

    if (len > prefix_len) {
        p = (char *)(path + strlen(g_fsid[pdrv].root) + 1);
        memcpy(relpath + strlen(g_fsid[pdrv].id), p, len - prefix_len - 1);
    }
    relpath[len] = '\0';

    return relpath;
}

static int32_t fatfs_mode_conv(int32_t m)
{
    int32_t res      = 0;
    int32_t acc_mode = m & O_ACCMODE;

    if (acc_mode == O_RDONLY) {
        res |= FA_READ;
    } else if (acc_mode == O_WRONLY) {
        res |= FA_WRITE;
    } else if (acc_mode == O_RDWR) {
        res |= FA_READ | FA_WRITE;
    }

    if (m & O_CREAT) {
        res |= FA_OPEN_ALWAYS;
    }

    if (m & O_TRUNC) {
        res |= FA_CREATE_ALWAYS;
    }

    if (m & O_EXCL) {
        res |= FA_CREATE_NEW;
    }

    if (m & O_APPEND) {
        res |= FA_OPEN_APPEND;
    }

    if (res == 0) {
        res |= FA_OPEN_EXISTING;
    }
    return res;
}


static int32_t fatfs_vfs_open(vfs_file_t *fp, const char *path, int32_t flags)
{
    int32_t ret = -EPERM;

    FIL  *f       = NULL;
    char *relpath = NULL;

    f = (FIL *)aos_malloc(sizeof(FIL));
    if (!f) {
        return -ENOMEM;
    }

    relpath = translate_relative_path(path);
    if (!relpath) {
        aos_free(f);
        return -EINVAL;
    }

    ret = f_open(f, relpath, fatfs_mode_conv(flags));
    if (ret == FR_OK) {
        fp->f_arg = (void *)f;
        aos_free(relpath);
        return ret;
    }

    aos_free(relpath);
    aos_free(f);
    return ret;
}

static int32_t fatfs_vfs_close(vfs_file_t *fp)
{
    int32_t ret = -EPERM;

    FIL *f = (FIL *)(fp->f_arg);

    if (f) {
        ret = f_close(f);
        if (ret == FR_OK) {
            aos_free(f);
            fp->f_arg = NULL;
        }
    }

    return ret;
}

static int32_t fatfs_vfs_read(vfs_file_t *fp, char *buf, uint32_t len)
{
    int32_t nbytes;
    int32_t ret = -EPERM;

    FIL *f = (FIL *)(fp->f_arg);

    if (f) {
        if ((ret = f_read(f, (void *)buf, (UINT)len, (UINT *)&nbytes)) == FR_OK) {
            return nbytes;
        }
    }

    return ret;
}

static int32_t fatfs_vfs_write(vfs_file_t *fp, const char *buf, uint32_t len)
{
    int32_t nbytes;
    int32_t ret = -EPERM;

    FIL *f = (FIL *)(fp->f_arg);

    if (f) {
        if ((ret = f_write(f, (void *)buf, (UINT)len, (UINT *)&nbytes)) == FR_OK) {
            return nbytes;
        }
    }

    return ret;
}

static int32_t fatfs_vfs_access(vfs_file_t *fp, const char *path, int32_t amode)
{
    char   *relpath = NULL;
    FILINFO info;
    int32_t len, ret;

#if FF_FS_READONLY
    if (amode == W_OK) {
        return -EACCES;
    }
#endif

    relpath = translate_relative_path(path);
    if (!relpath) {
        return -EINVAL;
    }

    len = strlen(relpath);
    if (relpath[len - 1] == ':' && relpath[len] == '\0') {
        ret = FR_OK;
    } else {
        ret = f_stat(relpath, &info);
    }

    aos_free(relpath);
    return ret;
}

static uint32_t fatfs_vfs_lseek(vfs_file_t *fp, int64_t off, int32_t whence)
{
    int64_t cur_pos, new_pos, size;

    int32_t ret = -EPERM;

    FIL *f = (FIL *)(fp->f_arg);

    new_pos = 0;

    if (f) {
        if (whence == SEEK_SET) {
            new_pos = off;
        } else if (whence == SEEK_CUR) {
            cur_pos = f_tell(f);
            new_pos = cur_pos + off;
        } else if (whence == SEEK_END) {
            size    = f_size(f);
            new_pos = size + off;
        } else {
            return -EINVAL;
        }

        if ((ret = f_lseek(f, new_pos)) != FR_OK) {
            return ret;
        }
    }

    return new_pos;
}

static int32_t fatfs_vfs_sync(vfs_file_t *fp)
{
    int32_t ret = -EPERM;

    FIL *f = (FIL *)(fp->f_arg);

    if (f) {
        ret = f_sync(f);
    }

    return ret;
}

static int32_t fatfs_vfs_stat(vfs_file_t *fp, const char *path, vfs_stat_t *st)
{
    char   *relpath = NULL;
    FILINFO info;
    int32_t len, ret;

    relpath = translate_relative_path(path);
    if (!relpath) {
        return -EINVAL;
    }

    len = strlen(relpath);
    if (relpath[len - 1] == ':' && relpath[len] == '\0') {
        st->st_size = 0;
        st->st_mode = S_IRWXU | S_IRWXG | S_IRWXO | S_IFDIR;
        ret         = 0;
    } else {

        if ((ret = f_stat(relpath, &info)) == FR_OK) {
            st->st_size = info.fsize;
            st->st_mode = S_IRWXU | S_IRWXG | S_IRWXO |
                          ((info.fattrib & AM_DIR) ? S_IFDIR : S_IFREG);
        }
    }

    aos_free(relpath);
    return ret;
}

static int32_t fatfs_vfs_statfs(vfs_file_t *fp, const char *path, vfs_statfs_t *stfs)
{
    int idx;
    FATFS *f;
    FRESULT res;
    DWORD fre_clust, fre_sect, tot_sect;
    char drive[5] = {0};
    size_t prefix_len;

    for (idx = 0; idx < (sizeof(g_fsid) / sizeof(g_fsid[0])); idx++) {
        prefix_len = strlen(g_fsid[idx].root);
        if (strncmp(g_fsid[idx].root, path, prefix_len) == 0) {
            break;
        }
    }

    if (idx == (sizeof(g_fsid) / sizeof(g_fsid[0]))) {
        return -1;
    }

    if (g_fatfs[idx] == NULL) {
        return -1;
    }

    f = g_fatfs[idx];

    snprintf(drive, sizeof(drive), "%d:", f->pdrv);
    res = f_getfree(drive, &fre_clust, &f);
    if (res) {
        return -1;
    }

    /* Get total sectors and free sectors */
    tot_sect = (f->n_fatent - 2) * f->csize;
    fre_sect = fre_clust * f->csize;

    memset(stfs, 0, sizeof(vfs_statfs_t));
    stfs->f_type = 0x4d44;
    stfs->f_bavail = fre_sect;
    stfs->f_bfree = fre_sect;
    stfs->f_blocks = tot_sect;
#if FF_MAX_SS != FF_MIN_SS
    stfs->f_bsize = f->ssize;
#else
    stfs->f_bsize = 512;
#endif
    stfs->f_files = 1024;

    return 0;
}

static int32_t fatfs_vfs_unlink(vfs_file_t *fp, const char *path)
{
    char    *relpath = NULL;
    int32_t  ret;

    relpath = translate_relative_path(path);
    if (!relpath) {
        return -EINVAL;
    }

    ret = f_unlink(relpath);

    aos_free(relpath);
    return ret;
}

static int32_t fatfs_vfs_rename(vfs_file_t *fp, const char *oldpath, const char *newpath)
{
    int32_t ret;

    char *oldname = NULL;
    char *newname = NULL;

    oldname = translate_relative_path(oldpath);
    if (!oldname) {
        return -EINVAL;
    }

    newname = translate_relative_path(newpath);
    if (!newname) {
        aos_free(oldname);
        return -EINVAL;
    }

    ret = f_rename(oldname, newname);

    aos_free(oldname);
    aos_free(newname);
    return ret;
}

static vfs_dir_t *fatfs_vfs_opendir(vfs_file_t *fp, const char *path)
{
    fat_dir_t *dp      = NULL;
    char      *relpath = NULL;

    relpath = translate_relative_path(path);
    if (!relpath) {
        return NULL;
    }

    dp = (fat_dir_t *)aos_malloc(sizeof(fat_dir_t) + MAX_NAME_LEN + 1);
    if (!dp) {
        aos_free(relpath);
        return NULL;
    }

    memset(dp, 0, sizeof(fat_dir_t) + MAX_NAME_LEN + 1);
    if (f_opendir(&dp->ffdir, relpath) == FR_OK) {
        aos_free(relpath);
        return (vfs_dir_t *)dp;
    }

    aos_free(relpath);
    aos_free(dp);
    return NULL;
}

static vfs_dirent_t *fatfs_vfs_readdir(vfs_file_t *fp, vfs_dir_t *dir)
{
    fat_dir_t    *dp = (fat_dir_t *)dir;
    vfs_dirent_t *out_dirent;

    if (!dp) {
        return NULL;
    }

    if (f_readdir(&dp->ffdir, &dp->filinfo) != FR_OK) {
        return NULL;
    }

    if (dp->filinfo.fname[0] == 0) {
        return NULL;
    }

    dp->cur_dirent.d_ino = 0;
    if (dp->filinfo.fattrib & AM_DIR) {
        dp->cur_dirent.d_type = AM_DIR;
    }

    strncpy(dp->cur_dirent.d_name, dp->filinfo.fname, MAX_NAME_LEN);
    dp->cur_dirent.d_name[MAX_NAME_LEN] = '\0';

    out_dirent = &dp->cur_dirent;
    return out_dirent;
}

static int32_t fatfs_vfs_closedir(vfs_file_t *fp, vfs_dir_t *dir)
{
    int32_t    ret = -EPERM;
    fat_dir_t *dp  = (fat_dir_t *)dir;

    if (!dp) {
        return -EINVAL;
    }

    ret = f_closedir(&dp->ffdir);
    if (ret == FR_OK) {
        aos_free(dp);
    }

    return ret;
}

static int32_t fatfs_vfs_mkdir(vfs_file_t *fp, const char *path)
{
    int32_t  ret     = -EPERM;
    char    *relpath = NULL;

    relpath = translate_relative_path(path);
    if (!relpath) {
        return -EINVAL;
    }

    ret = f_mkdir(relpath);

    aos_free(relpath);
    return ret;
}

static int32_t fatfs_vfs_rmdir(vfs_file_t *fp, const char *path)
{
    int32_t  ret     = -EPERM;
    char    *relpath = NULL;

    relpath = translate_relative_path(path);
    if (!relpath) {
        return -EINVAL;
    }

    ret = f_rmdir(relpath);

    aos_free(relpath);

    return ret;
}

static void fatfs_vfs_rewinddir(vfs_file_t *fp, vfs_dir_t *dir)
{
    fat_dir_t *dp = (fat_dir_t *)dir;

    if (!dp) {
        return;
    }

    f_rewinddir(&dp->ffdir);

    return;
}

static int32_t fatfs_vfs_telldir(vfs_file_t *fp, vfs_dir_t *dir)
{
    fat_dir_t *dp = (fat_dir_t *)dir;

    if (!dp) {
        return -1;
    }

    return (int32_t)(dp->ffdir.dptr);
}

static void fatfs_vfs_seekdir(vfs_file_t *fp, vfs_dir_t *dir, int32_t loc)
{
    fat_dir_t *dp = (fat_dir_t *)dir;

    if (!dp) {
        return;
    }

    dp->ffdir.dptr = loc;

    return;
}

static vfs_filesystem_ops_t fatfs_ops = {
    .open      = &fatfs_vfs_open,
    .close     = &fatfs_vfs_close,
    .read      = &fatfs_vfs_read,
    .write     = &fatfs_vfs_write,
    .access    = &fatfs_vfs_access,
    .lseek     = &fatfs_vfs_lseek,
    .sync      = &fatfs_vfs_sync,
    .stat      = &fatfs_vfs_stat,
    .statfs    = &fatfs_vfs_statfs,
    .unlink    = &fatfs_vfs_unlink,
    .rename    = &fatfs_vfs_rename,
    .opendir   = &fatfs_vfs_opendir,
    .readdir   = &fatfs_vfs_readdir,
    .closedir  = &fatfs_vfs_closedir,
    .mkdir     = &fatfs_vfs_mkdir,
    .rmdir     = &fatfs_vfs_rmdir,
    .rewinddir = &fatfs_vfs_rewinddir,
    .telldir   = &fatfs_vfs_telldir,
    .seekdir   = &fatfs_vfs_seekdir,
    .ioctl     = NULL
};

static int32_t fatfs_dev_register(int32_t pdrv)
{
    int32_t  err, index;
    FATFS   *fatfs = NULL;

    index = get_disk_index(pdrv);
    if (index < 0) {
        return -EINVAL;
    }

    if (g_fatfs[index] != NULL) {
        return FR_OK;
    }

    fatfs = (FATFS *)aos_malloc(sizeof(FATFS));
    if (!fatfs) {
        return -ENOMEM;
    }

    err = f_mount(fatfs, g_fsid[index].id, 1);

    if (err == FR_OK) {
        g_fatfs[index] = fatfs;
        return vfs_register_fs(g_fsid[index].root, &fatfs_ops, NULL);
    }

#if FF_USE_MKFS && !FF_FS_READONLY
    if (err == FR_NO_FILESYSTEM) {
        char *work = (char *)aos_malloc(FF_MAX_SS);
        if (!work) {
            err = -ENOMEM;
            goto error;
        }

        BYTE opt = FM_ANY;
        disk_ioctl(g_fsid[index].index, GET_FORMAT_OPTION, &opt);

        err = f_mkfs(g_fsid[index].id, opt, 0, work, FF_MAX_SS);
        aos_free(work);

        if (err != FR_OK) {
            goto error;
        }

        f_mount(NULL, g_fsid[index].id, 1);
        err = f_mount(fatfs, g_fsid[index].id, 1);

        if (err == FR_OK) {
            g_fatfs[index] = fatfs;
            return vfs_register_fs(g_fsid[index].root, &fatfs_ops, NULL);
        }
    }
error:
#endif
    aos_free(fatfs);
    return err;
}


static int32_t fatfs_dev_unregister(int32_t pdrv)
{
    int32_t index;
    int32_t err = FR_OK;

    index = get_disk_index(pdrv);
    if (index < 0) {
        return -EINVAL;
    }

    err = vfs_unregister_fs(g_fsid[index].root);
    if (err == FR_OK) {
        f_mount(NULL, g_fsid[index].id, 1);
        aos_free(g_fatfs[index]);
        g_fatfs[index] = NULL;
    }

    return err;
}

int32_t fatfs_register(void)
{
    int32_t err = -EINVAL;

#ifdef CONFIG_AOS_FATFS_SUPPORT_MMC
    if ((err = fatfs_dev_register(DEV_MMC)) != FR_OK) {
        return err;
    }
#endif

#ifdef CONFIG_AOS_FATFS_SUPPORT_USB
    if ((err = fatfs_dev_register(DEV_USB)) != FR_OK) {
        return err;
    }
#endif

#ifdef CONFIG_AOS_FATFS_SUPPORT_RAM
    if ((err = fatfs_dev_register(DEV_RAM)) != FR_OK) {
        return err;
    }
#endif

    return err;
}

int32_t fatfs_unregister(void)
{
    int32_t err = -EINVAL;

#ifdef CONFIG_AOS_FATFS_SUPPORT_MMC
    if ((err = fatfs_dev_unregister(DEV_MMC)) != FR_OK) {
        return err;
    }
#endif

#ifdef CONFIG_AOS_FATFS_SUPPORT_USB
    if ((err = fatfs_dev_unregister(DEV_USB)) != FR_OK) {
        return err;
    }
#endif

#ifdef CONFIG_AOS_FATFS_SUPPORT_RAM
    if ((err = fatfs_dev_unregister(DEV_RAM)) != FR_OK) {
        return err;
    }
#endif

    return err;
}
