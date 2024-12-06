#include <stdint.h>

/*
 * SPI_SLAVE GPIO Configuration  
 * PB11/AF3   ------> SPI_SCK
 * PB8/AF3    ------> SPI_MISO
 * PB9/AF3    ------> SPI_MOSI  
 */
#define BINARYSAMPLING_GPIO_PORT_SLAVE_SCK                    GPIOB
#define BINARYSAMPLING_GPIO_PORT_SLAVE_MISO                   GPIOB
#define BINARYSAMPLING_GPIO_PORT_SLAVE_NSS                    GPIOA
#define BINARYSAMPLING_GPIO_PIN_SPI_SLAVE_SCK                 LL_GPIO_PIN_11
#define BINARYSAMPLING_GPIO_PIN_SPI_SLAVE_MISO                LL_GPIO_PIN_8
#define BINARYSAMPLING_GPIO_PIN_SPI_SLAVE_NSS                 LL_GPIO_PIN_12
#define BINARYSAMPLING_GPIO_AF_SPI_SLAVE_SCK                  LL_GPIO_AF_3
#define BINARYSAMPLING_GPIO_AF_SPI_SLAVE_MISO                 LL_GPIO_AF_3
#define BINARYSAMPLING_GPIO_AF_SPI_SLAVE_NSS                  LL_GPIO_AF_3
#define BINARYSAMPLING_SPI_SLAVE                              SPI1
#define BINARYSAMPLING_LL_SPI_Slave_EnableClock()             LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI1)
#define BINARYSAMPLING_SPI_SLAVE_IRQn                         SPI1_IRQn
#define BINARYSAMPLING_SPI_SLAVE_IRQHandler                   SPI1_IRQHandler
#define BINARYSAMPLING_LL_DMAMUX_REQ_SPI_SLAVE_TX             LL_DMAMUX_REQ_SPI1_TX

#ifndef _BINARYSAMPLING_SPI_H
#define _BINARYSAMPLING_SPI_H

void BinaryIQSampling_SPI_Init(void);
void BinaryIQSampling_SPI_Transmit(uint8_t* buffer, uint32_t length);
uint8_t BinaryIQSampling_SPI_TXIsComplete(void);
void BinaryIQSampling_SPI_Finalize(void);
void BinaryIQSampling_SPI_DeInit(void);

void DMA_IRQHandler(void);

#endif