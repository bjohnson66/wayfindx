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
#include "nf/nf_types.h"
#include "ut/utilities.h" /**< Include utility functions. */
#include "ut/ut_types.h" /**< Include common type definitions. */

void startup();
void task_1hz();
void update_display();

boolean_t mode;


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



void startup(){
	//Initialize
	// Set clock pre-scaler to divide by 4
	clock_prescale_set(clock_div_4);
	
	mode = STAT_MODE;
		
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
	_delay_ms(0.1f);
	ut_init(); /**< Initialize utilities CSC. */
}


void task_1hz(){
	update_display();
}

void update_display(){
	char line0[MAX_COL] = SPACES;
	char line1[MAX_COL] = SPACES;  
	char line2[MAX_COL] = SPACES;
	char line3[MAX_COL] = SPACES;

	if (utc_time[0] == ' '){ //Until we solve for time
		char* temp = "Acquiring Satellites";
		for (int i = 0; i < MAX_COL; i++){
			line3[i] = temp[i];
		}		
	} else if (position_fix_indicator[0] != '1'){ //Display time once we solve for time
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
		
		if (mode == STAT_MODE){
			//line0
			for (int i = 0; i < LLA_LONG_BUFFER_SIZE; i++){
				line0[i] = latitudeLLA_str[i];
			}		
			
							
			line0[19] = mode + '0';
			line0[18] = ':';
			line0[17] = 'e';
			line0[16] = 'd';
			line0[15] = 'o';
			line0[14] = 'M';
			
			//line1
			for (int i = 0; i < GGA_LONG_BUFFER_SIZE; i++){
				line1[i] = longitude[i];
			}
			
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
			
			line3[MAX_COL-9] = 'A';
			line3[MAX_COL-8] = 'l';
			line3[MAX_COL-7] = 't';

		}else { //if mode != STAT_MODE
			
		} //end mode checks
	}//end else
	
	ds_print_string(line0, MAX_COL, 0);
	ds_print_string(line1, MAX_COL, 1);
	ds_print_string(line2, MAX_COL, 2);
	ds_print_string(line3, MAX_COL, 3);

	
} //end update display