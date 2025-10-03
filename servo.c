/*
 * servo.c
 *
 *  Created on: Apr 5, 2024
 *      Author: zfsalti
 */

#include "servo.h"
#include "Timer.h"

// 20 ms with 16 MHz.
#define SERVO_PERIOD_STEPS 320000


void servo_init() {
    right_match = 312500; // default right
    left_match = 284000; // default left
    // PB5
    SYSCTL_RCGCGPIO_R |= 0x2; // Port B
    while (SYSCTL_PRGPIO_R & 0x2 == 0) {}
    GPIO_PORTB_DEN_R |= 0x20; // 5th pin
    GPIO_PORTB_DIR_R |= 0x20; // output
    GPIO_PORTB_AFSEL_R |= 0x20;
    GPIO_PORTB_PCTL_R &= ~0xF00000;
    GPIO_PORTB_PCTL_R |= 0x700000;

    // Timer 1B
    SYSCTL_RCGCTIMER_R |= 0x2;
    while (SYSCTL_RCGCTIMER_R & 0x2 == 0) {}

    // PWM Mode
    TIMER1_CTL_R &= ~0x100; // disable it
    TIMER1_CFG_R = 0x4;
    TIMER1_TBMR_R &= ~0x7; // Clear bits 2, 1, 0
    TIMER1_TBMR_R |= 0x8; // Enable bit 3, TBAMS
    TIMER1_TBMR_R |= 0x2; // Set 0x2 for TBMR

    // 0x4 E200. 20 ms countdown. 320,000
    //TIMER1_TBPR_R = 0x4;
    //TIMER1_TBILR_R = 0xE200;
    TIMER1_TBPR_R = SERVO_PERIOD_STEPS >> 16;
    TIMER1_TBILR_R = (uint16_t) SERVO_PERIOD_STEPS;


    // 90 degrees
//    uint32_t match_value = 296000;
//
//    TIMER1_TBPMR_R = match_value >> 16;
//    TIMER1_TBMATCHR_R = (uint16_t) match_value;
    servo_move(0);

    TIMER1_CTL_R |= 0x100;

}

uint32_t degrees_to_match(uint16_t degrees) {
    double high_pulse_ms = (degrees / 180.0) + 1;
    uint32_t high_pulse_timer_steps = high_pulse_ms / (6.25 / 100000);
    uint32_t match_value = SERVO_PERIOD_STEPS - high_pulse_timer_steps;
    return match_value;

}


void servo_move_match(uint32_t match) {
    TIMER1_TBPMR_R = match >> 16;
    TIMER1_TBMATCHR_R = (uint16_t) match;
}

void servo_move_tuned(uint16_t degrees) {
    uint32_t range_of_match = right_match - left_match;
    uint32_t match_of_degree = right_match - (degrees * range_of_match) / 180;
    servo_move_match(match_of_degree);
}

void servo_move(uint16_t degrees) {
    uint32_t match = degrees_to_match(degrees);
    TIMER1_TBPMR_R = match >> 16;
    TIMER1_TBMATCHR_R = (uint16_t) match;
}


