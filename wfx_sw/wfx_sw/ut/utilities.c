#ifndef F_CPU
#define F_CPU 4000000UL /**< Define the CPU frequency to 4MHz. */
#endif
#include <avr/io.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include "utilities.h"

//global variables
boolean_t ut_mode; /**< Current mode */
uint8_t ut_operation; /**< Current operation */
uint8_t ut_memory_0idx; /**< Index for memory */
float ut_lat_mem_floats[MAX_MEM_INDEX]; /**< Array to store latitude */
float ut_long_mem_floats[MAX_MEM_INDEX]; /**< Array to store longitude */
char ut_lat_mem_str[LLA_LAT_BUFFER_SIZE]; /**< String to store latitude */
char ut_long_mem_str[LLA_LONG_BUFFER_SIZE]; /**< String to store longitude */
char ut_distance_str[DISTANCE_SIG_FIG]; /**< String to store distance */


//local static variables
static uint16_t num_button_polls; /**< Number of button polls */
static uint16_t btn_on_time[NUM_BUTTONS]; /**< Array to store button logic high counts */
static uint8_t btn_off_time[NUM_BUTTONS]; /**< Array to store button states after debouncing */
static boolean_t prev_state[NUM_BUTTONS]; /**< Previous state of buttons */
static boolean_t btn_state[NUM_BUTTONS]; /**< Current state of buttons */

//local functions
/**
 * @brief Checks if a button is pressed.
 * 
 * This function checks whether a specified button is pressed.
 * 
 * @param port Pointer to the port register.
 * @param pin The pin number of the button.
 * @return true if the button is pressed, false otherwise.
 */
boolean_t is_button_pressed(volatile uint8_t *port, uint8_t pin);

/**
 * @brief Loads position from non-volatile memory.
 * 
 * This function loads the longitude and latitude from non-volatile memory.
 * 
 * @param index The index of the memory location.
 * @param longitude Pointer to store the longitude value.
 * @param latitude Pointer to store the latitude value.
 */
void ut_load_from_non_vol(uint8_t index, float* longitude, float* latitude){
	//TODO, load ith element of array from non-vol... or whole array at once if possible?
	*longitude = 0.0f;
	*latitude = 0.0f;
}

/**
 * @brief Writes to non-volatile memory.
 * 
 * This function writes data to non-volatile memory based on the provided index.
 * 
 * @param index The index of the memory location.
 */
void ut_write_to_non_vol(uint8_t index){
	//TODO, look at index and read from ut_long_mem_floats and ut_lat_mem_floats
}

/**
 * @brief Converts latitude from float to string.
 * 
 * This function converts a floating-point latitude value to a string.
 * 
 * @param lat_float The latitude value as a float.
 * @param lat_string Pointer to the string to store the converted latitude.
 */
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

/**
 * @brief Converts longitude from float to string.
 * 
 * This function converts a floating-point longitude value to a string.
 * 
 * @param long_float The longitude value as a float.
 * @param long_string Pointer to the string to store the converted longitude.
 */
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


/**
 * @brief Initializes the pins for buttons, loads from SD card, and initializes stored locations on startup.
 * 
 * This function initializes the pins for buttons, loads data from an SD card, and initializes stored locations on startup.
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
	memset(ut_distance_str, ' ', DISTANCE_SIG_FIG * sizeof(char));
	
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

/**
 * @brief Checks the status of each button sequentially.
 * 
 * This function checks the status of each button sequentially. Action triggers on button release. It is to be called in the background via interrupt.
 * Post-polling action takes place in the following order: MODE_SELECT_BTN, MEM_SELECT_BTN, OP_SELECT_BTN.
 */
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

//local function definition
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

void SPI_init(){
	    // set CS, MOSI and SCK to output
	    DDR_SPI |= (1 << CS) | (1 << MOSI) | (1 << SCK);

	    // enable pull up resistor in MISO
	    DDR_SPI |= (1 << MISO);

	    // enable SPI, set as master, and clock to fosc/128
	    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
		// If we want to set clock to fck/16
		//SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
    }
uint8_t SPI_transfer(uint8_t data)
{
	// load data into register
	SPDR = data;

	// Wait for transmission complete
	while(!(SPSR & (1 << SPIF)));

	// return SPDR
	return SPDR;
}

void SD_powerUpSeq()
{
	// make sure card is deselected
	CS_DISABLE();

	// give SD card time to power up
	_delay_ms(1);

	// send 80 clock cycles to synchronize
	for(uint8_t i = 0; i < 10; i++)
	SPI_transfer(0xFF);

	// deselect SD card
	CS_DISABLE();
	SPI_transfer(0xFF);
}

/**
 * @brief A function that converts a degree value to an equivalent radian value
 */
float deg2rad(float deg) {
	return deg * (M_PI / 180);
}

// Function to convert a float distance value to a string
void float_to_string(float value, char *str, int max_size) {
	uint16_t whole_part = (int)value;
	float fractional_part = value - whole_part;
	uint8_t i;

	// Convert the whole part to string
	itoa(whole_part, str, 10); // Assuming you have itoa function available

	// Find the position of the decimal point
	for (i = 0; str[i] != '\0'; i++) {
		if ((str[i] == '\0') || (i >= max_size)) {
			break;
		}
	}
	
	if (i < max_size){
		// Add decimal point
		str[i++] = '.';
		
		if (i < max_size){
			//grab as many decimals as we can
			for (i = i; i < max_size; i++){
				fractional_part *= 10;
				str[i] = '0'+ (uint8_t)fractional_part;
				fractional_part -= (str[i] - '0');
			}
		}
	}
}

/**
 * @brief Performs distance calculation between user's current position and the position stored at the selected memory index
 *		  Uses Haversine formula; ouptuts in Km (updates the value of the ut_distance global variable)
 */
void ut_update_dist(){
	float dlat = deg2rad(ut_lat_mem_floats[ut_memory_0idx] - latitudeLLA_float);
	float dlon = deg2rad(ut_long_mem_floats[ut_memory_0idx] - longitudeLLA_float);
	float a = sin(dlat / 2) * sin(dlat / 2) +
	cos(deg2rad(latitudeLLA_float)) * cos(deg2rad(ut_lat_mem_floats[ut_memory_0idx])) * sin(dlon / 2) * sin(dlon / 2);
	float c = 2 * atan2(sqrt(a), sqrt(1 - a));
	float distance = (RADIUS_OF_EARTH + (altitudeLLA_float/1000) ) * c; //assume common altitude which has to be converted from m to KM
	//convert to string and copy to ut_distance_str;
	float_to_string(distance, ut_distance_str, DISTANCE_SIG_FIG);
}