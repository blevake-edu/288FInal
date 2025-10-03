/*
 * adc.c
 *
 *  Created on: Mar 22, 2024
 *      Author: zfsalti
 */

#include <inc/tm4c123gh6pm.h>
#include "adc.h"
#include <math.h>

void adc_init(void) {
    SYSCTL_RCGCGPIO_R |= 0b10; // turn on port B
    SYSCTL_RCGCADC_R = 0x1;
    while (SYSCTL_PRGPIO_R & 0b10 == 0) {}
    GPIO_PORTB_AFSEL_R |= 0x10;
    GPIO_PORTB_DIR_R &= ~0x10;
    GPIO_PORTB_DEN_R &= ~0x10; // clear for PB4
    GPIO_PORTB_AMSEL_R |= 0x10;
   // GPIO_PORTB_ADCCTL_R &= ~0xFF;

    // ADC module 0
    while (SYSCTL_PRADC_R & 0x1 == 0) {}
    //ADC0_PC_R &= ~0xF;
    //ADC0_PC_R |= 0x1;
    ADC0_SSPRI_R = 0x123;
    ADC0_ACTSS_R &= ~0x8;
    ADC0_EMUX_R &= ~0xF000;
    // sample sequencer SS3
    ADC0_SSMUX3_R &= ~0xF;
    ADC0_SSMUX3_R += 10;
    ADC0_SSCTL3_R = 0x6;
    ADC0_IM_R &= ~0x8;

    ADC0_ACTSS_R |= 0x8; // SS3 enable
   //ADC0_SAC_R = 3; //averaging doesn't work
}


uint16_t adc_read(void) {
    ADC0_PSSI_R = 0x8;
    while (ADC0_RIS_R & 0x8 == 0) {}
    uint16_t result = ADC0_SSFIFO3_R & 0xFFF;
    ADC0_ISC_R = 0x8;
    return result;
}

uint16_t adc_read_avg(void) {
    uint32_t average1 = ((uint32_t) adc_read() + (uint32_t) adc_read() + (uint32_t) adc_read() + (uint32_t) adc_read()) / 4.0;
    uint32_t average2 = ((uint32_t) adc_read() + (uint32_t) adc_read() + (uint32_t) adc_read() + (uint32_t) adc_read()) / 4.0;
    uint32_t average3 = ((uint32_t) adc_read() + (uint32_t) adc_read() + (uint32_t) adc_read() + (uint32_t) adc_read()) / 4.0;
    uint32_t average4 = ((uint32_t) adc_read() + (uint32_t) adc_read() + (uint32_t) adc_read() + (uint32_t) adc_read()) / 4.0;
    return (average1 + average2 + average3 + average4) / 4.0;
}


float ir_to_cm(unsigned short ir) {
    //return 50000000.0 * pow(ir, -1.958); // 5, 4
    //return 20000000.0 * pow(ir, -1.813); // 8
    //return 4000000.0 * pow(ir, -1.621); // Upstairs 4
    //return 100000000 * pow(ir, -2.193);
    //return 30000000 * pow(ir, -2.018); // Upstairs 2
    return 20000000 * pow(ir, -1.825); // Upstairs 11
}
