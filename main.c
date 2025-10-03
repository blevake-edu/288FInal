

/**
 * main.c
 */

#include "movement.h"
#include "uart-interrupt.h"
//#include "cyBot_uart.h"
#include "servo.h"
#include "ping.h"
#include "math.h"
#include "adc.h"
#include <stddef.h>
#include "open_interface.h"

typedef struct {
    float ping_distance;
    unsigned short ir_value;
    float ir_distance;
} Pair_t ;

typedef struct {
    int start_angle;
    int end_angle;
    float distance_to;
    float arc_length;
    float width;
} Object;

typedef enum {
    MODE_AUTO,
    MODE_MANUAL,
    MODE_SCAN,
    MODE_QUIT
} MODE;

typedef struct {
    /**
     * size of 181
     */
    Pair_t* scan_data;
    oi_t* oi;
    MODE current_mode;
} App;

volatile int movement_sieze = 0; // defined in movement.h

volatile char manual_mode_last_press = 0;

void command_handler(char byte, void* context) {
    App* app = context;
    switch (byte) {
    case 't': {
        // toggle mode
        if (app->current_mode == MODE_MANUAL)
            app->current_mode = MODE_AUTO;
        else {
            movement_sieze = 1;
            app->current_mode = MODE_MANUAL;
        }
        break;
    }
    case 'm': {
        // begin scan
        movement_sieze = 1;
        app->current_mode = MODE_SCAN;
        break;
    }
    case 'w':
    case 'a':
    case 's':
    case 'd':
    case '0': {
        // begin scan
        manual_mode_last_press = byte;
        break;
    }
    case 'q': {
        movement_sieze = 1;
        app->current_mode = MODE_MANUAL;
        manual_mode_last_press = byte;
    }
    default:
        break;
    }
}

App* app_alloc() {
    uart_interrupt_init();
    timer_init();
    App* app = malloc(sizeof(App));
    app->scan_data = malloc(181 * sizeof(Pair_t));
    servo_init();
    adc_init();
    app->oi = oi_alloc();
    oi_init(app->oi);
    ping_init();
    uart_set_handler(command_handler, app);
    app->current_mode = MODE_MANUAL;

    return app;
}

void app_free(App* app) {
    oi_free(app->oi);
    free(app->scan_data);
    free(app);
}


// size of 181
void scan(App* app) {
    char buf[10];
    int degrees;
    for (degrees = 0; degrees <= 180; degrees++) {
        servo_move_tuned(degrees);
        timer_waitMillis(100);

        unsigned short ir_val = adc_read_avg();
        float ir_dist = ir_to_cm(ir_val);
        ping_trigger();
        float sound_dist = ping_getDistance();
        app->scan_data[degrees].ping_distance = sound_dist;
        app->scan_data[degrees].ir_value = ir_val;
        app->scan_data[degrees].ir_distance = ir_dist;

        sprintf(buf, "%.3hu:%.3hu\r\n", degrees, (unsigned short) ir_dist);
        uart_sendStr(buf);

        // mode check
        if (app->current_mode == MODE_MANUAL) return;
        // mode check
    }
}



typedef enum {
    MovementAxisForward,
    MovementAxisBackward,
    MovementAxisRight,
    MovementAxisLeft,
    MovementAxisNone,
} MovementAxis;


void manualmode(App* app) {
    int rolling = 0;
    MovementAxis current_movement_axis = MovementAxisNone;
    char buf[12];
    while (app->current_mode == MODE_MANUAL) {
        switch (manual_mode_last_press) {
        case '0': // unpressed
            if (rolling == 1) {
            oi_setWheels(0, 0);
            oi_update(app->oi);
            rolling = 0;
            switch (current_movement_axis) {
            case MovementAxisRight: {
                // negative angle
                int angle_moved = abs(app->oi->angle); // casting to positive value and removing decimal part
                snprintf(buf, 10, "000r%03d\r\n", angle_moved);
                break;
            }
            case MovementAxisLeft: {
                // positive angle
                int angle_moved = abs(app->oi->angle); // casting to positive value and removing decimal part
                snprintf(buf, 10, "000l%03d\r\n", angle_moved);
                break;
            }
            case MovementAxisForward: {
                int distance_moved = abs(app->oi->distance / 10);
                snprintf(buf, 10, "000f%03d\r\n", distance_moved);
                break;
            }
            case MovementAxisBackward: {
                int distance_moved = abs(app->oi->distance / 10);
                snprintf(buf, 10, "000b%03d\r\n", distance_moved);
                break;
            }
            default:
                break;
            }
            uart_sendStr(buf);
            if(app->oi->bumpLeft || app->oi->bumpRight) {
                uart_sendStr("000o000\r\n");
            }
            if(app->oi->cliffLeft || app->oi->cliffRight || app->oi->cliffFrontLeft || app->oi->cliffFrontRight) {
                uart_sendStr("000c000\r\n");
            }
            if(app->oi->cliffLeftSignal > 2750 || app->oi->cliffRightSignal > 2750 || app->oi->cliffFrontRightSignal > 2750 || app->oi->cliffFrontLeftSignal > 2750) {
                uart_sendStr("000e000\r\n");
            }
            current_movement_axis = MovementAxisNone;
            }
            break;
        case 'w':
            if (rolling) break;
            oi_update(app->oi);
            oi_setWheels(100, 100);
            rolling = 1;
            current_movement_axis = MovementAxisForward;
            break;
        case 'a':
            if (rolling) break;
            oi_update(app->oi); // set angle turn tracking to 0
            oi_setWheels(50, -50);
            rolling = 1;
            current_movement_axis = MovementAxisLeft;
            break;
        case 's':
            if (rolling) break;
            oi_update(app->oi); // set angle turn tracking to 0
            oi_setWheels(-50, -50);
            rolling = 1;
            current_movement_axis = MovementAxisBackward;
            break;
        case 'd':
            if (rolling) break;
            oi_update(app->oi);
            oi_setWheels(-50, 50);
            rolling = 1;
            current_movement_axis = MovementAxisRight;
            break;
        case 'q':
            return;
        default:
            break;
        }
    }
}




 int main(void) {



    App* app = app_alloc();

    right_match = 313500;
    left_match = 285500;

    while (1) {

        switch (app->current_mode) {
        case MODE_SCAN:
            scan(app);
            app->current_mode = MODE_MANUAL; // after a manual scan, we left off in manual mode
            movement_sieze = 0;
            break;
        case MODE_MANUAL:
            movement_sieze = 0;
            manualmode(app);
            break;
        default:
            break;
        }

        if (manual_mode_last_press == 'q') break;

    }

    app_free(app);

	return 0;
}
