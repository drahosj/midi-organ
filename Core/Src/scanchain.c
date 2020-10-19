/*
 * scanchain.c
 *
 *  Created on: Oct 18, 2020
 *      Author: jake
 */
#include "scanchain.h"

int scanchain_init(const struct scanchain_pindef * pindef)
{
	/* Hold clear low and set all pins to idle state */
	HAL_GPIO_WritePin(pindef->clrn_port, pindef->clrn_pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(pindef->pln_port, pindef->pln_pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(pindef->lat_port, pindef->lat_pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(pindef->clrn_port, pindef->clrn_pin, GPIO_PIN_SET);

	return 0;
}

int scanchain_init_all(const struct scanchain_pindef pindefs[])
{
	int i;
	for (i = 0; pindefs[i].clrn_port != NULL; i++) {
		scanchain_init(&pindefs[i]);
	}
	return i;
}

int scanchain_scan(const struct scanchain_pindef * pindef, uint8_t * buf)
{
	uint8_t dummybuf[1];

	/* Latch inputs */
	HAL_GPIO_WritePin(pindef->lat_port, pindef->lat_pin, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(pindef->lat_port, pindef->lat_pin, GPIO_PIN_RESET);
	HAL_Delay(1);

	/* Parallel load */
	HAL_GPIO_WritePin(pindef->pln_port, pindef->pln_pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	/* Easiest way to clock the clk at least once while PLN is asserted */
	HAL_SPI_Receive(pindef->hspi, dummybuf, 1, 100);
	HAL_GPIO_WritePin(pindef->pln_port, pindef->pln_pin, GPIO_PIN_SET);
	HAL_Delay(1);

	return HAL_SPI_Receive(pindef->hspi, buf, pindef->chain_length, 1000);
}
