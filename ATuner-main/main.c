/*
 * main.c
 *
 *  Created on: Apr 5, 2024
 *      Author: zfsalti
 */

#include "servo.h"
#include "Timer.h"
#include "button.h"
#include "lcd.h"
#include <stdint.h>

int main() {
    timer_init();
    lcd_init();
    button_init();

    servo_init();
    lcd_printf("Move with left 2. Get value with last button.");
    uint32_t current_match = 296000; // Supposedly 90 degrees
    while (1) {
        switch (button_getButton()) {
        case 1:;
            current_match -= 500;
            servo_move_match(current_match);
            while (button_getButton() != 0) {}
            break;
        case 2:;
            current_match += 500;
            servo_move_match(current_match);
            while (button_getButton() != 0) {}
            break;
        case 4:;
            servo_move_match(current_match);
            lcd_printf("%lu\n", current_match);
        default:
            break;
        }
    }
}

