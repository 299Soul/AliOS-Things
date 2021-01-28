/*
 * Copyright (C) 2015-2021 Alibaba Group Holding Limited
 */
#ifndef APP_MESH_PROV_H_
#define APP_MESH_PROV_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief Mesh Proxy Application entry point.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_MESH_PROV)

#include <stdint.h>          // Standard Integer Definition
#include "ke_task.h"

#define BLE_INVALID_CONNECTION_INDEX    0xFF

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


extern struct app_env_tag app_env;

/// health thermometer application environment structure
struct app_mesh_prov_env_tag
{
    uint8_t connectionIndex;
    uint8_t isNotificationEnabled;
    uint16_t mtu;
};

typedef void(*app_mesh_prov_tx_done_t)(void);

typedef void(*app_mesh_prov_activity_stopped_t)(void);

/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

/// Health Thermomter Application environment
extern struct app_mesh_prov_env_tag app_mesh_prov_env;

/// Table of message handlers
extern const struct ke_state_handler app_mesh_prov_table_handler;

/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Initialize DataPath Server Application
 ****************************************************************************************
 */
void app_mesh_prov_init(void);

/**
 ****************************************************************************************
 * @brief Add a DataPath Server instance in the DB
 ****************************************************************************************
 */
void app_mesh_prov_add(void);

void app_mesh_prov_connected_evt_handler(uint8_t conidx);

void app_mesh_prov_disconnected_evt_handler(uint8_t conidx, uint8_t reason);

void app_mesh_prov_send_data_via_notification(uint8_t conidx, uint8_t* ptrData, uint32_t length);

void app_mesh_prov_send_data_via_write_command(uint8_t* ptrData, uint32_t length);

void app_mesh_prov_send_data_via_write_request(uint8_t* ptrData, uint32_t length);

void app_mesh_prov_register_tx_done(app_mesh_prov_tx_done_t callback);

void app_mesh_prov_update_conn_parameter(uint8_t conidx, uint32_t min_interval_in_ms, uint32_t max_interval_in_ms,
        uint32_t supervision_timeout_in_ms, uint8_t slaveLantency);

void app_mesh_prov_control_notification(bool isEnable);

void app_mesh_prov_mtu_exchanged_handler(uint8_t conidx, uint16_t mtu);


#endif //(BLE_APP_MESH_PROV)

/// @} APP

#endif // APP_MESH_PROV_H_
