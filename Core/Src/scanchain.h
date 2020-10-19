/*
 * scanchain.h
 *
 *  Created on: Oct 18, 2020
 *      Author: jake
 */

#ifndef SRC_SCANCHAIN_H_
#define SRC_SCANCHAIN_H_

#include "main.h"
#include <stdint.h>

struct scanchain_pindef {
	GPIO_TypeDef * clrn_port;
	GPIO_TypeDef * pln_port;
	GPIO_TypeDef * lat_port;
	uint16_t clrn_pin;
	uint16_t pln_pin;
	uint16_t lat_pin;
	SPI_HandleTypeDef * hspi;
	int chain_length;
};

int scanchain_init(const struct scanchain_pindef *);
int scanchain_init_all(const struct scanchain_pindef[]);
int scanchain_scan(const struct scanchain_pindef *, uint8_t * buf);

#endif /* SRC_SCANCHAIN_H_ */
