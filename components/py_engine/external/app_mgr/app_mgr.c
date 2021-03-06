/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include "amp_boot_file_transfer.h"
#include "amp_platform.h"
#include "amp_task.h"
#include "aos/kernel.h"
#include "aos_fs.h"
#include "aos_system.h"
#include "aos_tcp.h"
#include "app_upgrade.h"
#include "httpclient.h"
#include "mbedtls/md5.h"
#include "ota_agent.h"
#include "py_defines.h"
#include "app_mgr.h"

#define MOD_STR "APP_MGR"

static char app_version[32] = { 0 };
static char save_path[256] = { 0 };
static int32_t save_path_len = 0;

static PYAMP_SAVE_PATH_DEFULT pyamp_defult_save_path = PYAMP_SAVE_PATH_DEFULT_DATA;

void pyamp_save_path_defult_set(PYAMP_SAVE_PATH_DEFULT mode)
{
    pyamp_defult_save_path = mode;
}

int pyamp_save_path_defult_get()
{
    return pyamp_defult_save_path;
}

int pyamp_check_file_path(unsigned char *path, int32_t path_len)
{
    int ret = 0;
    // set path to test
    char test_path[256] = { 0 };
    char transferpath[256] = { 0 };

    memset(transferpath, 0, path_len);
    memcpy(transferpath, path, path_len);

    strncat(test_path, transferpath, sizeof(test_path) - 1);
    strncat(test_path, "_pathTest", sizeof(test_path) - 1);

    ret = aos_mkdir(test_path);
    if (ret == 0) {
        ret = aos_rmdir_r(test_path);
    }

    return ret;
}

void pyamp_save_file_path(unsigned char *path, int32_t path_len)
{
    if (path_len != 0) {
        save_path_len = path_len;
        memset(save_path, 0, path_len);
        memcpy(save_path, path, path_len);
    } else {
        save_path_len = path_len;
    }
}

int pyamp_get_file_path(unsigned char *path, int32_t path_len)
{
    if (save_path_len != 0) {
        memcpy(path, save_path, path_len);
        return save_path_len;
    }
    return 0;
}

void pyamp_apppack_init()
{
    /* remove all js file */
    char path[256] = { 0 };
    int path_len = 0;
    path_len = pyamp_get_file_path(path, 256);
    if (path_len != 0) {
        aos_rmdir_r(path);
        aos_mkdir(path);
    } else {
        PYAMP_SAVE_PATH_DEFULT save_path = PYAMP_SAVE_PATH_DEFULT_DATA;
        save_path = pyamp_save_path_defult_get();
        if (save_path == PYAMP_SAVE_PATH_DEFULT_SDCARD) {
            aos_rmdir_r(AMP_FS_EXT_ROOT_DIR);
            aos_mkdir(AMP_FS_EXT_ROOT_DIR);
        } else {
            aos_rmdir_r(AMP_FS_ROOT_DIR);
            aos_mkdir(AMP_FS_ROOT_DIR);
        }
    }
}

static int task_http_download_func(char *url, char *filepath)
{
    httpclient_t client = { 0 };
    httpclient_data_t client_data = { 0 };
    int num = 0;
    int ret;
    char rsp_buf[2048];

    char *req_url = url;
    int fd = aos_open(filepath, O_CREAT | O_RDWR);

    memset(rsp_buf, 0, sizeof(rsp_buf));
    client_data.response_buf = rsp_buf;
    client_data.response_buf_len = sizeof(rsp_buf);

    ret = httpclient_conn(&client, req_url);

    if (!ret) {
        ret = httpclient_send(&client, req_url, HTTP_GET, &client_data);

        do {
            ret = httpclient_recv(&client, &client_data);
            // printf("response_content_len=%d,
            // retrieve_len=%d,content_block_len=%d\n",
            // client_data.response_content_len,client_data.retrieve_len,client_data.content_block_len);
            // printf("ismore=%d\n", client_data.is_more);
            num = aos_write(fd, client_data.response_buf, client_data.content_block_len);
            if (num > 0) {
                // printf("aos_write num=%d\n", num);
            }
        } while (client_data.is_more);
    }
    ret = aos_sync(fd);
    httpclient_clse(&client);
    return ret;
}

// int apppack_download(char *url, download_js_cb_t func)
int pyapp_download_and_upgrade(char *url)
{
    int ret = -1;
    amp_warn(MOD_STR, "url = %s", url);
    ret = task_http_download_func(url, AMP_PY_PKG_DEFAULE);
    if (ret) {
        amp_error(MOD_STR, "online debug download failed");
    } else {
        amp_warn(MOD_STR, "begin to install pyapp");
        // ret = install_pyapp(AMP_PY_PKG_DEFAULE, AMP_PY_PKG_DIR);
        // amp_debug(MOD_STR, "upgrade sub module\n");
        if ((AMP_PY_PKG_DEFAULE != NULL) && (AMP_PY_PKG_DIR != NULL)) {
            amp_debug(MOD_STR, "store_file:%s, install_file:%s\n", AMP_PY_PKG_DEFAULE, AMP_PY_PKG_DIR);
            ret = miniUnzip(AMP_PY_PKG_DEFAULE, AMP_PY_PKG_DIR);
        }
        if (ret < 0) {
            amp_error(MOD_STR, "py app install failed\n");
        }
    }
    return ret;
}

static int32_t update_done = 1;
static int app_fp = -1;

int pyamp_download_apppack(uint8_t *buf, int32_t buf_len)
{
    int ret = -1;
    amp_warn(MOD_STR, "download buf len = %d", buf_len);
    int fp = aos_open("/data/pyamp/app.zip", O_CREAT | O_RDWR);
    if (fp < 0) {
        LOGE(MOD_STR, " open /data/pyamp/app.zip fail ");
        return 0;
    }
    ret = aos_write(fp, buf, buf_len);
    if (ret <= 0) {
        LOGE(MOD_STR, " write  /data/pyamp/app.zip fail\n");
        aos_close(fp);
        return 0;
    }
    aos_close(fp);
    return 0;
}

extern void *task_mutex;
extern int pyamp_upgrading;
static void download_work(void *arg)
{
    int ret;
    aos_task_t restart_task;

    // amp_warn(MOD_STR, "download_work task name=%s", jse_osal_get_taskname());
    amp_warn(MOD_STR, "url=%s", (char *)arg);

    ret = pyapp_download_and_upgrade((char *)arg);
    amp_debug(MOD_STR, "pyapp_download_and_upgrade done:%d", ret);
    update_done = 1;

    if (ret == 0) {
        amp_warn(MOD_STR, "Upgrade app success");
        aos_msleep(200);
    }

    pyamp_upgrading = 0;

    aos_reboot();

    aos_task_exit(0);
    return;
}

int pyamp_apppack_upgrade(char *url)
{
    pyamp_upgrading = 1;
    aos_task_t upgrade_task;

    amp_warn(MOD_STR, "pyamp_apppack_upgrade url=%s ", (char *)url);

    if (update_done) {
        update_done = 0;
        pyamp_apppack_init();

        amp_warn(MOD_STR, "create upgrade task ...");

        if (aos_task_new_ext(&upgrade_task, "amp_upgrade", download_work, (void *)url, 1024 * 6, AOS_DEFAULT_APP_PRI) !=
            0) {
            update_done = 1;
            amp_warn(MOD_STR, "jse_osal_task_new fail");

            return -1;
        }

    } else {
        aos_free(url);
        amp_warn(MOD_STR, "apppack pyamp_upgrading...");
    }

    return 0;
}

void pyamp_app_version_set(char *version)
{
    if (!version)
        return;
    snprintf(app_version, sizeof(app_version), "%s", version);
}

const char *pyamp_jsapp_version_get(void)
{
    return app_version;
}
