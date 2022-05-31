/*
 * dht11.h
 *
 *  The MIT License.
 *  Created on: 28.05.2022
 *      Author: Maciej Rutkowski
 */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include "stm32l0xx.h"

#define DHT11_STATE_BUSY		0x00
#define DHT11_STATE_READY		0x01
#define DHT11_STATE_ACK			0x03
#define DHT11_STATE_INIT		0x04
#define DHT11_STATE_FINISHED	0x05

typedef struct {
	/* ---------- CONFIGURABLE ---------- */
	uint16_t Pin;
	GPIO_TypeDef *Port;
	TIM_HandleTypeDef *usTimerHandle;

	// 5 byte array
	uint8_t *data;

	/* ---------- INTERNAL DO NOT MODIFY ---------- */
	uint8_t state;
	uint8_t recieved_bits;
	uint32_t last_recieved_at;
	uint8_t index;
	uint8_t *currently_recieving;

} DHT11_TypeDef;

/*
 * Call Start_Init and Start_End ~18ms apart
 * (RTOS delay or HAL tick may be used)
 */
uint8_t DHT11_Start_Init(DHT11_TypeDef *dht);
void DHT11_Start_End(DHT11_TypeDef *dht);

void DHT11_RecieveCpltCallback(DHT11_TypeDef *dht);

/*
 * Put in EXTI GPIO handler/callback
 */
void DHT11_IRQHandler(DHT11_TypeDef *dht);

#endif /* INC_DHT11_H_ */
