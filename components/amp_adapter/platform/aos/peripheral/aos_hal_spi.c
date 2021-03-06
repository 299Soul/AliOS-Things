/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <aos/errno.h>
#ifndef AOS_BOARD_HAAS700
#include <vfsdev/spi_dev.h>
#endif
#include "aos/hal/spi.h"
#include "aos_hal_spi.h"

int32_t aos_hal_spi_init(spi_dev_t *spi)
{
#ifndef AOS_BOARD_HAAS700
    return hal_spi_init(spi);
#else
    return -1;
#endif
}

int32_t aos_hal_spi_send(spi_dev_t *spi, const uint8_t *data, uint32_t size, uint32_t timeout)
{
#ifndef AOS_BOARD_HAAS700
    return hal_spi_send(spi, data, size, timeout);
#else
    return -1;
#endif
}

int32_t aos_hal_spi_recv(spi_dev_t *spi, uint8_t *data, uint32_t size, uint32_t timeout)
{
#ifndef AOS_BOARD_HAAS700
    return hal_spi_recv(spi, data, size, timeout);
#else
    return -1;
#endif
}

int32_t aos_hal_spi_send_recv(spi_dev_t *spi, uint8_t *tx_data, uint8_t *rx_data,
                          uint32_t rx_size, uint32_t timeout)
{
#ifndef AOS_BOARD_HAAS700
    return hal_spi_send_recv(spi, tx_data, rx_data, rx_size, timeout);
#else
    return -1;
#endif
}

int32_t aos_hal_spi_sends_recvs(spi_dev_t *spi, uint8_t *tx_data, uint32_t tx_size,
                            uint8_t *rx_data, uint32_t rx_size, uint32_t timeout)
{
    return hal_spi_sends_recvs(spi, tx_data, tx_size, rx_data, rx_size, timeout);
}

int32_t aos_hal_spi_send_and_recv(spi_dev_t *spi, uint8_t *tx_data, uint16_t tx_size, uint8_t *rx_data,
                                  uint16_t rx_size, uint32_t timeout)
{
#ifndef AOS_BOARD_HAAS700
    return hal_spi_send_and_recv(spi, tx_data, tx_size, rx_data, rx_size, timeout);
#else
    return -1;
#endif
}

int32_t aos_hal_spi_finalize(spi_dev_t *spi)
{
#ifndef AOS_BOARD_HAAS700
    return hal_spi_finalize(spi);
#else
    return -1;
#endif
}


