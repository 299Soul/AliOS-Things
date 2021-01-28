/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include "k_api.h"
#include "task_group.h"
#include "mksh.h"
#include "k_proc.h"

#ifdef AOS_COMP_IRQ
void aos_irq_exit_bypid(uint32_t pid);
#endif

static klist_t task_group_list_head = {
    &task_group_list_head,
    &task_group_list_head
};

extern int ucli_init_sh(task_group_t *group);

static void task_group_list_add(task_group_t *group)
{
    CPSR_ALLOC();

    RHINO_CRITICAL_ENTER();
    klist_insert(&task_group_list_head, &group->node);
    RHINO_CRITICAL_EXIT();
}

static void task_group_list_rm(task_group_t *group)
{
    CPSR_ALLOC();

    RHINO_CRITICAL_ENTER();
    klist_rm(&group->node);
    RHINO_CRITICAL_EXIT();
}

klist_t *task_group_get_list_head(void)
{
    return &task_group_list_head;
}

task_group_t *task_group_get_by_pid(uint32_t pid)
{
    klist_t *head;
    klist_t *list_iter;

    task_group_t *group_iter;
    task_group_t *group;

    CPSR_ALLOC();

    group = NULL;
    head  = task_group_get_list_head();

    RHINO_CRITICAL_ENTER();

    list_iter  = head->next;
    for (; list_iter != head; list_iter = list_iter->next) {
        group_iter = group_info_entry(list_iter, task_group_t, node);
        if (group_iter && (group_iter->pid == pid)) {
            group = group_iter;
            break;
        }
    }

    RHINO_CRITICAL_EXIT();

    return group;
}

int task_group_init(task_group_t *group, const char *name, int pid)
{
    int ret;
    int len;

    /*memset(group, 0, sizeof(task_group_t));*/

    group->pid = pid;

    len = strlen((const char *)name);
    if (len >= TG_NAME_MAX_LEN) {
        len = TG_NAME_MAX_LEN - 1;
    }

    memcpy(group->tg_name, (const char *)name, len);

    group->state = TGS_NORMAL;

    /* process kobj init */
    klist_init(&(group->kobj_list.task_head));
    klist_init(&(group->kobj_list.mutex_head));
#if (RHINO_CONFIG_SEM > 0)
    klist_init(&(group->kobj_list.sem_head));
#endif
#if (RHINO_CONFIG_QUEUE > 0)
    klist_init(&(group->kobj_list.queue_head));
#endif
#if (RHINO_CONFIG_BUF_QUEUE > 0)
    klist_init(&(group->kobj_list.buf_queue_head));
#endif
#if (RHINO_CONFIG_EVENT_FLAG > 0)
    klist_init(&(group->kobj_list.event_head));
#endif

    task_group_list_add(group);

    ret = fsfd_init_sh(group);
    if (ret != 0) {
        goto rm_group;
    }

    ret = socketfd_init_sh(group);
    if (ret != 0) {
        goto exit_fsfd;
    }

    ret = res_init_sh(group);
    if (ret != 0) {
        goto exit_socketfd;
    }

    ret = cb_init_sh(group);
    if (ret != 0) {
        goto exit_res;
    }

    ret = ucli_init_sh(group);
    if (ret != 0) {
        goto exit_cb;
    }

    return 0;

exit_cb:
    cb_exit_sh(group);
exit_res:
    res_exit_sh(group);
exit_socketfd:
    socketfd_exit_sh(group);
exit_fsfd:
    fsfd_exit_sh(group);
rm_group:
    task_group_list_rm(group);

    return ret;
}

static void task_group_release(task_group_t *group)
{
    ucli_exit_sh(group);

    fsfd_exit_sh(group);

    socketfd_exit_sh(group);

    res_exit_sh(group);

    cb_exit_sh(group);

    msg_exit_sh(group->pid);
}

static void task_group_destory(task_group_t *group)
{
    task_group_list_rm(group);
    krhino_mm_free(group);
}

static void free_user_res(task_group_t *group)
{
    kmutex_t     *mutex;
    klist_t      *head, *iter;

    /*mutex res exit*/
    head = &(group->kobj_list.mutex_head);
    iter = head->next;

    while (iter != head) {
        mutex = krhino_list_entry(iter, kmutex_t, blk_obj.obj_list);
        iter = iter->next;
        krhino_mutex_dyn_del(mutex);
    }

    /*sem res exit*/
#if (RHINO_CONFIG_SEM > 0)
    ksem_t       *sem;
    head = &(group->kobj_list.sem_head);
    iter = head->next;

    while (iter != head) {
        sem = krhino_list_entry(iter, ksem_t, blk_obj.obj_list);
        iter = iter->next;
        krhino_sem_dyn_del(sem);
    }
#endif

    /*queue res exit*/
#if (RHINO_CONFIG_QUEUE > 0)
    kqueue_t     *queue;
    head = &(group->kobj_list.queue_head);
    iter = head->next;

    while (iter != head) {
        queue = krhino_list_entry(iter, kqueue_t, blk_obj.obj_list);
        iter = iter->next;
        krhino_queue_dyn_del(queue);
    }
#endif

    /*buf queue res exit*/
#if (RHINO_CONFIG_BUF_QUEUE > 0)
    kbuf_queue_t *buf_queue;
    head = &(group->kobj_list.buf_queue_head);
    iter = head->next;

    while (iter != head) {
        buf_queue = krhino_list_entry(iter, kbuf_queue_t, blk_obj.obj_list);
        iter = iter->next;
        krhino_buf_queue_dyn_del(buf_queue);
    }
#endif

    /* event res exit*/
#if (RHINO_CONFIG_EVENT_FLAG > 0)
    kevent_t     *event;
    head = &(group->kobj_list.event_head);
    iter = head->next;

    while (iter != head) {
        event = krhino_list_entry(iter, kevent_t, blk_obj.obj_list);
        iter = iter->next;
        krhino_event_dyn_del(event);
    }
#endif
}

void task_group_proc_free(task_group_t *group)
{
    int pid;

    pid = group->pid;
    #ifdef AOS_COMP_IRQ
    aos_irq_exit_bypid(pid);
    #endif
    free_user_res(group);
    task_group_release(group);
    task_group_destory(group);
    k_proc_unload(pid);

    printf("process %d is unloaded successfully\r\n", pid);
}

