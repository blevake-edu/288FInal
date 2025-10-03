/*
 * adc.h
 *
 *  Created on: Mar 22, 2024
 *      Author: zfsalti
 */

#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

void adc_init(void);


uint16_t adc_read(void);

uint16_t adc_read_avg(void);

float ir_to_cm(unsigned short ir);

#endif /* ADC_H_ */
