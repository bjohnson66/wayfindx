#include "ir.h"
#include <avr/interrupt.h>
#include "../ut/utilities.h"

//global
uint16_t ir_test_counter = 0;
boolean_t ir_trigger_1hz_flag_g = false;


//local static
uint8_t timer2_overflow_counter;

/**
 * @brief Interrupt Service Routine (ISR) for Timer/Counter0 overflow.
 * Handles PWM generation and toggles PORTD,5.
 */
/*
ISR(TIMER0_OVF_vect) { // INT0 ISR
	
}
*/

/*
catch case where fan stops and timer rolls over.
*/
ISR(TIMER2_OVF_vect) {
	timer2_overflow_counter++;
	if (timer2_overflow_counter >= 15){
		timer2_overflow_counter = 0;
		ir_test_counter++;
		if (!ir_trigger_1hz_flag_g){
			ir_trigger_1hz_flag_g = true;
		}
	}
}

/**
 * @brief Interrupt Service Routine (ISR) for Timer/Counter2 overflow.
 * Sets the update RPM flag.
 */
/*
ISR(TIMER2_OVF_vect) {

}
*/

/**
 * @brief Initializes interrupt system functionality.
 * - Configures Timer2 for time management.
 */
void ir_init()
{
	cli();
	//----------------------------------------------
	//timer 1 for task management
	// Set Timer1 in normal mode (WGM13:0 = 0)
	//Set up timer2 for periodic display of RPMs to smooth out reading
	// Set Timer2 in normal mode (WGM22:0 = 0)
	TCCR2A = 0x00;
	TCCR2B = 0x00;

	// Set pre-scaler to clk/1024 for smoothing
	TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20); // Pre-scaler = 64

	// Enable Timer2 Overflow Interrupt
	TIMSK2 |= (1 << TOIE2);
	
	sei(); // Enable interrupts.

}
