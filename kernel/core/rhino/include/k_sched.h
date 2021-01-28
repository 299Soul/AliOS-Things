/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef K_SCHED_H
#define K_SCHED_H

#define KSCHED_FIFO          0u
#define KSCHED_RR            1u
#define KSCHED_CFS           2u

#define SCHED_MAX_LOCK_COUNT 200u
#define NUM_WORDS            ((RHINO_CONFIG_PRI_MAX + 31) / 32)

typedef struct {
    klist_t  *cur_list_item[RHINO_CONFIG_PRI_MAX];
    uint32_t  task_bit_map[NUM_WORDS];
    uint8_t   highest_pri;
} runqueue_t;

/**
 * This function will disable schedule
 * @return the operation status, RHINO_SUCCESS is OK, others is error
 */
kstat_t krhino_sched_disable(void);

/**
 * This function will enable schedule
 * @return the operation status, RHINO_SUCCESS is OK, others is error
 */
kstat_t krhino_sched_enable(void);

/**
 * Get the maximum schedule priority that can be set
 * @param[in]  policy  schedule policy
 * @return >=0: the returned priority. otherwise, error.
 */
int krhino_sched_get_priority_max(int policy);

/**
 * Get the minimum schedule priority that can be set
 *
 * @param[in]  policy  schedule policy
 * @return >=0: the returned priority. otherwise, error.
 */
int krhino_sched_get_priority_min(int policy);

#endif /* K_SCHED_H */

