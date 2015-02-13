#ifndef PINS_H_
#define PINS_H_

#include <msp430g2231.h>

/*
 * Pin assignments:
 */

/************* Port 1 Pin Assignments ************/
//P1.2
#define PIN_CLK_COMM BIT2
//P1.3
#define PIN_MR_COMM BIT3
//P1.4
#define PIN_DATA_LED BIT4
//P1.5
#define PIN_LATCH_ENABLE BIT5
//P1.6
#define PIN_DATA_MUX BIT6
//P1.7
#define PIN_STROBE BIT7

//Input pins
//P1.0
#define PIN_AUDIO BIT0
//P1.1
#define PIN_VOLUME BIT1

/*
// DATA_MUX = P1.2
#define PIN_DATA_MUX BIT2
// DATA_LED = P1.3
#define PIN_DATA_LED BIT3
// MR_COMM        = P1.4
#define PIN_MR_COMM BIT4
// CLK_COMM = P1.5
#define PIN_CLK_COMM BIT5
*/


/************* Port 2 Pin Assignments *************/
//P2.7
#define PIN_RESET BIT7


/*
//audio ADC input on pin P1.0		(input)
#define PIN_AUDIO BIT0
//volume ADC input on pin P1.1		(input)
#define PIN_VOLUME BIT1
//strobe on pin P1.7
#define PIN_STROBE BIT7
//PIN_LATCH_ENABLE on pin P1.6
#define PIN_LATCH_ENABLE  BIT6

// PIN_RESET   = P2.7
#define PIN_RESET BIT7
*/

void set_pin_data_directions();
void set_default_outputs();

/*
 * Each of these functions refers to the pin logic state AT THE DESTINATION CHIP.
 * Some of these pins have to be set to the inverse to achieve the state at
 * the chip pin.  In other words, a pin on the MSP430 may have to be high for
 * the chip receiving the signal to see it as low and vice versa.
 */

void latch_enable();
void latch_disable();
void data_mux_high();
void data_mux_low();
void data_led_high();
void data_led_low();
void memory_clear_high();
void memory_clear_low();
void clock_high();
void clock_low();

void strobe_high();
void strobe_low();
void reset_high();
void reset_low();

#endif
