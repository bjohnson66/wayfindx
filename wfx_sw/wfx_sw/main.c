/**
 * @file wfx_sw.c
 * @brief Control system for an Arduino-based setup featuring a GPS module, buttons, and a 20x4 LCD.
 *
 * WayFindX - Device that allows user to track up to 10 GPS locations at once, calculating 
 * distance between the user and one location at a time.
 *
 * @author "Avengers Assembly" - Abele Atresso and Bradley Johnson
 * @date: 2024/04/24
 */ 

/**
 * @mainpage WayFindX
 * @brief Synopsis of the WayFindX Embedded Systems Project.
 *
 * WayFindX is a handheld GPS receiver project developed by Avengers Assembly, consisting of Bradley Johnson and Abele Atresso. The project aims to create a portable navigation device that enhances outdoor navigation capabilities for users such as hikers, adventurers, and outdoor enthusiasts. WayFindX provides essential features including position and velocity tracking, storage of multiple locations, and distance calculation to selected stored positions.
 *
 * @section features Key Features
 * - Display of user position and velocity information
 * - Storage of multiple locations for navigation
 * - Calculation of distance to selected stored positions
 * - Intuitive user interface with control buttons and LCD display
 *
 * @section hardware Hardware Components
 * The project utilizes the following hardware components:
 * - Arduino microcontroller (ATMega328p)
 * - HD44780 Controlled 20x4 LCD for display
 * - Control Stick for user input
 * - NEO-6M GPS module for positioning
 * - Power Switch and 9V battery for power management
 * - Additional components for connectivity
 * - Custom-designed 3D printed case for enclosure
 *
 * @section software Software Interface
 * WayFindX provides a user-friendly interface with control buttons for navigation and interaction. The software is designed to display essential information such as current position, stored locations, and operational modes on the LCD display. Users can cycle through saved positions, select operations, and execute actions with ease.
 *
 * @section challenges Challenges Faced
 * The project encountered various challenges including serial communication, interfacing with new ICs, soldering, and enclosure design. Overcoming these challenges required careful planning, experimentation, and troubleshooting.
 *
 * For detailed documentation and code implementation, refer to the project source code and comments.\
 * @version 1.0
 * @copyright (C) 2024 Bradley Johnson and Abele Atresso
 */

#ifndef F_CPU
#define F_CPU 4000000UL /**< Define the CPU frequency to 4MHz. */
#endif

//PORT Pin 2 PD2
#include <avr/power.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "ds/ds.h" /**< Include display-related functions. */
#include "ir/ir.h" /**< Include interrupt routines. */
#include "nf/nf.h"  /**< Include navigation fetch functions */
#include "nf/nf_types.h"
#include "ut/utilities.h" /**< Include utility functions. */
#include "ut/ut_types.h" /**< Include common type definitions. */
#include <string.h>

void startup();
void task_1hz();
void update_display();


/**
 * @brief Main loop function.
 *
 * This function controls the program flow and will not return.
 * It initializes the peripherals, clears the display, and continuously updates the system behavior.
 */
int main(void)
{
	startup();
    /* Main loop */
    while (1){
		//read_nmea_msg
		read_nmea_msg_raw();
		if (ir_trigger_1hz_flag_g == true){
			task_1hz();
			ir_trigger_1hz_flag_g = false;
		}
	}
}


/**
 * @brief Performs startup initialization.
 *
 * This function initializes various peripherals and components during startup, such as the display, interrupt routines, navigation fetch, and utilities.
 */
void startup(){
	//Initialize
	// Set clock pre-scaler to divide by 4
	clock_prescale_set(clock_div_4);
			
	// Initialize computer software components (CSC's)
	ds_init(); /**< Initialize display CSC. */
	{
		char* welcome = "- - - - ~~~~ - - - -";
		ds_print_string(welcome, MAX_COL, 0);
	}
	{ //Welcome Screen - limited scope
		char* welcome = "- - - WayFindX - - -";
		ds_print_string(welcome, MAX_COL, 1);
	}
	{
		char* welcome = "- - - - ~~~~ - - - -";
		ds_print_string(welcome, MAX_COL, 2);
	}
	_delay_ms(0.1f);
		
	ir_init(); /**< Initialize interrupt routines. */
	_delay_ms(0.1f);
	if (nf_init()){ /**<Initialize navigation fetch CSC. */
		char* err = "  Nav init failure  ";
		ds_print_string(err, MAX_COL, 1);
		while(1){};
	}
	_delay_ms(0.6f);
	ut_init(); /**< Initialize utilities CSC. */
}

/**
 * @brief Executes tasks that should occur every 1Hz.
 *
 * This function is called once per second and performs tasks such as updating the display.
 */
void task_1hz(){
	//Condition where USART if out of sync with NEO6-M
	if ((position_fix_indicator[0] == '1') && (speed[0] == ' ') && (speed[1] == ' ')){
		/* Re-Initialize navigation fetch CSC. */
		do {
			char* err = "Nav module off sync!";
			ds_print_string(err, MAX_COL, 1);
		} while(nf_init());
	}else if(ut_mode == NAV_MODE) {
		//if not corrupted; do distance calculation if in nav mode
		ut_update_dist();
	}
	update_display();
}

/**
 * @brief Updates the display with relevant information.
 *
 * This function updates the display with information such as GPS coordinates, mode, operation, and memory status.
 */
void update_display(){
	char line0[MAX_COL] = SPACES;
	char line1[MAX_COL] = SPACES;  
	char line2[MAX_COL] = SPACES;
	char line3[MAX_COL] = SPACES;

	if (utc_time[0] == ' '){ //Until we solve for time
		char counter[7];
		itoa(ir_sec_counter, counter, 10);
		line0[0] = 'T';
		line0[1] = 'T';
		line0[2] = 'F';
		line0[3] = 'F';
		line0[4] = ':';
		for (int i = 0; i < 7; i++){
			if (counter[i] == 0){
				break;
			}
			line0[i+5] = counter[i];
		}
		
		char* temp = "Acquiring Satellites";
		for (int i = 0; i < MAX_COL; i++){
			line3[i] = temp[i];
		}		
	} else if (position_fix_indicator[0] != '1'){ //Display time once we solve for time
		char counter[7];
		itoa(ir_sec_counter, counter, 10);
		line0[0] = 'T';
		line0[1] = 'T';
		line0[2] = 'F';
		line0[3] = 'F';
		line0[4] = ':';
		for (int i = 0; i < 7; i++){
			if (counter[i] == 0){
				break;
			}
			line0[i+5] = counter[i];
		}
				
		line2[0] = 'U';
		line2[1] = 'T';
		line2[2] = 'C';
		line2[3] = ':';
		for (int i=0; i < GGA_UTC_BUFFER_SIZE; i++){
			line2[4+i] = utc_time[i];
		}
		
		char* temp = "Getting PVT Solution";
		for (int i = 0; i < MAX_COL; i++){
			line3[i] = temp[i];
		}
		
	} else if (position_fix_indicator[0] == '1') {	//Once we get a fix, go into normal operation
		convertNMEAtoLLA();
		
		//Mode agnostic parts
		//line0
		for (int i = 0; i < LLA_LAT_BUFFER_SIZE; i++){
			line0[i] = latitudeLLA_str[i];
		}
		
		line0[19] = ut_mode + '0';
		line0[18] = ':';
		line0[17] = 'e';
		line0[16] = 'd';
		line0[15] = 'o';
		line0[14] = 'M';
		
		for (int i = 0; i < LLA_LONG_BUFFER_SIZE; i++){
			line1[i] = longitudeLLA_str[i];
		}
		
		//mode-specific parts
		if (ut_mode == STAT_MODE){
			//line1
			line1[MAX_COL-8] = 'H';
			line1[MAX_COL-7] = 'D';
			line1[MAX_COL-6] = 'O';
			line1[MAX_COL-5] = 'P';
			line1[MAX_COL-4] = ':';
			line1[MAX_COL-3] = hdop[0];
			line1[MAX_COL-2] = hdop[1];
			line1[MAX_COL-1] = hdop[2];
			
			//line2 
			line2[0] = 'U';
			line2[1] = 'T';
			line2[2] = 'C';
			line2[3] = ':';
			for (int i=0; i < GGA_UTC_BUFFER_SIZE; i++){
				line2[4+i] = utc_time[i];
			}
			
			line2[MAX_COL-1] = satellites_used[1];
			line2[MAX_COL-2] = satellites_used[0];
			line2[MAX_COL-3] = ':';
			line2[MAX_COL-4] = 'V';
			line2[MAX_COL-5] = 'S';
			line2[MAX_COL-6] = '#';
			
			//line3
			line3[0] = 'V';
			line3[1] = 'e';
			line3[2] = 'l';
			line3[3] = ':';
			//put vel here once we get it done TODO
			for (int i =0; i < 6; i++){
				line3[4+i] = speed[i];
			}
			
			line3[MAX_COL-9] = 'A';
			line3[MAX_COL-8] = 'l';
			line3[MAX_COL-7] = 't';
			for (int i = 0; i < 6; i ++){
				line3[MAX_COL-6+i] = msl_altitude[i];
			}
			
		}else { //if mode != STAT_MODE
			//line 1
			switch (ut_operation){
				case SAVE_OP:
					strncpy(line1+(MAX_COL-5), SAVE_STR, 5);
					break;
				case CLEAR_OP:
					strncpy(line1+(MAX_COL-5), CLEAR_STR, 5);
					break;
				case RESET_OP:
					strncpy(line1+(MAX_COL-5), RESET_STR, 5);
					break;
				default:
					strncpy(line1+(MAX_COL-5), "Error", 5);
					break; 
			}
			//line2
			line2[MAX_COL-4] = 'M';
			line2[MAX_COL-3] = 'e';
			line2[MAX_COL-2] = 'm';
			line2[MAX_COL-1] = ut_memory_0idx + '0';
			
			for (int i = 0; i < LLA_LAT_BUFFER_SIZE-2; i++){
				line2[i] = ut_lat_mem_str[i];
			}
			for (int i = 0; i < LLA_LONG_BUFFER_SIZE; i++){
				line3[i] = ut_long_mem_str[i];
			}

			//line3
			line3[MAX_COL-10]= 'D';
			line3[MAX_COL-9]= 'i';
			line3[MAX_COL-8]= 's';
			line3[MAX_COL-7]= 't';
			for (int i = 0; i < DISTANCE_SIG_FIG; i++){
				line3[MAX_COL-(6-i)] = ut_distance_str[i];
			}
		} //end mode checks (stat mode)
	}//end normal operation
	
	ds_print_string(line0, MAX_COL, 0);
	ds_print_string(line1, MAX_COL, 1);
	ds_print_string(line2, MAX_COL, 2);
	ds_print_string(line3, MAX_COL, 3);
	
} //end update display