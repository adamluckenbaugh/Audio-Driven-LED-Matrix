#include <msp430g2231.h> 

/*
 * |main.c|: Main file for the LED matrix driver project.
 *
 * Author: Adam Luckenbaugh
 * Date:   28 June 2014
 *
 */

#include "pins.h"	//pin assignments & functions to change pin states

/****** function declarations *******/
extern void set_pin_data_directions();
extern void set_default_outputs();
void set_system_clock();
void timerA_init();
void adc_init();

/***** global variables *************/
volatile unsigned gBitMasks[7] = {};
volatile unsigned char gTimerTicks = 0;
volatile unsigned char gADCTimerTicks = 0;

//these are used for displaying the bit masks
volatile unsigned char gCurrentBit = 0x01;
volatile unsigned char gCurrentColumn = 0;

//these are used for storing the latest audio numbers in each bit mask
volatile unsigned char gADCColumn = 0;
#define SA0 0
#define SA1 1
#define SA2_1 2
#define SA2_2 3
#define SA3 4
#define SA4 5
#define SA5 6
#define SA6 7
volatile unsigned char gADCState = SA0;
volatile unsigned char gADCFinished = 0;

#define ADC_NUM_CHANNELS 2
volatile unsigned int gADCValues[ADC_NUM_CHANNELS];

//FSM state-related defines and variables
#define CLKING_DATA   0
#define CLK_HIGH      1
#define CLK_LOW       2
#define MEM_CLR_CYCLE 3
volatile unsigned char gState = CLKING_DATA;

//macros
#define _BMASK_SET(x) (0b11111111 >> x);
#define THRESHOLD(x) ( (x * (gADCValues[0] >> 3)) + 20)

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
    set_system_clock();
    set_pin_data_directions();
    set_default_outputs();
    timerA_init();
    adc_init();
    
    //Enable interrupts
    __enable_interrupt();
    
    while(1)
    {
    	if(gTimerTicks)
    	{
			switch(gState)
			{
				case MEM_CLR_CYCLE:
					memory_clear_low();
					memory_clear_high();
					gState = CLKING_DATA;
					break;
			
				case CLK_LOW:
					clock_low();
					
					if(gCurrentBit == 0x80)
					{
						latch_disable();
						gState = MEM_CLR_CYCLE;
					}
					else
					{
						gState = CLKING_DATA;
					}
					
					break;
				
				case CLK_HIGH:
					//clock the bit into the shift registers
					clock_high();
					
					if(gCurrentBit == 0x80)
					{
						//enable the latch because the bit masks are now ready to be displayed
						latch_enable();
					}
					
					gState = CLK_LOW;
					break;
					
				case CLKING_DATA:
					
					//set the data pin for the LED shift register
					if( (gBitMasks[gCurrentColumn] & gCurrentBit) != 0)
					{
						data_led_high();
					}
					else
					{
						data_led_low();
					}
					
					//set the data pin for the mux shift register
					
					//NOTE: Q7 on the shift register is not used and it holds the first bit clocked in
					if( ( (0x01 << gCurrentColumn) & gCurrentBit) != 0)
					{
						data_mux_high();
					}
					else
					{
						data_mux_low();
					}
					
					//increment the current bit by shifting right 1 bit
					gCurrentBit >>= 1;
					
					if(gCurrentBit == 0x00)
					{
						//It is finished clocking in the current bit mask, so reset the current bit marker
						gCurrentBit = 0x80;
						
						if(gCurrentColumn == 6)
						{
							//this is now done displaying the current set of bit masks on the LED matrix
							
							//reset the column marker
							gCurrentColumn = 0;
						}
						else
						{
							gCurrentColumn++;
						}
					}
					
					//set the next state
					gState = CLK_HIGH;
					
					break;
			}
			
			
			if(gADCTimerTicks == 2)
			{
				//work on getting the next set of bit masks
				switch(gADCState)
				{
					case SA0:
						reset_high();
						strobe_high();
						gADCState = SA1;
						break;
						
					case SA1:
						strobe_low();
						gADCState = SA2_1;
						break;
						
					case SA2_1:
						strobe_high();
						gADCState = SA2_2;
						break;
						
					case SA2_2:
						reset_low();
						gADCState = SA3;
						break;
						
					case SA3:
						strobe_low();
						gADCState = SA4;
						break;
						
					case SA4:
						gADCState = SA5;
						
						//trigger another ADC conversion
						ADC10CTL0 |= ADC10SC;
						
						break;
						
					case SA5:
						if(gADCFinished == 1)
						{
							gADCState = SA6;
							gADCFinished = 0;
						}
						
					case SA6:
						strobe_high();
						//increment the bit mask we are editing, wrapping around if necessary.
						//Also, set the next ADC FSM state.
						if(gADCColumn == 6)
						{
							gADCColumn = 0;
							gADCState = SA3;	//do the read cycle again
						}
						else
						{
							gADCColumn++;
							gADCState = SA3;
						}
						break;
				}
				gADCTimerTicks = 0;
			}
    		//decrement the number of recorded timer ticks
    		gTimerTicks--;
    	}
    }
}

void set_system_clock()
{
	/* https://groups.google.com/forum/#!topic/ti-launchpad/ISV1nmCUOxI */
	//set ~20MHz clock
	BCSCTL2 = 0x10; // MCLK = DCOCLK / 2, so that the CPU will not crash
	BCSCTL1 = 0x8F; // the highest RSEL
	DCOCTL = 0xE0; // the highest DCO
	/********************************************************************/
}

void timerA_init()
{
	CCTL0 = CCIE;			 // CCR0 interrupt enabled
	TACTL = TASSEL_2 | MC_1; // Set the timer A to SMCLCK, Up mode
	CCR0 = 200;	//10us period
}

void adc_init()
{
	//This works for no volume control
	/*
	ADC10CTL0 &= ~ENC;												// Disable ADC
	ADC10CTL0 = SREF_0 | ADC10SHT_3 | ADC10ON | ADC10IE;			// 64 clock ticks, ADC On, enable ADC interrupt
	ADC10CTL1 = ADC10SSEL_3 | INCH_0;								//SMCLK, A0
	ADC10AE0 |= PIN_AUDIO;											//enable adc input
	ADC10CTL0 |= ENC;             									// Enable
	*/
	
	// Disable ADC before configuration.
	ADC10CTL0 &= ~ENC;

	// Turn ADC on in single line before configuration.
	ADC10CTL0 = ADC10ON;

	// Make sure the ADC is not running per 22.2.7
	while(ADC10CTL1 & ADC10BUSY);

	// Repeat conversion.
	ADC10DTC0 = ADC10CT;
	// Only one conversion at a time.
	ADC10DTC1 = ADC_NUM_CHANNELS;
	// Put results at specified place in memory.
	ADC10SA = ((unsigned int)gADCValues);

	// 8 clock ticks, ADC On, Multi-Sample Conversion, Interrupts enabled.
	ADC10CTL0 |= ADC10SHT_1 | SREF_0  | ADC10IE | MSC;
	// Set first channel to A1 (decrements to A0 while reading), Use SMCLK, sequence of channels.
	ADC10CTL1 = INCH_1 | ADC10SSEL_3 | CONSEQ_1;

	// Enable conversion.
	ADC10CTL0 |= ENC;
	
}

// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	gTimerTicks++;
	gADCTimerTicks++;
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
	// Based on the volume of the input audio signal detected by the ADC, 
	// set the height of the LED column in the matrix.
	if(gADCValues[1] > THRESHOLD(8))
	{
		gBitMasks[gADCColumn] = _BMASK_SET(0);
	}
	else if(gADCValues[1] > THRESHOLD(7))
	{
		gBitMasks[gADCColumn] = _BMASK_SET(1);
	}
	else if(gADCValues[1] > THRESHOLD(6))
	{
		gBitMasks[gADCColumn] = _BMASK_SET(2);
	}
	else if(gADCValues[1] > THRESHOLD(5))
	{
		gBitMasks[gADCColumn] = _BMASK_SET(3);
	}
	else if(gADCValues[1] > THRESHOLD(4))
	{
		gBitMasks[gADCColumn] = _BMASK_SET(4);
	}
	else if(gADCValues[1] > THRESHOLD(3))
	{
		gBitMasks[gADCColumn] = _BMASK_SET(5);
	}
	else if(gADCValues[1] > THRESHOLD(2))
	{
		gBitMasks[gADCColumn] = _BMASK_SET(6);
	}
	else if(gADCValues[1] > THRESHOLD(1))
	{
		gBitMasks[gADCColumn] = _BMASK_SET(7);
	}
	else
	{
		gBitMasks[gADCColumn] = 0;
	}
	gADCFinished = 1;
}
