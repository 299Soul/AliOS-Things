#include <stdio.h>
#include "aos/kernel.h"
#include <k_api.h>
#include <stdint.h>
#include <stdbool.h>
#include "board.h"
#include "aos/hal/spi.h"
#include "spi_api.h"
#include "spi_ex_api.h"
#include "pinmap.h"

/*SPI pin location:
* SPI0:
*   - S0:  PA_16(MOSI)/PA_17(MISO)/PA_18(SCLK)/PA_19(CS).
*   - S1:  PB_4(MOSI)/PB_5(MISO)/PB_6(SCLK)/PB_7(CS).
*/

spi_t spi_a;
spi_t spi_b;

typedef enum{
    SPI_NUM_0 = 0,  /*!< SPI port 0 */
    SPI_NUM_1 ,     /*!< SPI port 1 */
    SPI_NUM_MAX
} spi_port_t;

typedef struct spi_pin{
    spi_t * dev;
    PinName spi_mosi;
    PinName spi_miso;
    PinName spi_sclk;
    PinName spi_cs;
} spi_pin_t;

static spi_pin_t spi_dev[SPI_NUM_MAX] = {
    {&spi_a, PA_16, PA_17, PA_18, PA_19},
    {&spi_b, PB_4, PB_5, PB_6, PB_7}
};

#define HAL_WAIT_FOREVER 0xFFFFFFFFU

#define HAL_SPI_MODE_MASTER 1 /* spi communication is master mode */
#define HAL_SPI_MODE_SLAVE  2 /* spi communication is slave mode */


aos_sem_t slave_tx_down_sema;
aos_sem_t slave_rx_down_sema;
aos_sem_t master_tx_down_sema;
aos_sem_t master_rx_down_sema;


void Master_tr_done_callback(void *pdata, SpiIrq event)
{
        switch(event){
                case SpiRxIrq:
                        aos_sem_signal(&master_rx_down_sema);
                        break;
                case SpiTxIrq:
                        aos_sem_signal(&master_tx_down_sema);
			break;
                default:
                        DBG_8195A("unknown interrput evnent!\n");
        }
}

void Slave_tr_done_callback(void *pdata, SpiIrq event)
{
        switch(event){
                case SpiRxIrq:
                        aos_sem_signal(&slave_rx_down_sema); 
			break;
                case SpiTxIrq:
			aos_sem_signal(&slave_tx_down_sema);
                        break;
                default:
                        DBG_8195A("unknown interrput evnent!\n");
        }
}

int32_t hal_spi_init(spi_dev_t *spi)
{
	int port = spi->port;
	int spi_slave;

	aos_sem_new(&slave_tx_down_sema, 0);
	aos_sem_new(&slave_rx_down_sema, 0);
	aos_sem_new(&master_tx_down_sema, 0);
	aos_sem_new(&master_rx_down_sema, 0);

	spi_a.spi_idx = MBED_SPI0;
	spi_b.spi_idx = MBED_SPI1;


	if(spi->config.mode == HAL_SPI_MODE_MASTER)
		spi_slave = 0;
	else if(spi->config.mode == HAL_SPI_MODE_SLAVE)
		spi_slave = 1;
	else
		printf("ERROR: SPI Config Mode Set ERROR = %d", spi->config.mode);

	spi_init(spi_dev[port].dev, spi_dev[port].spi_mosi, 
			spi_dev[port].spi_miso,
		       	spi_dev[port].spi_sclk, spi_dev[port].spi_cs);
	spi_format(spi_dev[port].dev, 8, 0, spi_slave);
	spi_frequency(spi_dev[port].dev, spi->config.freq);
	return 0;
}

int32_t hal_spi_send(spi_dev_t *spi, const uint8_t *data, uint16_t size, uint32_t timeout)
{
        int spi_slave;
	int port = spi->port;

        if(spi->config.mode == HAL_SPI_MODE_MASTER)
                spi_slave = 0;
        else if(spi->config.mode == HAL_SPI_MODE_SLAVE)
                spi_slave = 1;
        else
                printf("ERROR: SPI Config Mode Set ERROR = %d", spi->config.mode);

	if(spi_slave){
                spi_irq_hook(spi_dev[port].dev,(spi_irq_handler) Slave_tr_done_callback, (uint32_t)spi_dev[port].dev);
		spi_slave_write_stream(spi_dev[port].dev, (uint8_t *)data, size);
		aos_sem_wait(&slave_tx_down_sema, timeout);
	}else{
	        spi_irq_hook(spi_dev[port].dev,(spi_irq_handler) Master_tr_done_callback, (uint32_t)spi_dev[port].dev);
		spi_master_write_stream(spi_dev[port].dev, (uint8_t *)data, size);
		aos_sem_wait(&master_tx_down_sema, timeout);
	}
}

/**
 * spi_recv
 *
 * @param[in]   spi      the spi device
 * @param[out]  data     spi recv data
 * @param[in]   size     spi recv data size
 * @param[in]  timeout   timeout in milisecond, set this value to HAL_WAIT_FOREVER
 *                       if you want to wait forever
 *
 * @return  0 : on success, EIO : if the SPI device could not be initialised
 */
int32_t hal_spi_recv(spi_dev_t *spi, uint8_t *data, uint16_t size, uint32_t timeout)
{
        int spi_slave;
	int port = spi->port;

        if(spi->config.mode == HAL_SPI_MODE_MASTER)
                spi_slave = 0;
        else if(spi->config.mode == HAL_SPI_MODE_SLAVE)
                spi_slave = 1;
        else
                printf("ERROR: SPI Config Mode Set ERROR = %d", spi->config.mode);

        if(spi_slave){
                spi_irq_hook(spi_dev[port].dev,(spi_irq_handler) Slave_tr_done_callback, (uint32_t)spi_dev[port].dev);
		spi_slave_read_stream(spi_dev[port].dev, data, size);
                aos_sem_wait(&slave_rx_down_sema, timeout);
	}else{
                spi_irq_hook(spi_dev[port].dev,(spi_irq_handler) Master_tr_done_callback, (uint32_t)spi_dev[port].dev);
		spi_master_read_stream(spi_dev[port].dev, data, size);
                aos_sem_wait(&master_rx_down_sema, timeout);
        }
}

/**
 * spi send data and recv
 *
 * @param[in]  spi      the spi device
 * @param[in]  tx_data  spi send data
 * @param[in]  rx_data  spi recv data
 * @param[in]  size     spi data to be sent and recived
 * @param[in]  timeout  timeout in milisecond, set this value to HAL_WAIT_FOREVER
 *                      if you want to wait forever
 *
 * @return  0, on success;  EIO : if the SPI device could not be initialised
 */
int32_t hal_spi_send_recv(spi_dev_t *spi, uint8_t *tx_data, uint8_t *rx_data,
                          uint16_t size, uint32_t timeout)
{
        int spi_slave = 0;
	int port = spi->port;

        if(spi->config.mode != HAL_SPI_MODE_MASTER)
                printf("ERROR: Only support SPI Master Mode Send and RECV \n\r");

        spi_irq_hook(spi_dev[port].dev,(spi_irq_handler) Master_tr_done_callback, (uint32_t)spi_dev[port].dev);
	spi_master_write_read_stream(spi_dev[port].dev, tx_data, rx_data, size);
        aos_sem_wait(&master_rx_down_sema, timeout);
}

int32_t hal_spi_finalize(spi_dev_t *spi)
{
	spi_free(spi_dev[spi->port].dev);
	aos_sem_free(&slave_tx_down_sema);
        aos_sem_free(&slave_rx_down_sema);
        aos_sem_free(&master_tx_down_sema);
        aos_sem_free(&master_rx_down_sema);

}



int32_t hal_spi_send_and_recv(spi_dev_t *spi, uint8_t *tx_data, uint16_t tx_size, uint8_t *rx_data,
                              uint16_t rx_size, uint32_t timeout)
{
        return -1;
}


 int32_t hal_spi_send_and_send(spi_dev_t *spi, uint8_t *tx1_data, uint16_t tx1_size, uint8_t *tx2_data,
                               uint16_t tx2_size, uint32_t timeout)
{
        return -1;
}