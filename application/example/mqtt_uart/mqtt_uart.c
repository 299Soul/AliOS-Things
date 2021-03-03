/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "linkkit/dev_sign_api.h"
#include "linkkit/mqtt_api.h"
#include "linkkit/wrappers/wrappers.h"
#include "linkkit/infra/infra_compat.h"

#include "aos/hal/gpio.h"
#include "ulog/ulog.h"
#include "hal_iomux_haas1000.h"
#include "k_api.h"
#include "aos/cli.h"
#include "aos/kernel.h"
#include "aos/hal/uart.h"

#define PRODUCT_KEY             "a1hVZMSs1If"
#define DEVICE_NAME             "uart_test"
#define DEVICE_SECRET           "ed9bae1fe9a23d279d75e24576920090"

#define UART_BUF_SIZE   100
#define UART_RX_TIMEOUT 500
#define UART_TX_TIMEOUT 100

#define TASK_RECVDATA_NAME      "revdata"
#define TASK_RECVDATA_STACKSIZE 1024
#define TASK_RECVDATA_PRI       50

static uart_dev_t uart_demo;

void *pclient = NULL;

char DEMO_PRODUCT_KEY[IOTX_PRODUCT_KEY_LEN + 1] = {0};
char DEMO_DEVICE_NAME[IOTX_DEVICE_NAME_LEN + 1] = {0};
char DEMO_DEVICE_SECRET[IOTX_DEVICE_SECRET_LEN + 1] = {0};

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

void example_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    int res = 0;
    iotx_mqtt_topic_info_t     *topic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            /* print topic name and topic message */
            EXAMPLE_TRACE("Message Arrived:");
            EXAMPLE_TRACE("Topic  : %.*s", topic_info->topic_len, topic_info->ptopic);
            EXAMPLE_TRACE("Payload: %.*s", topic_info->payload_len, topic_info->payload);
            EXAMPLE_TRACE("\n");
            res = hal_uart_send(&uart_demo, topic_info->payload, topic_info->payload_len, UART_TX_TIMEOUT);
            if (res != 0) {
                printf("uart data send error!\n");
            }
            break;
        default:
            break;
    }
}

int example_subscribe(void *handle)
{
    int res = 0;
    const char  *fmt = "/sys/%s/%s/thing/model/down_raw";
    char *topic = NULL;
    int topic_len = 0;

    topic_len = strlen(fmt) + strlen(DEMO_PRODUCT_KEY) + strlen(DEMO_DEVICE_NAME) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, DEMO_PRODUCT_KEY, DEMO_DEVICE_NAME);

    res = IOT_MQTT_Subscribe(handle, topic, IOTX_MQTT_QOS0, example_message_arrive, NULL);
    if (res < 0) {
        EXAMPLE_TRACE("subscribe failed");
        HAL_Free(topic);
        return -1;
    }

    HAL_Free(topic);
    return 0;
}

int example_publish(void *handle, char *payload, int len)
{
    int             res = 0;
    const char     *fmt = "/sys/%s/%s/thing/model/up_raw";
    char           *topic = NULL;
    int             topic_len = 0;

    topic_len = strlen(fmt) + strlen(DEMO_PRODUCT_KEY) + strlen(DEMO_DEVICE_NAME) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, DEMO_PRODUCT_KEY, DEMO_DEVICE_NAME);

    res = IOT_MQTT_Publish_Simple(0, topic, IOTX_MQTT_QOS0, payload, len);
    if (res < 0) {
        EXAMPLE_TRACE("publish failed, res = %d", res);
        HAL_Free(topic);
        return -1;
    }

    HAL_Free(topic);
    return 0;
}

void example_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    EXAMPLE_TRACE("msg->event_type : %d", msg->event_type);
}

/*
 *  NOTE: About demo topic of /${productKey}/${deviceName}/user/get
 *
 *  The demo device has been configured in IoT console (https://iot.console.aliyun.com)
 *  so that its /${productKey}/${deviceName}/user/get can both be subscribed and published
 *
 *  We design this to completely demonstrate publish & subscribe process, in this way
 *  MQTT client can receive original packet sent by itself
 *
 *  For new devices created by yourself, pub/sub privilege also requires being granted
 *  to its /${productKey}/${deviceName}/user/get for successfully running whole example
 */

void set_iotx_info()
{
    char _product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char _device_name[IOTX_DEVICE_NAME_LEN + 1] = {0};

    HAL_GetProductKey(_product_key);
    if (strlen(_product_key) == 0) {
        HAL_SetProductKey(PRODUCT_KEY);
    }

    HAL_GetDeviceName(_device_name);
    if (strlen(_device_name) == 0) {
        HAL_SetDeviceName(DEVICE_NAME);
        HAL_SetDeviceSecret(DEVICE_SECRET);
    }
}

/* task entry */
static void task_recvdata_entry(void *arg)
{
    int  i;
    int  ret;
    int  rev_length;
    char rev_buf[UART_BUF_SIZE] = {0};

    while (1) {
        ret = hal_uart_recv_II(&uart_demo, rev_buf, UART_BUF_SIZE, &rev_length, UART_RX_TIMEOUT);
        if (ret == 0) {
            EXAMPLE_TRACE("uart rev_length:%d\n",rev_length);
            for(i = 0;i<rev_length; i++ ) {
                rev_buf[i] = 0xff & rev_buf[i];
                EXAMPLE_TRACE("uart rev_buf:0x%x\n",rev_buf[i]);
            }
            example_publish(pclient, rev_buf, rev_length);
            memset(rev_buf, 0, UART_BUF_SIZE);
        }
        aos_msleep(50);

    }
}

int rfid_uart_init(void)
{
    int ret = 0;
	/* task handle */
	aos_task_t task_recvdata;

    uart_demo.port                = 2;
    uart_demo.config.baud_rate    = 9600;
    uart_demo.config.mode         = MODE_TX_RX;
    uart_demo.config.flow_control = FLOW_CONTROL_DISABLED;
    uart_demo.config.stop_bits    = STOP_BITS_1;
    uart_demo.config.parity       = NO_PARITY;
    uart_demo.config.data_width   = DATA_WIDTH_8BIT;

    ret = hal_uart_init(&uart_demo);
    if(ret != 0) {
        EXAMPLE_TRACE("init uart error\r\n");
        return -1;
    }
    /* Create the task to receive data */
    ret = aos_task_new_ext(&task_recvdata, TASK_RECVDATA_NAME, task_recvdata_entry, NULL,
                           TASK_RECVDATA_STACKSIZE, TASK_RECVDATA_PRI);
    if (ret != 0) {
        hal_uart_finalize(&uart_demo);
        EXAMPLE_TRACE("create uart data recv task error\r\n");
        return;
    }
    return ret;
}

int linkkit_main(void *paras)
{

    int                     res = 0;
    iotx_mqtt_param_t       mqtt_params;

    set_iotx_info();

    HAL_GetProductKey(DEMO_PRODUCT_KEY);
    HAL_GetDeviceName(DEMO_DEVICE_NAME);
    HAL_GetDeviceSecret(DEMO_DEVICE_SECRET);

    EXAMPLE_TRACE("mqtt example");

    /* Initialize MQTT parameter */
    /*
     * Note:
     *
     * If you did NOT set value for members of mqtt_params, SDK will use their default values
     * If you wish to customize some parameter, just un-comment value assigning expressions below
     *
     **/
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    /**
     *
     *  MQTT connect hostname string
     *
     *  MQTT server's hostname can be customized here
     *
     *  default value is ${productKey}.iot-as-mqtt.cn-shanghai.aliyuncs.com
     */
    /* mqtt_params.host = "something.iot-as-mqtt.cn-shanghai.aliyuncs.com"; */

    /**
     *
     *  MQTT connect port number
     *
     *  TCP/TLS port which can be 443 or 1883 or 80 or etc, you can customize it here
     *
     *  default value is 1883 in TCP case, and 443 in TLS case
     */
    /* mqtt_params.port = 1883; */

    /**
     *
     * MQTT request timeout interval
     *
     * MQTT message request timeout for waiting ACK in MQTT Protocol
     *
     * default value is 2000ms.
     */
    /* mqtt_params.request_timeout_ms = 2000; */

    /**
     *
     * MQTT clean session flag
     *
     * If CleanSession is set to 0, the Server MUST resume communications with the Client based on state from
     * the current Session (as identified by the Client identifier).
     *
     * If CleanSession is set to 1, the Client and Server MUST discard any previous Session and Start a new one.
     *
     * default value is 0.
     */
    /* mqtt_params.clean_session = 0; */

    /**
     *
     * MQTT keepAlive interval
     *
     * KeepAlive is the maximum time interval that is permitted to elapse between the point at which
     * the Client finishes transmitting one Control Packet and the point it starts sending the next.
     *
     * default value is 60000.
     */
    /* mqtt_params.keepalive_interval_ms = 60000; */

    /**
     *
     * MQTT write buffer size
     *
     * Write buffer is allocated to place upstream MQTT messages, MQTT client will be limitted
     * to send packet no longer than this to Cloud
     *
     * default value is 1024.
     *
     */
    /* mqtt_params.write_buf_size = 1024; */

    /**
     *
     * MQTT read buffer size
     *
     * Write buffer is allocated to place downstream MQTT messages, MQTT client will be limitted
     * to recv packet no longer than this from Cloud
     *
     * default value is 1024.
     *
     */
    /* mqtt_params.read_buf_size = 1024; */

    /**
     *
     * MQTT event callback function
     *
     * Event callback function will be called by SDK when it want to notify user what is happening inside itself
     *
     * default value is NULL, which means PUB/SUB event won't be exposed.
     *
     */
    mqtt_params.handle_event.h_fp = example_event_handle;

    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        return -1;
    }

    res = example_subscribe(pclient);
    if (res < 0) {
        IOT_MQTT_Destroy(&pclient);
        return -1;
    }
    rfid_uart_init();
    
    while (1) {
        IOT_MQTT_Yield(pclient, 200);
    }

    return 0;
}

