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

#include <avr/io.h>
#include <avr/power.h>
#include <util/delay.h>
#include "ut/utilities.h" /**< Include utility functions. */
#include "ds/ds.h" /**< Include display-related functions. */
#include "ir/ir.h" /**< Include interrupt routines. */
#include "ut/ut_types.h" /**< Include common type definitions. */

void task_1hz();

/**
 * @brief Main loop function.
 *
 * This function controls the program flow and will not return.
 * It initializes the peripherals, clears the display, and continuously updates the system behavior.
 */
int main(void)
{
	//Initialize
	// Set clock pre-scaler to divide by 4
	clock_prescale_set(clock_div_4);
		
	// Initialize computer software components (CSC's)
	ds_init(); /**< Initialize display CSC. */
	ir_init(); /**< Initialize interrupt routines. */
	ut_init(); /**< Initialize utilities CSC. */
	
	{ //Welcome Screen - limited scope
		char* test1 = "- - WayFindX - -";
		ds_print_string(test1, MAX_COL, 0);	
	}
		
    /* Main loop */
    while (1){
       _delay_ms(0.1f);
		if (ir_trigger_1hz_flag_g == true){
			task_1hz();
			ir_trigger_1hz_flag_g = false;
		}
	}
}

void task_1hz(){
	test_ir_display();
	char* test1 = "Hello World   :)";
	ds_print_string(test1, MAX_COL, 1);
}