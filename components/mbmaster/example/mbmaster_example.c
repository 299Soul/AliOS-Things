/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "aos/kernel.h"
#include "mbmaster.h"
#include "ulog/ulog.h"
#ifdef AOS_COMP_CLI
#include "aos/cli.h"
#endif

/* Define the parameters */
#define SERIAL_PORT         2    /* uart port */
#define SERIAL_BAUD_RATE    9600 /* uart baud rate */
#define DEVICE1_SLAVE_ADDR  0x1  /* remote deveice address */
#define DEVICE1_REG1_ADDR   0x0  /* register address of remote device */
#define RECV_LEN_MAX        20   /* buffer len, must >=  (REQ_REGISTER_NUMBER * 2) */
#define REQ_REGISTER_NUMBER 10    /* Number of registers requested */

/* mode name used by ulog */
#define MODBUSM_APP "modbus_app"

void mb_main(void)
{
    uint8_t     buf[RECV_LEN_MAX];
    uint8_t     len;
    mb_status_t status;
    uint16_t    simulator1 = 0, simulator2 = 0;
    uint16_t    data_write = 0, data_resp = 0;
    uint16_t   *register_buf;

    /* The handler is allocated by calling the aos_mbmaster_rtu_init */
    mb_handler_t *mb_handler;

    /**
     * Initialize the modbus communication port, using rtu mode.
     * Need to set the appropriatet port number, baud rate, parity
     * according to the actual situation.
     */
    status = mbmaster_rtu_init(&mb_handler, SERIAL_PORT, SERIAL_BAUD_RATE, MB_PAR_NONE);
    LOGD(MODBUS_MOUDLE, "mbmaster_rtu_init status is %d", status);
    if (status != MB_SUCCESS) {
        LOGE(MODBUSM_APP, "mbmaster init error");
        return;
    }

    /* note: This example loops every 2 seconds, first sending a write request, then sending a read request. */
    while (1) {
        /**
         * Initiate write single registers request.
         * data_resp: respond from the remote device. If the write operation succeeds,
         * it's equal to data_write.
         */
        status = mbmaster_write_single_register(mb_handler, DEVICE1_SLAVE_ADDR, DEVICE1_REG1_ADDR,
                                                data_write, NULL, &data_resp, NULL, AOS_WAIT_FOREVER);
        if (status == MB_SUCCESS) {
            if (data_write != data_resp) {
                LOGE(MODBUSM_APP, "write single register error");
            } else {
                LOGI(MODBUSM_APP, "write single register ok");
            }
        } else {
            LOGE(MODBUSM_APP, "write single register error");
        }

        data_write++; /* generate a new value */

        aos_msleep(1000);

        memset(buf, 0, RECV_LEN_MAX);

        /**
         * Initiate read holding registers request
         * The buf length needs to be greater than or equal to (REQ_REGISTER_NUMBER * 2)
         */
        status = mbmaster_read_holding_registers(mb_handler, DEVICE1_SLAVE_ADDR, DEVICE1_REG1_ADDR,
                                                 REQ_REGISTER_NUMBER, buf, &len, AOS_WAIT_FOREVER);
        if (status == MB_SUCCESS) {
            /* The register length on modbus is 16 bits */
            register_buf = (uint16_t *)buf;
            simulator1 = register_buf[0];
            simulator2 = register_buf[1];
            LOGI(MODBUSM_APP, "read holding register simulator1: %d,simulator2: %d", simulator1, simulator2);
        } else {
            LOGE(MODBUSM_APP, "read holding register error");
        }

        aos_msleep(1000);
    }
}

int mbmaster_comp_example(int argc, char *argv[])
{
    aos_set_log_level(AOS_LL_DEBUG);

    LOGI(MODBUSM_APP, "mbmaster test");

    mb_main();

    return 0;
}

#ifdef AOS_COMP_CLI
/* reg args: fun, cmd, description*/
ALIOS_CLI_CMD_REGISTER(mbmaster_comp_example, mbmaster_example, mbmaster component base example)
#endif

