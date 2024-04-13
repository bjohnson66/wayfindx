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
#define F_CPU 4000000UL /**< Define the CPU frequency to 8MHz. */
#endif

//PORT Pin 2 PD2
#include <avr/power.h>
#include <avr/io.h>
#include <util/delay.h>

#include "ds/ds.h" /**< Include display-related functions. */
#include "ir/ir.h" /**< Include interrupt routines. */
#include "nf/nf.h"  /**< Include navigation fetch functions */
#include "ut/utilities.h" /**< Include utility functions. */
#include "ut/ut_types.h" /**< Include common type definitions. */

void startup();
void task_1hz();
void test_ir_display();

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
		read_nmea_msg();
		if (ir_trigger_1hz_flag_g == true){
			task_1hz();
			ir_trigger_1hz_flag_g = false;
		}
	}
}



void startup(){
	//Initialize
	// Set clock pre-scaler to divide by 4
	clock_prescale_set(clock_div_4);
		
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
		
	ir_init(); /**< Initialize interrupt routines. */
	ut_init(); /**< Initialize utilities CSC. */
	if (nf_init()){ /**<Initialize navigation fetch CSC. */
		char* err = "Nav init failure";
		ds_print_string(err, MAX_COL, 1);
		while(1){};
	}
}


void task_1hz(){
	test_ir_display();
	
}

void test_ir_display(){
	char* ir_test_string = "                ";
	//print counter to display
	if (ir_test_counter >= 10000){
		ir_test_string[11] = '0' + (ir_test_counter / 10000) % 10; // Get the ten thousands place
		}else{
		ir_test_string[11] = ' ';
	}
	if (ir_test_counter >= 1000){
		ir_test_string[12] = '0' + (ir_test_counter / 1000) % 10; // Get the thousands place
		}else{
		ir_test_string[12] = ' ';
	}
	if (ir_test_counter >= 100) {
		ir_test_string[13] = '0' + (ir_test_counter / 100) % 10; // Get the hundreds place
		} else{
		ir_test_string[13] = ' ';
	}
	if (ir_test_counter >= 10)  {
		ir_test_string[14] = '0' + (ir_test_counter / 10) % 10; // Get the tens place
		}else{
		ir_test_string[14] = ' ';
	}

	ir_test_string[15] = '0' + ir_test_counter % 10; // Get the ones place
	ds_print_string(ir_test_string, MAX_COL, 0);
}