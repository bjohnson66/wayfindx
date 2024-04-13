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
#include <avr/sleep.h>
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
	
	int sleep_cnt = 0;
    /* Main loop */
    while (1){
       _delay_ms(0.01f);
		if (ir_trigger_1hz_flag_g == true){
			task_1hz();
			ir_trigger_1hz_flag_g = false;
			SMCR = SMCR | (1 << SE); //go to sleep
			sleep_mode();
			sleep_cnt=0;
		}
		char* test1 = "Hello World   :(";
		sleep_cnt++;
		//print counter to display
		if (sleep_cnt >= 10000){
			test1[3] = '0' + (sleep_cnt / 10000) % 10; // Get the ten thousands place
			}else{
			test1[3] = ' ';
		}
		if (sleep_cnt >= 1000){
			test1[4] = '0' + (sleep_cnt / 1000) % 10; // Get the thousands place
			}else{
			test1[4] = ' ';
		}
		if (sleep_cnt >= 100) {
			test1[5] = '0' + (sleep_cnt / 100) % 10; // Get the hundreds place
			} else{
			test1[5] = ' ';
		}
		if (sleep_cnt >= 10)  {
			test1[6] = '0' + (sleep_cnt / 10) % 10; // Get the tens place
			}else{
			test1[6] = ' ';
		}

		test1[7] = '0' + sleep_cnt % 10; // Get the ones place
		ds_print_string(test1, MAX_COL, 1);
		
	}
}

void task_1hz(){
	test_ir_display();
	char* test1 = "Hello World   :)";
	ds_print_string(test1, MAX_COL, 1);
}