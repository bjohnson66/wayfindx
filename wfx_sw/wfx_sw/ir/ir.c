#include "ir.h"
#include <avr/interrupt.h>
#include "../ut/utilities.h"

//global
uint16_t ir_sec_counter = 0; /**< Seconds counter for indicating TTFF */
boolean_t ir_trigger_1hz_flag_g = false; /**< Global flag indicating 1Hz trigger */



//local static
static uint8_t timer2_overflow_counter;


// Interrupt Service Routine for Timer0 overflow: Poll all four buttons in background set state if we have polled enough times
ISR(TIMER0_OVF_vect) {
	ut_poll_btns();
	TCNT0 = 0;
}

// Interrupt Service Routine for Timer2 overflow: Used to trip trigger flag at 1Hz
ISR(TIMER2_OVF_vect) {
	timer2_overflow_counter++;
	if (timer2_overflow_counter >= 15){
		timer2_overflow_counter = 0;
		ir_sec_counter++;
		if (!ir_trigger_1hz_flag_g){
			ir_trigger_1hz_flag_g = true;
		}
	}
}

/**
 * @brief Initializes interrupt system functionality.
 * 
 * This function configures Timer2 for time management and Timer 0 for background button polling.
 */
void ir_init()
{
	cli();
	//----------------------------------------------
	//timer 0 for background button polling
	OCR0A = 2;        // Set TOP (maximum value for counter)
 
	TCNT0 = 0x00;		//set counter to zero
	TCCR0A = 0x00;		//set TCCR1A to zero before configuring timer0
	TCCR0B = 0x00;
	// TOP is defined as OCR0A when WGM2:0 = 5

	// Set pre-scaler to /8
	TCCR0B |= (1 << CS01);
	// Enable Timer0 Overflow Interrupt
	TIMSK0 |= (1 << TOIE0);
	// Initialize Timer0 count
	TCNT0 = 0;


	//Set up timer2 for 1 hz task managment
	// Set Timer2 in normal mode (WGM22:0 = 0)
	TCCR2A = 0x00;
	TCCR2B = 0x00;
	// Set pre-scaler to clk/1024 for smoothing
	TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20); // Pre-scaler = 64
	// Enable Timer2 Overflow Interrupt
	TIMSK2 |= (1 << TOIE2);


	sei(); // Enable interrupts.
}
