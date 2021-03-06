/*
 * Copyright (c) 2021 Bestechnic (Shanghai) Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifdef __INTERCONNECTION__
#ifndef __APP_INTERCONNECTION_SPP_H__
#define __APP_INTERCONNECTION_SPP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bluetooth.h"

#define SPP_RECV_BUFFER_SIZE                        3072

uint8_t* app_interconnection_ea_parse_data(uint8_t* sourceDataPtr, uint32_t* eaDataPtr);

uint16_t app_interconnection_spp_data_calculate_crc16(uint8_t *buf, uint32_t len);

struct spp_device *app_interconnection_get_spp_client();

struct spp_device *app_interconnection_get_spp_server();

typedef void(*app_spp_tx_done_t)(void);

void red_spp_register_tx_done(app_spp_tx_done_t callback);

void app_spp_client_open(uint8_t* pServiceSearchRequest, uint8_t serviceSearchRequestLen);

void app_spp_client_close(void);

void app_spp_client_close(void);

void app_spp_client_init_variable(void);

bt_status_t app_interconnection_spp_client_read_data(char * buffer, uint16_t* ptrLen);

bt_status_t app_interconnection_spp_server_read_data(char * buffer, uint16_t* ptrLen);

void app_interconnection_send_data_via_spp(uint8_t* ptrData, uint16_t length, uint8_t port);

void app_interconnection_spp_client_disconnected_callback(void);

void app_interconnection_spp_server_disconnected_callback(void);

void app_spp_client_init_variable(void);

void app_spp_server_init(void);

void app_spp_server_close(void);

void app_spp_server_init_variable(void);

void app_ibrt_spp_client_open();

#ifdef __cplusplus
}
#endif

#endif // #ifndef __APP_INTERCONNECTION_SPP_H__
#endif // #ifdef __INTERCONNECTION__