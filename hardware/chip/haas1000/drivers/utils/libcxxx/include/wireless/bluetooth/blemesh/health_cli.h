/** @file
 *  @brief Bluetooth Mesh Health Client Model APIs.
 */

/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __BT_MESH_HEALTH_CLI_H
#define __BT_MESH_HEALTH_CLI_H

/** Mesh Health Client Model Context */
struct bt_mesh_health_cli {
	struct bt_mesh_model *model;

	void (*current_status)(struct bt_mesh_health_cli *cli, u16_t addr,
			       u8_t test_id, u16_t cid, u8_t *faults,
			       size_t fault_count);

	struct k_sem          op_sync;
	u32_t                 op_pending;
	void                 *op_param;
};

extern const struct bt_mesh_model_op bt_mesh_health_cli_op[];

#define BT_MESH_MODEL_HEALTH_CLI(cli_data)                                   \
		BT_MESH_MODEL(BT_MESH_MODEL_ID_HEALTH_CLI,                   \
			      bt_mesh_health_cli_op, NULL, cli_data)

#endif /* __BT_MESH_HEALTH_CLI_H */
