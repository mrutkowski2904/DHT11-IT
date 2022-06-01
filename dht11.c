#include "dht11.h"

#include <string.h>

uint8_t DHT11_Start_Init(DHT11_TypeDef *dht) {
	GPIO_InitTypeDef dht_pin = { 0 };
	dht_pin.Pin = dht->Pin;
	dht_pin.Pull = GPIO_NOPULL;

	dht->index = 0;
	dht->recieved_bits = 0;
	dht->state = DHT11_STATE_INIT;

	memset(dht->data, 0, 5);

	dht_pin.Mode = GPIO_MODE_OUTPUT_OD;
	HAL_GPIO_Init(dht->Port, &dht_pin);

	// Init start sequence
	HAL_GPIO_WritePin(dht->Port, dht->Pin, GPIO_PIN_RESET);
	return dht->state;
}

void DHT11_Start_End(DHT11_TypeDef *dht) {
	if (dht->state == DHT11_STATE_INIT) {
		// finish init sequence
		HAL_GPIO_WritePin(dht->Port, dht->Pin, GPIO_PIN_SET);

		// configure pin for interrupt
		GPIO_InitTypeDef dht_pin = { 0 };
		dht_pin.Pin = dht->Pin;
		dht_pin.Pull = GPIO_NOPULL;
		dht_pin.Mode = GPIO_MODE_IT_RISING_FALLING;
		HAL_GPIO_Init(dht->Port, &dht_pin);

		dht->state = DHT11_STATE_ACK;
		dht->recieved_bits = 0;

		HAL_TIM_Base_Start(dht->usTimerHandle);
		__HAL_TIM_SET_COUNTER(dht->usTimerHandle, 0U);
	}
}

__weak void DHT11_RecieveCpltCallback(DHT11_TypeDef *dht) {
	/*
	 * Copy this callback to your code, remove __weak in copied callback
	 */
}

void DHT11_IRQHandler(DHT11_TypeDef *dht) {
	dht->last_recieved_at = __HAL_TIM_GET_COUNTER(dht->usTimerHandle);
	__HAL_TIM_SET_COUNTER(dht->usTimerHandle, 0U);

	if (HAL_GPIO_ReadPin(dht->Port, dht->Pin) == 0) {
		// falling edge
		dht->recieved_bits++;

		if (dht->recieved_bits == 2 && dht->state == DHT11_STATE_ACK) {
			dht->recieved_bits = 0;
			dht->state = DHT11_STATE_BUSY;
			return;
		}

		if (dht->state == DHT11_STATE_BUSY) {
			if (dht->recieved_bits == 8) {
				dht->index++;
				dht->recieved_bits = 0;
			}

			if (dht->last_recieved_at > 50 && dht->index <= 4) {
				// recieved 1
				*(dht->data + dht->index) |= (1 << (7 - dht->recieved_bits));
			}

			if (dht->index == 4 && dht->recieved_bits == 7) {
				HAL_TIM_Base_Stop(dht->usTimerHandle);
				dht->state = DHT11_STATE_FINISHED;
				DHT11_RecieveCpltCallback(dht);
			}

		}
	}
}
