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
#ifndef F_CPU
#define F_CPU 8000000UL /**< Define the CPU frequency to 8MHz. */
#endif

//PORT Pin 2 PD2
#include <avr/power.h>
#include <avr/io.h>
#include <util/delay.h>

#include "ds/ds.h" /**< Include display-related functions. */
#include "ir/ir.h" /**< Include interrupt routines. */
#include "nf/nf.h"  /**< Include navigation fetch functions */
#include "nf/nf_types.h"
#include "ut/utilities.h" /**< Include utility functions. */
#include "ut/ut_types.h" /**< Include common type definitions. */

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
		if (ir_trigger_1hz_flag_g == true){
			task_1hz();
			ir_trigger_1hz_flag_g = false;
		}
		//read_nmea_msg
		read_nmea_msg_raw();
	}
}



void startup(){
	//Initialize
	// Set clock pre-scaler to divide by 4
	clock_prescale_set(clock_div_2);
		
	// Initialize computer software components (CSC's)
	ds_init(); /**< Initialize display CSC. */
	{ //Welcome Screen - limited scope
		char* welcome = "- - WayFindX - -";
		ds_print_string(welcome, MAX_COL, 0);
	}
	{
		char* welcome = "- - - ~~~~ - - -";
		ds_print_string(welcome, MAX_COL, 1);
	}
	_delay_ms(0.1f);
		
	ir_init(); /**< Initialize interrupt routines. */
	_delay_ms(0.1f);
	if (nf_init()){ /**<Initialize navigation fetch CSC. */
		char* err = "Nav init failure";
		ds_print_string(err, MAX_COL, 1);
		while(1){};
	}
	_delay_ms(0.1f);
	ut_init(); /**< Initialize utilities CSC. */
}


void task_1hz(){
	update_display();
}

void update_display(){
	{
			char* line_str = "SVXX FIXX DOPXXX";
			line_str[2] = satellites_used[0];
			line_str[3] = satellites_used[1];
			
			line_str[8] = position_fix_indicator[0];
			
			line_str[13] = hdop[0];
			line_str[14] = hdop[1];
			line_str[15] = hdop[2];
			
			ds_print_string(line_str, MAX_COL, 0);
	}
	
	{
		char* line_str = "UTC:XXXXXXXXX XX";
		for (int i=0; i < GGA_UTC_BUFFER_SIZE; i++){
			line_str[4+i] = utc_time[i];
		}
		
		line_str[14] = ns_indicator2[0];
		line_str[15] = ew_indicator[0];
		
		ds_print_string(line_str, MAX_COL, 1);
	}
}