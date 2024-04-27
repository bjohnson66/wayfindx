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


void task_1hz(){
	update_display();
	//Condition where USART if out of sync with NEO6-M
	if ((position_fix_indicator[0] == '1') && (speed[0] == ' ') && (speed[1] == ' ')){
		/* Re-Initialize navigation fetch CSC. */
		do {
			char* err = "Nav module off sync!";
			ds_print_string(err, MAX_COL, 1);
		} while(nf_init());
	}
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
			
			
		} //end mode checks
	}//end else
	
	ds_print_string(line0, MAX_COL, 0);
	ds_print_string(line1, MAX_COL, 1);
	ds_print_string(line2, MAX_COL, 2);
	ds_print_string(line3, MAX_COL, 3);

	
} //end update display