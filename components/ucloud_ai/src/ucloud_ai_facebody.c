/*
 * Copyright (C) 2021-2023 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <ulog/ulog.h>
#include "ucloud_ai_common.h"

#define TAG "UCLOUD_AI_FACEBODY"

void ucloud_ai_facebody_comparing_face(char *path, char *myface, ucloud_ai_cb_t cb)
{
    int len;
    char *p_upload_url = NULL;
    char *p_myface_url = NULL;

    /*update capture.jpg to oss*/
    if (path) {
        p_upload_url = ucloud_ai_upload_file(path);
        if (!p_upload_url)
            return;
    }

    if (myface) {
        p_myface_url = ucloud_ai_upload_file(myface);
        if (!p_myface_url)
            return;
    }
    /*do facebody detection*/
    compareFace(p_upload_url, p_myface_url, cb);
    if (p_upload_url)
        free(p_upload_url);
    if (p_myface_url)
        free(p_myface_url);
}

void ucloud_ai_facebody_recognize_expression(char *path, ucloud_ai_cb_t cb)
{
    int ret;
    int len;
    char *p_upload_url;

    /*update capture.jpg to oss*/
    p_upload_url = ucloud_ai_upload_file(path);
    if (!p_upload_url)
        return;

    recognizeExpression(p_upload_url, cb);
    free(p_upload_url);
}

void ucloud_ai_facebody_generate_human_anime_style(char *path, ucloud_ai_cb_t cb)
{
    int ret;
    int len;
    char *p_upload_url;

    /*update capture.jpg to oss*/
    p_upload_url = ucloud_ai_upload_file(path);
    if (!p_upload_url)
        return;

    /*do generating human anime style, and get image url*/
    generateHumanAnimeStyle(p_upload_url, cb);
    free(p_upload_url);
}

void ucloud_ai_facebody_detect_pedestrian(char *path, ucloud_ai_cb_t cb)
{
    int ret;
    int len;
    char *p_upload_url;

    /*update capture.jpg to oss*/
    p_upload_url = ucloud_ai_upload_file(path);
    if (!p_upload_url)
        return;

    /*do generating human anime style, and get image url*/
    detectPedestrian(p_upload_url, cb);
    free(p_upload_url);
}
