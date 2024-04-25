#include "utilities.h"
#include "../ds/ds.h"
#include <avr/io.h>

//global variables
boolean_t ut_mode;
uint8_t ut_operation;
uint8_t ut_memory_0idx;

//local static variables
static uint16_t num_button_polls;
static uint16_t btn_counts[NUM_BUTTONS]; // Array to store button logic high counts

//local static functions
boolean_t is_button_pressed(volatile uint8_t *port, uint8_t pin);


/*
 *@brief ut_init initializes what ut needs on startup
 *
 *@return none
*/
void ut_init()
{
	//initialize globals
	ut_mode = NAV_MODE;
	ut_operation = SAVE_OP;
	ut_memory_0idx = 0;

	//init local static
	num_button_polls = 0;
	for (int i = 0; i < NUM_BUTTONS; i++){
		btn_counts[i] = 0;
	}


    // Set PD2, PD3, PD4, PD5 pins as inputs
	DDRD &= ~(1 << ACTION_BTN);
	DDRD &= ~(1 << OP_SELECT_BTN);
	DDRD &= ~(1 << MEM_SELECT_BTN);
	DDRD &= ~(1 << MODE_SELECT_BTN);
	
	 // Enable pull-up resistors for PD2, PD3, PD4, PD5 pins
    PORTD |= (1 << ACTION_BTN);
    PORTD |= (1 << OP_SELECT_BTN);
    PORTD |= (1 << MEM_SELECT_BTN);
    PORTD |= (1 << MODE_SELECT_BTN);


    DDRB |= (1 << DEBUG_LED_PIN);


	return;
}

void ut_poll_btns(){
	boolean_t btn_state[NUM_BUTTONS] = {false}; // Array to store button states after debouncing
	num_button_polls++;

	// Iterate through each button
	for (int i = 0; i < NUM_BUTTONS; i ++){
		if (is_button_pressed(&PIND, i + 2)) {
			// Increment button logic high count if pressed
			btn_counts[i]++;
		}

		btn_state[i] = false; // Set button state to released
		//once we have debounced enough times
		if (num_button_polls >= DEBOUNCE_TIME){
			DEBUG_LIGHT_ON
			// Check if button press threshold is reached
			if (btn_counts[i] >= MIN_THRESHOLD) {
				btn_state[i] = true; // Set button state to pressed
				DEBUG_LIGHT_OFF
			}
			btn_counts[i] = 0;
		}
	} //end for loop
	num_button_polls %= DEBOUNCE_TIME;

	// Check for virtual short and take action accordingly
	if (btn_state[MODE_SELECT_BTN - 2]) {
		// Mode select button pressed
		ut_mode ^= 1;  //toggle mode
	} else if (btn_state[MEM_SELECT_BTN - 2]) {
		// Memory select button pressed
		ut_memory_0idx = (ut_memory_0idx + 1)%MAX_MEM_INDEX; //cycle memory index selected

	} else if (btn_state[OP_SELECT_BTN - 2]) {
		// Operation select button pressed
		ut_operation = (ut_operation + 1)%NUM_OPERATIONS; //cycle operation selected

	} else if (btn_state[ACTION_BTN - 2]) {
		// Action button pressed
		//#TODO Implement action for action button press
	}
}

//local functions
/**
 * @brief Simple utility function to poll an individual button. keeping for maintainability
		  if button needs switched to different port or pin
 */
boolean_t is_button_pressed(volatile uint8_t *port, uint8_t pin) {
	// Check if the button is pressed (logic low)
	if (!(*port & (1 << pin))) {
		return true;
	}
	return false;
}