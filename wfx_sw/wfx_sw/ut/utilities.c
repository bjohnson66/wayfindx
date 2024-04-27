#include <avr/io.h>
#include "utilities.h"

//global variables
boolean_t ut_mode;
uint8_t ut_operation;
uint8_t ut_memory_0idx;
float ut_lat_mem_floats[MAX_MEM_INDEX];
float ut_long_mem_floats[MAX_MEM_INDEX];
char ut_lat_mem_str[LLA_LAT_BUFFER_SIZE];
char ut_long_mem_str[LLA_LONG_BUFFER_SIZE];


//local static variables
static uint16_t num_button_polls;
static uint16_t btn_on_time[NUM_BUTTONS]; // Array to store button logic high counts
static uint8_t btn_off_time[NUM_BUTTONS]; // Array to store button states after debouncing
static boolean_t prev_state[NUM_BUTTONS];
static boolean_t btn_state[NUM_BUTTONS]; // Array to store button states after debouncing

//local functions
boolean_t is_button_pressed(volatile uint8_t *port, uint8_t pin);

//load position from non-vol memory
void ut_load_from_non_vol(uint8_t index, float* longitude, float* latitude){
	//TODO, load ith element of array from non-vol... or whole array at once if possible?
	*longitude = 0.0f;
	*latitude = 0.0f;
}

void ut_write_to_non_vol(uint8_t index){
	//TODO, look at index and read from ut_long_mem_floats and ut_lat_mem_floats
}


void ut_convert_lat_float_to_string(float lat_float, char* lat_string){
	//passed by value; will not change float from where function is called
	if (lat_float < 0){
		lat_string[0] = '-';
		lat_float *= -1; //make positive for integer math 
	} else{
		lat_string[0] = '+';
	}

	// Extract integer and decimal parts
	uint16_t integer_part = (uint16_t)(lat_float);
	float fractional_part = lat_float - integer_part;
	uint32_t decimal_part = (uint32_t)(fractional_part * 100000);


	// Convert integer part to string
	lat_string[1] = '0' + ((integer_part / 10) % 10); // Tens
	lat_string[2] = '0' + (integer_part % 10); // Ones

	// Decimal point
	lat_string[3] = '.';

	// Convert decimal part to string
	lat_string[4] = '0' + ((decimal_part / 10000) % 10); // Ten-thousands
	lat_string[5] = '0' + ((decimal_part / 1000)% 10); // Thousands
	lat_string[6] = '0' + ((decimal_part / 100) % 10); // Hundreds
	lat_string[7] = '0' + ((decimal_part / 10) % 10); // Tens
	lat_string[8] = '0' + (decimal_part % 10); // Ones
}

void ut_convert_long_float_to_string(float long_float, char* long_string){
	//passed by value; will not change float from where function is called
	if (long_float < 0){
		long_string[0] = '-';
		long_float *= -1; //make positive for integer math
	} else{
		long_string[0] = '+';
	}

	// Extract integer and decimal parts
	uint16_t integer_part = (uint16_t)(long_float);
	float fractional_part = long_float - integer_part;
	uint32_t decimal_part = (uint32_t)(fractional_part * 100000);


	// Convert integer part to string
	long_string[1] = '0' + ((integer_part / 100) % 10); // Hundreds
	long_string[2] = '0' + ((integer_part / 10) % 10); // Tens
	long_string[3] = '0' + (integer_part % 10); // Ones

	// Decimal point
	long_string[4] = '.';

	// Convert decimal part to string
	long_string[5] = '0' + ((decimal_part / 10000) % 10); // Ten-thousands
	long_string[6] = '0' + ((decimal_part / 1000)% 10); // Thousands
	long_string[7] = '0' + ((decimal_part / 100) % 10); // Hundreds
	long_string[8] = '0' + ((decimal_part / 10) % 10); // Tens
	long_string[9] = '0' + (decimal_part % 10); // Ones
}


/*
 *@brief ut_init initializes what ut needs on startup
 *
 *@return none
*/
void ut_init()
{
	//read from SD card
	for (int i = 0; i < MAX_MEM_INDEX; i++){
		ut_load_from_non_vol(i, ut_long_mem_floats+i, ut_lat_mem_floats+i);
	}

	//initialize globals
	ut_mode = NAV_MODE;
	ut_operation = SAVE_OP;
	ut_memory_0idx = 0;

	ut_convert_lat_float_to_string(ut_lat_mem_floats[ut_memory_0idx], ut_lat_mem_str);
	ut_convert_long_float_to_string(ut_long_mem_floats[ut_memory_0idx], ut_long_mem_str);

	//init local static
	num_button_polls = 0;
	for (int i = 0; i < NUM_BUTTONS; i++){
		btn_on_time[i] = 0;
		btn_off_time[i] = 0;
		prev_state[i] = false;
		btn_state[i] = false;

	}


    // Set PD2, PD3, PD4, PD5 pins as inputs
	DDRC &= ~(1 << ACTION_BTN);
	DDRC &= ~(1 << OP_SELECT_BTN);
	DDRC &= ~(1 << MEM_SELECT_BTN);
	DDRC &= ~(1 << MODE_SELECT_BTN);
	
	 // Enable pull-up resistors for PC0, PC1, PC2, PC3 pins
    PORTC |= (1 << ACTION_BTN);
    PORTC |= (1 << OP_SELECT_BTN);
    PORTC |= (1 << MEM_SELECT_BTN);
    PORTC |= (1 << MODE_SELECT_BTN);
	
	return;
}

//triggers button on release, called by interrupt
void ut_poll_btns(){
	// Iterate through each button
	for (int i = 0; i < NUM_BUTTONS; i ++){
		prev_state[i] = btn_state[i];

		if (is_button_pressed(&PINC, i)) {
			// Increment button logic high count if pressed
			btn_on_time[i]++;
			btn_off_time[i] = 0;
			// Check if button press threshold is reached
			if (btn_on_time[i] >= ON_TIME_THRESHHOLD) {
				btn_state[i] = true; // Set button state to pressed
				btn_on_time[i] = 0;
				btn_off_time[i] = 0;
			}

		}else{
			btn_off_time[i]++;
			if (btn_off_time[i] >= RESET_TIME_THRESHHOLD){
				btn_state[i] = false; // Set button state to released
				btn_on_time[i] = 0;
				btn_off_time[i] = 0;
			}
		}


			
	} //end for loop


	// Check for virtual short and take action accordingly
	if (!(btn_state[MODE_SELECT_BTN]) && (prev_state[MODE_SELECT_BTN])) {
		// Mode select button pressed
		ut_mode ^= 1;  //toggle mode
	} else if ((ut_mode != STAT_MODE) && !(btn_state[MEM_SELECT_BTN]) && (prev_state[MEM_SELECT_BTN])) {
		// Memory select button pressed
		ut_memory_0idx = (ut_memory_0idx + 1)%MAX_MEM_INDEX; //cycle memory index selected
		//update strings to reflect selected mem location
		ut_convert_lat_float_to_string(ut_lat_mem_floats[ut_memory_0idx], ut_lat_mem_str);  
		ut_convert_long_float_to_string(ut_long_mem_floats[ut_memory_0idx], ut_long_mem_str);

	} else if ((ut_mode != STAT_MODE) && !(btn_state[OP_SELECT_BTN]) && (prev_state[OP_SELECT_BTN])) {
		// Operation select button pressed
		ut_operation = (ut_operation + 1)%NUM_OPERATIONS; //cycle operation selected

	} else if ((ut_mode != STAT_MODE) && !(btn_state[ACTION_BTN]) && (prev_state[ACTION_BTN])) {
		// Action button pressed
		//#TODO Implement action for action button press
		switch (ut_operation){
			case SAVE_OP:
				//Load into global array
				ut_lat_mem_floats[ut_memory_0idx] = latitudeLLA_float;
				ut_long_mem_floats[ut_memory_0idx] = longitudeLLA_float;

				//update string
				ut_convert_lat_float_to_string(ut_lat_mem_floats[ut_memory_0idx], ut_lat_mem_str);
				ut_convert_long_float_to_string(ut_long_mem_floats[ut_memory_0idx], ut_long_mem_str);

				//write to SD card
				ut_write_to_non_vol(ut_memory_0idx);

			break;
			case CLEAR_OP:
				 //Load into global array
				 ut_lat_mem_floats[ut_memory_0idx] = 0.0f;
				 ut_long_mem_floats[ut_memory_0idx] = 0.0f;

				 //update string
				 ut_convert_lat_float_to_string(ut_lat_mem_floats[ut_memory_0idx], ut_lat_mem_str);
				 ut_convert_long_float_to_string(ut_long_mem_floats[ut_memory_0idx], ut_long_mem_str);

				 //write to SD card
				 ut_write_to_non_vol(ut_memory_0idx);
 
			break;
			case RESET_OP:
				for (int i  = 0; i < MAX_MEM_INDEX; i++){
					//Load into global array
					ut_lat_mem_floats[i] = 0.0f;
					ut_long_mem_floats[i] = 0.0f;

					//update string
					ut_convert_lat_float_to_string(ut_lat_mem_floats[i], ut_lat_mem_str);
					ut_convert_long_float_to_string(ut_long_mem_floats[i], ut_long_mem_str);

					//write to SD card
					ut_write_to_non_vol(i);
				}
			break;
			default: //not reachable; error will show on screen (see main)
			break;
		}
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