/*
 * movement.c
 *
 *  Created on: Feb 2, 2024
 *      Author: zfsalti
 */

#include "movement.h"

double move_forward(oi_t* sensor_data, double distance_mm) {

    double sum = 0;

    oi_update(sensor_data);
    oi_setWheels(100, 100);

    while (sum < distance_mm) {
        sum += sensor_data->distance;
        oi_update(sensor_data);
        if (movement_sieze) {
            oi_setWheels(0, 0);
            return sum;
        }
    }

    oi_setWheels(0, 0);

    return sum;

}

double move_backward(oi_t* sensor_data, double distance_mm) {

    double sum = 0;
    distance_mm *= -1;

    oi_update(sensor_data);
    oi_setWheels(-100, -100);

    while (sum > distance_mm) {
        sum += sensor_data->distance;
        oi_update(sensor_data);
        if (movement_sieze) {
            oi_setWheels(0, 0);
            return sum;
        }
    }

    oi_setWheels(0, 0);

    return -1 * sum;

}


double turn_right(oi_t* sensor_data, double degrees) {
    double angle = 0;
    degrees *= -1;
    //degrees += 18; // CyBot 05
    degrees += 18;
    oi_update(sensor_data);
    oi_setWheels(-MOVEMENT_TURNING_SPEED, MOVEMENT_TURNING_SPEED);

    while (angle > degrees){
       angle += sensor_data->angle;
       oi_update(sensor_data);

       if (movement_sieze) {
           oi_setWheels(0, 0);
           return angle;
       }
    }
    oi_setWheels(0, 0);
    return angle;
}
double turn_left(oi_t* sensor_data, double degrees) {
    double angle = 0;

    oi_update(sensor_data);
    oi_setWheels(MOVEMENT_TURNING_SPEED, -MOVEMENT_TURNING_SPEED);
    //degrees -= 14; // CyBot 05
    degrees -= 14;
    while (angle < degrees){
       angle += sensor_data->angle;
       oi_update(sensor_data);

       if (movement_sieze) {
           oi_setWheels(0, 0);
           return angle;
       }

    }
    oi_setWheels(0, 0);
    return angle;
}

double get_out_of_way(oi_t* sensor_data, double distance_mm) {
    double distanceMovedToFinish = 0;
       oi_setWheels(250, 250);
       double offset_x_component = 0;
       while (distanceMovedToFinish < distance_mm) {
           //distanceMovedToFinish += sensor_data->bumpLeft;
           distanceMovedToFinish += sensor_data->distance;
           oi_update(sensor_data);
           if (sensor_data->bumpRight || sensor_data->bumpLeft) {
               distanceMovedToFinish -= move_backward(sensor_data, 150);
               if (sensor_data->bumpRight) {
                   turn_left(sensor_data, 90);
                   offset_x_component -= move_forward(sensor_data, 150);
                   turn_right(sensor_data, 90);
               }
               else {
                   turn_right(sensor_data, 90);
                   offset_x_component += move_forward(sensor_data, 150);
                   turn_left(sensor_data, 90);
               }
               oi_setWheels(250, 250);
           }

       }
       oi_setWheels(0, 0);
       return distanceMovedToFinish;
}

double move_and_avoid(oi_t* sensor_data, double distance_mm) {
    double distanceMovedToFinish = 0;
    oi_setWheels(250, 250);
    double offset_x_component = 0;
    while (distanceMovedToFinish < distance_mm) {
        //distanceMovedToFinish += sensor_data->bumpLeft;
        distanceMovedToFinish += sensor_data->distance;
        oi_update(sensor_data);
        if (sensor_data->bumpRight || sensor_data->bumpLeft) {
            distanceMovedToFinish -= move_backward(sensor_data, 150);
            if (sensor_data->bumpRight) {
                turn_left(sensor_data, 90);
                offset_x_component -= move_forward(sensor_data, 150);
                turn_right(sensor_data, 90);
            }
            else {
                turn_right(sensor_data, 90);
                offset_x_component += move_forward(sensor_data, 150);
                turn_left(sensor_data, 90);
            }
            oi_setWheels(250, 250);
        }

    }

   /* if (offset_x_component > 0) {
        turn_left(sensor_data, 90);
    } else if (offset_x_component < 0) {
        turn_right(sensor_data, 90);
        offset_x_component *= -1;

    }

    oi_setWheels(250, 250);
    double distanceToOffset = 0;
    oi_update(sensor_data);
    while (distanceToOffset < offset_x_component) {
        distanceToOffset += sensor_data->distance;
        oi_update(sensor_data);
    }
    */

    oi_setWheels(0, 0);
    return distanceMovedToFinish;
}

