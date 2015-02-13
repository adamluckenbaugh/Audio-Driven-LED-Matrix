#include "pins.h"

void set_pin_data_directions()
{
	//set the output pins
	P1DIR |= PIN_DATA_MUX | PIN_DATA_LED | PIN_MR_COMM | PIN_CLK_COMM | PIN_STROBE | PIN_LATCH_ENABLE;
	
	P2SEL &= ~(PIN_RESET);	//rededicate XOUT (P2.7) to GPIO
	P2DIR |= PIN_RESET;		//output on (P2.7)
	
	//set the input pins
	P1DIR &= ~(PIN_AUDIO | PIN_VOLUME);
}

void set_default_outputs()
{
	//shift registers' pin settings
	latch_disable();
	data_mux_low();
	data_led_low();
	memory_clear_low();
	clock_low();
	
	//frequency chip settings
	reset_high();
	strobe_high();
}

void latch_enable()
{
	P1OUT &= ~(PIN_LATCH_ENABLE);
}

void latch_disable()
{
	P1OUT |= PIN_LATCH_ENABLE;
}

void data_mux_high()
{
	P1OUT &= ~(PIN_DATA_MUX);
}

void data_mux_low()
{
	P1OUT |= PIN_DATA_MUX;
}

void data_led_high()
{
	P1OUT &= ~(PIN_DATA_LED);
}

void data_led_low()
{
	P1OUT |= PIN_DATA_LED;
}

void memory_clear_high()
{
	P1OUT &= ~(PIN_MR_COMM);
}

void memory_clear_low()
{
	P1OUT |= PIN_MR_COMM;
}

void clock_high()
{
	P1OUT &= ~(PIN_CLK_COMM);
}

void clock_low()
{
	P1OUT |= PIN_CLK_COMM;
}

void strobe_high()
{
	P1OUT &= ~(PIN_STROBE);
}

void strobe_low()
{
	P1OUT |= PIN_STROBE;
}

void reset_high()
{
	P2OUT &= ~(PIN_RESET);
}

void reset_low()
{
	P2OUT |= PIN_RESET;
}
