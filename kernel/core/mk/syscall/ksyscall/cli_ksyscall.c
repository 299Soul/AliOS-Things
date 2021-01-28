/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include <stdint.h>
#include "k_config.h"
#include "aos/cli.h"
#include "cli_syscall_arg.h"
#include "cli_console.h"

int32_t sys_cli_register_command_stub(void *arg)
{
    cli_register_command_syscall_arg_t *_arg = arg;

    return aos_cli_register_command(_arg->cmd);
}

int32_t sys_cli_unregister_command_stub(void *arg)
{
    cli_unregister_command_syscall_arg_t *_arg = arg;

    return aos_cli_unregister_command(_arg->cmd);
}

int32_t sys_cli_register_commands_stub(void *arg)
{
    cli_register_commands_syscall_arg_t *_arg = arg;
    const struct cli_command         *cmd;

    int i;
    int ret;

    cmd = _arg->cmd;
    for (i = 0; i < _arg->num_cmds; i++) {
        ret = aos_cli_register_command(cmd);
        if (0 != ret) {
            return ret;
        }
        cmd++;
    }

    return 0;
}

int32_t sys_cli_unregister_commands_stub(void *arg)
{
    cli_unregister_commands_syscall_arg_t *_arg = arg;
    const struct cli_command           *cmd;

    int i;
    int ret;

    cmd = _arg->cmd;
    for (i = 0; i < _arg->num_cmds; i++) {
        ret = aos_cli_unregister_command(cmd);
        if (0 != ret) {
            return ret;
        }
        cmd++;
    }

    return 0;
}

int32_t sys_cli_task_set_console_stub(void *arg)
{
    cli_task_set_console_syscall_arg_t *_arg = arg;

    return aos_cli_task_set_console(_arg->task_handle, _arg->console);
}
