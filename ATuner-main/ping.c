/**
 * Driver for ping sensor
 * @file ping.c
 * @author
 */

#include "ping.h"
#include "Timer.h"

volatile unsigned long START_TIME = 0;
volatile unsigned long END_TIME = 0;
volatile enum{LOW, HIGH, DONE} STATE = LOW; // State of ping echo pulse

void ping_init (void){


  // YOUR CODE HERE


    // Configure and enable the timer
 //   TIMER3_CTL_R ???;

   SYSCTL_RCGCGPIO_R |= (1 << 1); // enable port b
   while (SYSCTL_PRGPIO_R & 0x2 == 0) {}
   GPIO_PORTB_DEN_R |= (1 << 3);
   GPIO_PORTB_AFSEL_R |= (1 << 3);
   GPIO_PORTB_PCTL_R &= ~(0xF << 12);
   GPIO_PORTB_PCTL_R |= (7 << 12);

   SYSCTL_RCGCTIMER_R |= (1 << 3);
   while (SYSCTL_PRTIMER_R & 0x8 == 0) {}

   TIMER3_CTL_R &= ~(1 << 8);
   TIMER3_CFG_R = 0x4;

   TIMER3_TBMR_R = 0x7;
   TIMER3_CTL_R |= (3 << 10);
   TIMER3_TBPR_R |= 0xFF;
   TIMER3_TBILR_R |= 0xFFFF;

   TIMER3_ICR_R |= (1 << 10);
   TIMER3_IMR_R |= (1 << 10);

   IntRegister(INT_TIMER3B, TIMER3B_Handler);
   NVIC_EN1_R |= 0x10;
   NVIC_PRI9_R &= ~(0x7 << 5);
   NVIC_PRI9_R |= (1 << 5);
   TIMER3_CTL_R |= (1 << 8);
   IntMasterEnable();


}

void ping_trigger (void){
    STATE = LOW;
    // Disable timer and disable timer interrupt
    TIMER3_IMR_R &= ~(1 << 10);
    TIMER3_CTL_R &= ~(1 << 8);

    // Disable alternate function (disconnect timer from port pin)
    SYSCTL_RCGCGPIO_R |= (1 << 1); // enable port b
    while (SYSCTL_PRGPIO_R & 0x2 == 0) {}
    GPIO_PORTB_DIR_R |= (1 << 3);
    GPIO_PORTB_DEN_R |= (1 << 3);

    GPIO_PORTB_AFSEL_R &= ~(1 << 3);

    // YOUR CODE HERE FOR PING TRIGGER/START PULSE
    GPIO_PORTB_DATA_R &= ~(1 << 3); // set to 0
    GPIO_PORTB_DATA_R |= (1 << 3); // set to 1
    timer_waitMicros(5);
    GPIO_PORTB_DATA_R &= ~(1 << 3); // set to 0

    // Clear an interrupt that may have been erroneously triggered
    TIMER3_ICR_R |= (1 << 10);
    // Re-enable alternate function, timer interrupt, and timer
    GPIO_PORTB_AFSEL_R |= (1 << 3);
    TIMER3_IMR_R |= (1 << 10);
    TIMER3_CTL_R |= (1 << 8);
}

void TIMER3B_Handler(void){

  // YOUR CODE HERE
  // As needed, go back to review your interrupt handler code for the UART lab.
  // What are the first lines of code in the ISR? Regardless of the device, interrupt handling
  // includes checking the source of the interrupt and clearing the interrupt status bit.
  // Checking the source: test the MIS bit in the MIS register (is the ISR executing
  // because the input capture event happened and interrupts were enabled for that event?
  // Clearing the interrupt: set the ICR bit (so that same event doesn't trigger another interrupt)
  // The rest of the code in the ISR depends on actions needed when the event happens.

    if (TIMER3_MIS_R & 1 << 10) {
        TIMER3_ICR_R |= (1 << 10);
    switch (STATE) {
    case DONE:
    case LOW:;
        START_TIME = TIMER3_TBR_R & 0xFFFFFF;
        STATE = HIGH;
        break;
    case HIGH:;
        END_TIME = TIMER3_TBR_R & 0xFFFFFF;
        STATE = DONE;
        break;
    default:
        break;
    }
  }


}

float ping_getDistance (void){
    while (STATE != DONE) {}
    STATE = LOW;
    unsigned long length = START_TIME - END_TIME;
    double seconds = (length * 62.5) / 1000000000.0;

    return seconds * 34300 * 0.5;
}

PingDebug ping_getDistanceDebug() {
    PingDebug pd;
    pd.overflow = 0;
    pd.distance = ping_getDistance();
    pd.pulse_width = START_TIME - END_TIME;
    if (pd.pulse_width < 0) {
        pd.overflow = 1;
    }

    return pd;
}
