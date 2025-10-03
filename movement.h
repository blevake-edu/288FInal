/*
 * movement.h
 *
 *  Created on: Feb 2, 2024
 *      Author: zfsalti
 */

#ifndef MOVEMENT_H_
#define MOVEMENT_H_

#include "open_interface.h"

#define MOVEMENT_TURNING_SPEED 100

extern volatile int movement_sieze;

double move_forward(oi_t* sensor_data, double distance_mm);
double move_backward(oi_t* sensor_data, double distance_mm);

double turn_right(oi_t* sensor_data, double degrees);
double turn_left(oi_t* sensor_data, double degrees);

double move_and_avoid(oi_t* sensor_data, double distance_mm);

#endif /* MOVEMENT_H_ */
