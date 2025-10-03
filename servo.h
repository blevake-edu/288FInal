/*
 * servo.h
 *
 *  Created on: Apr 5, 2024
 *      Author: zfsalti
 */

#ifndef SERVO_H_TXT_
#define SERVO_H_TXT_

#include <stdint.h>

void servo_init();

void servo_move(uint16_t degrees);

void servo_move_match(uint32_t match);

void servo_move_tuned(uint16_t degrees);

uint32_t degrees_to_match(uint16_t degrees);

/**
 * value, if wanted to be calibrated, must be set after calling servo_init().
 */
uint32_t right_match;
/**
 * value, if wanted to be calibrated, must be set after calling servo_init().
 */
uint32_t left_match;

#endif /* SERVO_H_TXT_ */
