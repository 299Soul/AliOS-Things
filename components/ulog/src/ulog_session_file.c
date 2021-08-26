/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include "ulog_session_file.h"
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "ulog_api.h"
#include "ulog/ulog.h"

#include "aos/errno.h"
#include "cJSON.h"
#include "aos/kernel.h"
#include "uagent.h"

/**
 * record the log files operating. Recover(reset or get) in reload_log_argu()
 * used and count in write_log_line()
 */
static uint32_t operating_file_offset = 0;
static uint32_t gu32_log_file_size = LOCAL_FILE_SIZE;
static char guc_logfile_path[ULOG_FILE_PATH_SIZE / 2] = {0};
/**
* indicates if log on fs feature initialized
*
*/
static uint8_t session_fs_init = 0;

static uint8_t log_file_failed = 0;

#if ULOG_RESERVED_FS
static char *ulog_fs_tmp[ULOG_RESERVED_FS_SIZE] = { NULL };

static uint16_t tmp_queue_in = 0;
static uint16_t tmp_queue_out = 0;

static void stop_operating();

static int push_fs_tmp(const char *data, const unsigned short len)
{
    int rc = -1;
    if (NULL != data && len != 0 && ((tmp_queue_in + 1) % ULOG_RESERVED_FS_SIZE) != tmp_queue_out) {
        if (NULL == ulog_fs_tmp[tmp_queue_in]) {
            ulog_fs_tmp[tmp_queue_in] = (char *)aos_malloc(len + 1);
            if (NULL != ulog_fs_tmp[tmp_queue_in]) {
                memcpy(ulog_fs_tmp[tmp_queue_in], data, len);
                ulog_fs_tmp[tmp_queue_in][len] = '\0';
                tmp_queue_in = (tmp_queue_in + 1) % ULOG_RESERVED_FS_SIZE;
                rc = 0;
            }
        }
    }
    return rc;
}

static int pop_fs_tmp(char *data, const unsigned short len)
{
    int rc = -EINVAL;
    if (NULL != data && len != 0) {
        if (tmp_queue_in == tmp_queue_out) {
            rc = -1;
        } else {
            if (NULL != ulog_fs_tmp[tmp_queue_out]) {
                strncpy(data, ulog_fs_tmp[tmp_queue_out], len - 1);
                aos_free(ulog_fs_tmp[tmp_queue_out]);
                ulog_fs_tmp[tmp_queue_out] = NULL;
                tmp_queue_out = (tmp_queue_out + 1) % ULOG_RESERVED_FS_SIZE;
                rc = 0;

            } else {
                rc = -EIO;
            }
        }
    }
    return rc;
}
#endif

#if ULOG_UPLOAD_LOG_FILE
#include "sys/socket.h"
#include "httpclient.h"
static httpclient_t *httpc_handle = NULL ;


static char *up_uri = NULL;
static bool http_client_initd = false;

static int get_server_uri(const char *url, char **server, char **uri)
{
    int rc = -1;
    if (NULL != url) {
        uint8_t pos = -1;
        if (0 == strncmp(url, URL_PREFIX_HTTP, strlen(URL_PREFIX_HTTP))) {
            pos = strlen(URL_PREFIX_HTTP);
        } else if (0 == strncmp(url, URL_PREFIX_HTTPS, strlen(URL_PREFIX_HTTPS))) {
            pos = strlen(URL_PREFIX_HTTPS);
        }
        if (pos > 0) {
            char *p = NULL;
            p = strchr(&url[pos], '/');
            if (NULL != p) {
                p++;
                *server = (char *)aos_malloc(p - url + 1);
                if (NULL != *server) {
                    memcpy(*server, url, p - url);
                    (*server)[p - url] = '\0';
                    const short n = strlen(p);
                    *uri = (char *)aos_malloc(n + 1);
                    if (NULL != *uri) {
                        memcpy(*uri, p, n);
                        (*uri)[n] = '\0';
                        rc = 0;
                    } else {
                        aos_free(*server);
                        *server = NULL;
                    }
                }
            }
        }
    }
    return rc;
}

static void report_up_process(const http_upload_fail_t status, const char process, const ulog_idx_type idx)
{
    cJSON *resp = cJSON_CreateObject();
    if (NULL != resp) {
        char *text = NULL;
        cJSON_AddItemToObject(resp, "idx", cJSON_CreateNumber(idx));
        if (status >= http_upload_start) {
            cJSON_AddItemToObject(resp, "process", cJSON_CreateNumber(process));
        } else {
            cJSON_AddItemToObject(resp, "error", cJSON_CreateNumber(status));
        }
        text = cJSON_PrintUnformatted(resp);
        cJSON_Delete(resp);
        uagent_send(UAGENT_MOD_ULOG, ULOG_LOG_LIST, strlen(text), text, send_policy_object);
        cJSON_free(text);
    }
}

int http_start(const char *url, const unsigned short idx)
{
    int rc = -1;
    if (!http_client_initd) {
        http_client_initd = true;
    }

    if (httpc_handle == NULL) {
        LOGI(ULOG_TAG_SELF, "http start %s idx %d", url, idx);
        httpc_handle = (httpclient_t *)aos_malloc(sizeof(httpclient_t));
        if (NULL != httpc_handle) {
            up_uri = (uint8_t *)aos_malloc(strlen(url));
            if (NULL != up_uri) {
                strncpy(up_uri, url, strlen(url));
                char buf[16];
                snprintf(buf, 16, "up idx=%d", idx);
                rc = ulog_man(buf);
            } else {
                aos_free(httpc_handle);
                httpc_handle = NULL;
            }
        } else {
            LOGE(ULOG_TAG_SELF, "allock http connect instanse fail");
        }
    } else {
        LOGE(ULOG_TAG_SELF, "Last upload not finish");
    }
    return rc;
}


static uint8_t *rsp_buf = NULL;
static uint8_t *req_buf = NULL;
void on_fs_upload(const uint32_t idx, const uint32_t start)
{
    if (get_working_from_cfg_mm() == idx) {
        on_fs_record_pause(1, 0);
    }
    int fd = open_log_file(idx, O_RDONLY, 0);
    http_upload_fail_t http_operate = http_upload_common_fail;
    if (fd >= 0) {
        char *customer_header = "Accept: text/xml,text/javascript,text/html,application/json\r\n";
        httpclient_set_custom_header(httpc_handle, customer_header);
        char *upload_stream = (char *)aos_malloc(gu32_log_file_size + ULOG_SIZE);
        if (NULL != upload_stream) {
            int n = -1;
            n = aos_read(fd, upload_stream, gu32_log_file_size + ULOG_SIZE);
            if (0 < n) {
                char retry = HTTP_REQ_RETRY;
                httpclient_data_t client_data = {0};
                rsp_buf = (uint8_t *)aos_malloc(RSP_BUF_SIZE);
                req_buf = (uint8_t *)aos_malloc(RSP_BUF_SIZE);
                if (rsp_buf != NULL && req_buf != NULL) {
                    memset(req_buf, 0, sizeof(req_buf));
                    client_data.header_buf = req_buf;
                    client_data.header_buf_len = sizeof(req_buf);

                    memset(rsp_buf, 0, sizeof(rsp_buf));
                    client_data.response_buf = rsp_buf;
                    client_data.response_buf_len = sizeof(rsp_buf);

                    client_data.post_buf = upload_stream;
                    client_data.post_buf_len = n;
                    client_data.post_content_type = "text/plain";

                    while (HTTP_SUCCESS != httpclient_put(httpc_handle, up_uri, &client_data)) {
                        if (--retry <= 0) {
                            break;
                        } else {
                            LOGW(ULOG_TAG_SELF, "fs rqst %d fail", n);
                            aos_msleep(1000);
                        }
                    }
                    if (retry > 0) {
                        http_operate = http_upload_finish;
                    } else {
                        http_operate = http_upload_memory_fail;
                    }
                }
            } else {
                http_operate = http_upload_text_empty;
                LOGW(ULOG_TAG_SELF, "nothing read");
            }
            aos_free(upload_stream);
        } else {
            http_operate = http_upload_memory_fail;
            LOGE(ULOG_TAG_SELF, "allocate file fail");
        }
    } else {
        LOGE(ULOG_TAG_SELF, "http construct header fail");
    }
    aos_close(fd);

    report_up_process(http_operate, http_operate == http_upload_finish ? 100 : 0, idx);

    httpclient_clse(httpc_handle);
    aos_free(rsp_buf);
    rsp_buf = NULL;
    aos_free(req_buf);
    req_buf = NULL;
    aos_free(up_uri);
    up_uri = NULL;
    aos_free(httpc_handle);
    httpc_handle = NULL;
    /* try re-start record, no impact even the record no abort before */
    on_fs_record_pause(0, 1);
}

#endif

/**
*
* Check if file already exist
*
* @param  file_idx file index to be checked
* @return true indicats it is existed, or else not.
*/
static bool log_file_exist(const uint16_t file_idx)
{
    bool rc = false;
    char ulog_file_name[ULOG_FILE_PATH_SIZE] = {0};
    int fd = -1;

    if (file_idx <= LOCAL_FILE_CNT) {
        snprintf(ulog_file_name, sizeof(ulog_file_name), ULOG_FILE_FORMAT, guc_logfile_path, file_idx);
        fd = aos_open(ulog_file_name, (O_RDWR | O_CREAT | O_EXCL)
#ifdef CSP_LINUXHOST
                                , 0644
#endif
                               );
        SESSION_FS_DEBUG("check if file %s exist %d\n", ulog_file_name, fd);

        if (fd < 0) {
#ifdef CSP_LINUXHOST
            if (EEXIST == errno) {
                SESSION_FS_DEBUG("file %s alreay exist\n", ulog_file_name);
                rc = true;
            }
#else /* !CSP_LINUXHOST */
            if (fd == -EEXIST) {
                SESSION_FS_DEBUG("file %s alreay exist\n", ulog_file_name);
                rc = true;
            }
#endif /* CSP_LINUXHOST */
        } else {
            /*TODO:why not close*/
            aos_close(fd);
            aos_unlink(ulog_file_name);
        }
    }
    return rc;
}

static int open_create_log_file(const ulog_idx_type file_idx, const bool keep_open)
{
    int fd = -1;
    char ulog_file_name[ULOG_FILE_PATH_SIZE] = {0};

    snprintf(ulog_file_name, sizeof(ulog_file_name), ULOG_FILE_FORMAT, guc_logfile_path, file_idx);
    SESSION_FS_DEBUG("open create log %s\n", ulog_file_name);
    aos_unlink(ulog_file_name);
    fd = aos_open(ulog_file_name, (O_RDWR | O_CREAT | O_TRUNC)
#ifdef CSP_LINUXHOST
                  , 0644
#endif
                 );
    if (fd >= 0) {
        if (!keep_open) {
            aos_close(fd);
        }
    } else {
        SESSION_FS_INFO("open create file %s fail fd %d, errno %d\n", ulog_file_name, fd, errno);
    }
    return fd;
}

int open_log_file(const ulog_idx_type file_idx, int flag, const off_t off)
{
    int fd = -1;
    char ulog_file_name[ULOG_FILE_PATH_SIZE];
    memset(ulog_file_name, 0, sizeof(ulog_file_name));
    snprintf(ulog_file_name, sizeof(ulog_file_name), ULOG_FILE_FORMAT, guc_logfile_path, file_idx);
    fd = aos_open(ulog_file_name, flag);
    if (fd >= 0) {
        const int seek_off = aos_lseek(fd, off, SEEK_SET);
        if (seek_off != off) {
            SESSION_FS_INFO("seek fail %s %d\n", ulog_file_name, seek_off);
            aos_close(fd);
            fd = -1;
        }
    } else {
        SESSION_FS_INFO("open %s flag %d fail fd %d\n", ulog_file_name, flag, fd);
    }
    return fd;
}

/**
*
* Copies one line from file_instanse into buf, On sucessful the destination buf
* buf is changed into NULL terminated C String. If log content size is longer
* than buf_len-1, remain will be not saved and NULL terminatated is implicitly
* appended at the end of destination.
*
* @param  file_instanse file description from aos_open() before
* @param  buf local buffer use for saved, ZERO format is not MUST
* @param  buf_len buffer size
* @return actual log text length readed in argumenent buf;
*         '\n',' is counted. Expected value: 1~buf_len-1
*         0 indicates EOF of the file, buf_len indicates the possible passing value is
*         limited to read the whole line.
*/
int get_log_line(const int fd, char *buf, const uint16_t buf_len)
{
    int rc = -1;
    int cnt = 0;

    if (fd < 0 || NULL == buf || buf_len <= 0) {
        return -1;
    }

    memset(buf, 0, buf_len);
    while ((cnt < buf_len) && (0 < aos_read(fd, &buf[cnt], 1))) {
        if (buf[cnt] == LOG_LINE_SEPARATOR) {
            break;
        }
        cnt++;
    }

    if (cnt == 0) {
        /* Nothing read, this is an empty file */
        rc = 0;
    } else if (cnt < buf_len) {
        if (buf[cnt - 1] == LOG_LINE_SEPARATOR) {
            /* replacement/end with null terminated */
            buf[cnt - 1] = 0;
        } else {
            buf[cnt] = 0;
        }
        rc = cnt;

    } else {/* cnt == buf_len */
        /* two possible result */
        /* buffer len is just fit */
        /* buffer is not sufficient to save whole line,
        last characher will be missed and replace of null-terminated */
        rc = cnt;

        /* replacement with null terminated */
        buf[cnt - 1] = 0;
    }

    return rc;
}

/**
*
* Write one line into specify log file, which instance is file_instanse. Append a LOG_LINE_SEPARATOR
* after log context to separate logs
*
* @param  file_instanse file description from aos_open() before
* @param  buf local buffer use for write
* @param  keep_open keep opening after write finished
* @param operating indicates the rolling-back mechanism used.
* @return actual writen text length(includes length of LOG_LINE_SEPARATOR).
*         -EINVAL indicates parameter illegal, other value indicates call aos_write failure
*/
int write_log_line(const int file_instanse, const char *buf, const bool keep_open)
{
    int rtn = -EINVAL;
    if (file_instanse >= 0 && buf != NULL) {
        int rc = -1;
        rtn = aos_write(file_instanse, buf, strlen(buf));
        if (rtn > 0) {
            rc = aos_write(file_instanse, LOG_LINE_END_STR, 1);
            if (1 == rc) {
                rtn++;
            } else {
                rtn = rc;
            }
            aos_sync(file_instanse);
        } else {
            SESSION_FS_INFO("write fail rc %d\n", rtn);
        }

        if (!keep_open) {
            aos_close(file_instanse);
        }
    }

    return rtn;
}

/**
*
* Refresh ulog cfg item and saved in cfg file, also create the new ulog file
*
*
* @return  0 indicates create new log file sucessfully, but not means update config pass;
*          -1 create new log file fail
*/
static int update_new_log_file(const ulog_idx_type idx)
{
    int rc = -1;
    int fd = -1;

    if (0 == update_mm_cfg(ulog_cfg_type_working, idx, ulog_cfg_para_none, NULL)) {
        /* read it for test */
        if (get_working_from_cfg_mm() == idx) {

            /* create log file */
            if (open_create_log_file(idx, false) >= 0) {
                rc = 0;
                char time_start[24];

                ulog_format_time(time_start, sizeof(time_start));

                update_mm_cfg(ulog_cfg_type_list, idx, ulog_cfg_para_start, time_start);
                fd = open_create_log_file(ULOG_FILE_CFG_IDX, true);

                if (fd >= 0) { /* need update cfg file */
                    if (0 != cfg_mm_2_file(fd)) {
                        /* sync to cfg file fail, have no impact unless the board boot-up before it sync write */
                        SESSION_FS_INFO("sync to cfg file fail %d\n", fd);
                    }
                    aos_close(fd);
                } else {
                    SESSION_FS_INFO("refresh ulog cfg fail fd %d\n", fd);
                }

            } else {
                SESSION_FS_INFO("sync to cfg file fail %d\n", fd);
            }
        } else {

            SESSION_FS_INFO("Fatal Error Operate ulog mm cfg\n");
        }
    } else {

        SESSION_FS_INFO("Fatal Error update ulog mm cfg\n");
    }

    return rc;
}

/**
*
* Reload log history arguments from ulog cfg file(ulog_000.log) in fs,
* includes operating_file_idx & operating_file_offset. New ulog_000.log
* will be created if none ulog_000.log found or text is illegal.
* This is vital for ulog pop via fs, ulog pop via fs will be forbidden if
* this step fail.
*
* return 0 if this step pass, else indicates this step fail
*
*/
static int reload_log_argu()
{
    int rc = -1;
    struct aos_stat st_logstat = {0};
    char ulog_file_name[ULOG_FILE_PATH_SIZE] = {0};
    ulog_idx_type tmp_idx = ULOG_FILE_IDX_INVALID;

    operating_file_offset = 0;

    if (log_file_exist(ULOG_FILE_CFG_IDX)) {
        /* ulog cfg exist, try to read it */
        load_cfg_2_mm();
        tmp_idx = get_working_from_cfg_mm();

        SESSION_FS_INFO("[%s]log file idx %d\n", ULOG_TAG_SELF, tmp_idx);
        if (tmp_idx <= LOCAL_FILE_CNT) {
            snprintf(ulog_file_name, sizeof(ulog_file_name), ULOG_FILE_FORMAT, guc_logfile_path, tmp_idx);
            rc = aos_stat(ulog_file_name, &st_logstat);
            if (rc == 0) {
                operating_file_offset += st_logstat.st_size;
            } else {
                /* no such log file exist, then create it */
                rc = update_new_log_file(tmp_idx);
            }
        }
    }

    if (LOCAL_FILE_CNT < tmp_idx) {
        rc = update_new_log_file(ULOG_FILE_IDX_START);
    }

    return rc;
}

void on_show_ulog_file()
{
    aos_dir_t *dp;
    SESSION_FS_INFO("log files in %s\n", guc_logfile_path);
    dp = (aos_dir_t *)aos_opendir(guc_logfile_path);

    if (dp != NULL) {
        aos_dirent_t *out_dirent;
        while (1) {
            out_dirent = (aos_dirent_t *)aos_readdir(dp);
            if (out_dirent != NULL) {
                SESSION_FS_INFO("file name is %s\n", out_dirent->d_name);
            } else {
                break;
            }
        }
    }
    aos_closedir(dp);
}

static void stop_operating()
{
    char time_stop[24];

    ulog_format_time(time_stop, sizeof(time_stop));
    update_mm_cfg(ulog_cfg_type_list, get_working_from_cfg_mm(), ulog_cfg_para_end, time_stop);
}

static void write_fail_retry()
{
    int8_t retry = ULOG_FILE_FAIL_COUNT;
    char log_file_name[ULOG_FILE_PATH_SIZE];
    char buf[ULOG_SIZE];

    log_file_failed++;
    if (log_file_failed >= ULOG_FILE_FAIL_COUNT) {
        snprintf(log_file_name, ULOG_FILE_PATH_SIZE, ULOG_FILE_FORMAT, guc_logfile_path, get_working_from_cfg_mm());
        while (0 != aos_unlink(log_file_name)) {
            if (--retry <= 0) {
                SESSION_FS_INFO("file %s error on remove, retry %d\n", log_file_name, retry);
                break;
            }
        }

        if (retry > 0) {
            SESSION_FS_INFO("remove file %s, then create new one %d\n", log_file_name, get_working_from_cfg_mm());
            if (0 == update_new_log_file(get_working_from_cfg_mm())) {
#if ULOG_RESERVED_FS
                memset(buf, 0, ULOG_SIZE);
                while (0 == pop_fs_tmp(buf, ULOG_SIZE)) {
                    pop_out_on_fs(buf, strlen(buf));
                    memset(buf, 0, ULOG_SIZE);
                }
#endif /* ULOG_RESERVED_FS */
            }
        }
    }
}

/**
* @brief not thread-safe, but only be used in one task(ulog), so not necessary considering mutex
* @param data
* @param len
*
* @return -1 indicates not send out sucessfully
*
*/
int32_t pop_out_on_fs(const char *data, const uint16_t len)
{
    int32_t rc = -1;
    int fd = 0;
    int write_rlt = 0;
    ulog_idx_type idx = ULOG_FILE_IDX_INVALID;

    idx = get_working_from_cfg_mm();
    if (idx > LOCAL_FILE_CNT) {
        SESSION_FS_INFO("fail to get working log file idx %d update working cfg\n", idx);
        rc = reload_log_argu();
        if (rc < 0) {
            SESSION_FS_INFO("fail to pop log to fs for reload log cfg fail \n");
            return -1;
        }
        /*this time idx will be fine not need to check*/
        idx = get_working_from_cfg_mm();
    }

    fd = open_log_file(idx, O_WRONLY, operating_file_offset);
    if (fd < 0) {
        SESSION_FS_INFO("fail to pop log to fs for open working log file %d offset fail %d \n", idx, operating_file_offset, errno);
        rc = -1;
#if ULOG_RESERVED_FS
        rc = push_fs_tmp(data, len);
        if (0 != rc) {
            SESSION_FS_INFO("*(%d)", rc);
        }
        return rc;
#endif
    }


    write_rlt = write_log_line(fd, data, true);
    aos_sync(fd);
    aos_close(fd);
    if (write_rlt < 0) {
        SESSION_FS_INFO("write fail %d retry %d\n", write_rlt, log_file_failed);
        rc = -1;
#if ULOG_RESERVED_FS
        /* save them temporary */
        rc = push_fs_tmp(data, len);
        if (0 != rc) {
            SESSION_FS_INFO("*(%d)", rc);
        }
#endif /* ULOG_RESERVED_FS */
        /* check fail count */
        write_fail_retry();
        return rc;
    }

    log_file_failed = 0;
    operating_file_offset += write_rlt;
    if (operating_file_offset >= gu32_log_file_size) {
        stop_operating();

        /* roll back if working index reaches end */
        idx++;
        if (idx > LOCAL_FILE_CNT) {
            idx = ULOG_FILE_IDX_START;
        }
        operating_file_offset = 0;
        rc = update_new_log_file(idx);
        if (rc) {
            SESSION_FS_INFO("creat new log file %d fail %d in pop to fs\n", idx, rc);
        }
    }

    return 0;
}

void on_fs_record_pause(const uint32_t on, const uint32_t off)
{
    if ((on ^ off) == 0) {
        return ;
    }

    SESSION_FS_INFO(ULOG_TAG_SELF, "ulog fs ctrl on %d off %d \n", on, off);
    if (1 == on) {
        stop_operating();
    } else {/* resume the file record */
            if (0 == reload_log_argu()) {
                LOGI(ULOG_TAG_SELF, "reload ulog idx %d off %d",
                        get_working_from_cfg_mm(), operating_file_offset);
#if ULOG_RESERVED_FS
                char buf[ULOG_SIZE];
                memset(buf, 0, ULOG_SIZE);
                while (0 == pop_fs_tmp(buf, ULOG_SIZE)) {
                    pop_out_on_fs(buf, strlen(buf));
                    memset(buf, 0, ULOG_SIZE);
                }
#endif
            } else {
                LOGE(ULOG_TAG_SELF, "restart ulog fs fail");
            }
        }
}

void fs_control_cli(const char cmd, const char *param)
{
    if (param != NULL) {
        switch (cmd) {
            case 't': {
                int control_cmd = strtoul(param, NULL, 10);
                if (control_cmd) {
                    ulog_man("fspause on=1");
                } else {
                    ulog_man("fspause off=1");
                }
                break;
            }
            default:
                break;
        }
    }
}

int ulog_fs_log_file_size(unsigned int filesize)
{
    if (filesize < ULOG_SIZE) {
        return -1;
    }

    gu32_log_file_size = filesize;
    return 0;
}


int ulog_fs_log_file_path(char *filepath)
{
    size_t len = 0;
    size_t max_len = 0;
    int ret = 0;
    aos_dir_t *pstdir = NULL;

    if (NULL == filepath) {
        return -1;
    }

    len = strlen(filepath);
    /*we need to reserve one byte for /0 and one byte for / */
    max_len = (ULOG_FILE_PATH_SIZE / 2) - 1;
    if (len > max_len) {
        SESSION_FS_INFO("log file path length %d over size %d", len, max_len - 1);
        return -1;
    }
    memset(guc_logfile_path, 0, sizeof(guc_logfile_path));
    memcpy(guc_logfile_path, filepath, len);

    /*remove / at first to make dir ,then and / */
    if (filepath[len - 1] == '/') {
        guc_logfile_path[len - 1] = 0;
    }

    /*check logfile path exist */
    pstdir = aos_opendir(guc_logfile_path);
    if (NULL == pstdir) {
        /*log file path doesn't exist , creat it*/
        ret = aos_mkdir(guc_logfile_path);
        if (ret) {
            SESSION_FS_INFO("%s %d log dir path %s doesn't exist and mkdir fail %d \r\n", __FILE__, __LINE__, guc_logfile_path, ret);
            return -1;
        }
    } else {
        aos_closedir(pstdir);
    }

    /*and / for log file process*/
    guc_logfile_path[len - 1] = '/';

    return 0;
}

/**
* @brief ulog on fs init
*
* @return 0 indicates initialized sucessfully, or else fail
*
*/
int32_t ulog_fs_init()
{
    int32_t rc = -1;
    if (0 == session_fs_init) {
        session_fs_init = 1;
        cfg_init_mutex();
        ulog_fs_log_file_path(ULOG_DEAULT_FS_PATH);
        rc = reload_log_argu();
        if (rc == 0) {
            SESSION_FS_INFO("reload ulog idx %d len %d \n", get_working_from_cfg_mm(), operating_file_offset);
        }
    }
    return rc;
}

