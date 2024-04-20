#ifndef F_CPU
#define F_CPU 4000000UL /**< Define the CPU frequency to 8MHz. */
#endif

#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include "nf.h"
#include "nf_types.h"
#include "../lib/uart.h"
#include "../ds/ds.h"

//defines
#define UART_BAUD_RATE 9600
#define NMEA_MSG_ID_SIZE 5

//NMEA message IDs from NMEA documentation
#define GGA_TYPE "GPGGA"
#define GLL_TYPE "GPGLL"
#define GSA_TYPE "GPGSA"
#define GSV_TYPE "GPGSV"
#define MSS_TYPE "GPMSS"
#define RMC_TYPE "GPRMC"
#define VTG_TYPE "GPVTG"
#define ZDA_TYPE "GPZDA"
#define OK_TO_SEND_TYPE "PSRF1"

//global
//GGA MESSAGE
char utc_time[GGA_UTC_BUFFER_SIZE];				// UTC Time, e.g., "161229.487"
char latitude[GGA_LAT_BUFFER_SIZE];				// Latitude, e.g., "3723.2475"
char ns_indicator[GGA_INDICATOR_SIZE];				// N/S Indicator, 'N' for north or 'S' for south
char longitude[GGA_LONG_BUFFER_SIZE];				// Longitude, e.g., "12158.3416"
char ew_indicator[GGA_INDICATOR_SIZE];				// E/W Indicator, 'E' for east or 'W' for west
char position_fix_indicator[GGA_INDICATOR_SIZE];	// Position Fix Indicator, see Table 1-4
char satellites_used[GGA_SV_USD_BUFFER_SIZE];		// Satellites Used, range 0 to 12 eg 07
char hdop[GGA_HDOP_BUFFER_SIZE];					// HDOP (Horizontal Dilution of Precision), e.g., "1.0"

float latitudeLLA_float;   // Latitude in degrees
float longitudeLLA_float;  // Longitude in degrees
float altitudeLLA_float;   // Altitude in meters
char latitudeLLA_str[LLA_LAT_BUFFER_SIZE];   // Latitude in degrees
char longitudeLLA_str[LLA_LONG_BUFFER_SIZE];  // Longitude in degrees
char altitudeLLA_str[LLA_ALT_BUFFER_SIZE];   // Altitude in meters

//local static
static char nmea_msg_id_buffer[NMEA_MSG_ID_SIZE];
static char gga_msg_buffer[GGA_SIZE];


//local function declarations


//function definitions

uint8_t nf_init(){
	cli();
	 /*
     *  Initialize UART library, pass baudrate and AVR cpu clock
     *  with the macro 
     *  UART_BAUD_SELECT() (normal speed mode )
     *  or 
     *  UART_BAUD_SELECT_DOUBLE_SPEED() ( double speed mode)
     */
    uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );
	memset(nmea_msg_id_buffer,0,sizeof(char)*NMEA_MSG_ID_SIZE); //zeroize
	memset(gga_msg_buffer, ' ',sizeof(char)*GGA_SIZE); //zeroize msg buffer

	// Initialize the arrays within gga_msg 
	memset(utc_time, ' ', GGA_UTC_BUFFER_SIZE * sizeof(char));
	memset(latitude, ' ', GGA_LAT_BUFFER_SIZE * sizeof(char));
	memset(ns_indicator, ' ', GGA_INDICATOR_SIZE * sizeof(char));
	memset(longitude, ' ', GGA_LONG_BUFFER_SIZE * sizeof(char));
	memset(ew_indicator, ' ', GGA_INDICATOR_SIZE * sizeof(char));
	memset(position_fix_indicator, ' ', GGA_INDICATOR_SIZE * sizeof(char));
	memset(satellites_used, ' ', GGA_SV_USD_BUFFER_SIZE * sizeof(char));
	memset(hdop, ' ', GGA_HDOP_BUFFER_SIZE * sizeof(char));
	
	memset(latitudeLLA_str, ' ', LLA_LAT_BUFFER_SIZE * sizeof(char));
	memset(longitudeLLA_str, ' ', LLA_LONG_BUFFER_SIZE * sizeof(char));
	memset(altitudeLLA_str, ' ', LLA_ALT_BUFFER_SIZE * sizeof(char));

	latitudeLLA_float = 0;  
	longitudeLLA_float = 0;  
	altitudeLLA_float = 0; 
	
	
	sei(); //UART is interrupt based;	
	return NF_INIT_SUCCESS;
}

void get_serial_char(char* outputchar){
	unsigned int c;
	while (1){
		/*
		* Get received character from ringbuffer
		* uart_getc() returns in the lower byte the received character and 
		* in the higher byte (bitmask) the last receive error
		* UART_NO_DATA is returned when no data is available.
		*
		*/
		c = uart_getc();
		if (!( c & UART_NO_DATA ))
		{
			/*
				* new data available from UART
				* check for Frame or Overrun error
				*/
			if ( c & UART_FRAME_ERROR )
			{
				/* Framing Error detected, i.e no stop bit detected */
				#ifdef _DEBUG_
					char* output = "NF Frame Error! ";
					ds_print_string(output, 16, 0);
				#endif
			}
			if ( c & UART_OVERRUN_ERROR )
			{
				/* 
					* Overrun, a character already present in the UART UDR register was 
					* not read by the interrupt handler before the next character arrived,
					* one or more received characters have been dropped
					*/
				char* output = "           OR ER";
				ds_print_string(output, 16, 0);
			}
			if ( c & UART_BUFFER_OVERFLOW )
			{
				/* 
					* We are not reading the receive buffer fast enough,
					* one or more received character have been dropped 
					*/
				char* output = "OF ER";
				ds_print_string(output, 5, 0);
			}
			/* 
				* send received character back
				*/
			*outputchar = (unsigned char)c;
			return;
		}
	}
	
}


void read_nmea_msg_raw(){
	char tempChar;
	do {
		get_serial_char(&tempChar);
	} while (tempChar != '$');
	
	memset(nmea_msg_id_buffer,0,sizeof(char)*NMEA_MSG_ID_SIZE); //zeroize buffer
	for (int i = 0; i < NMEA_MSG_ID_SIZE; i++){
		get_serial_char(nmea_msg_id_buffer + i);
	}
	get_serial_char(&tempChar); //eat comma
	
	int counter = 0; //common counter to save memory alloc time	
	//-----------------------------------------------------------------
	//only need GGA and VTG for requirements. Drop others
	if(strcmp(nmea_msg_id_buffer, GGA_TYPE) == 0){
		//scrape raw
		for (counter = 0; counter < GGA_SIZE; counter ++){
			get_serial_char(gga_msg_buffer+counter);
		}
		
		//process message
		int offset = 0;
		//////////////////////////////////////////
		//grab UTC if available
		if (gga_msg_buffer[offset] != ','){
			for (int i =0; i < GGA_UTC_BUFFER_SIZE; i++){
				utc_time[i] = gga_msg_buffer[offset++];
			}
			//ds_print_string(utc_time, GGA_UTC_BUFFER_SIZE, 0);	
		}else{ //otherwise skip comma from if statement
			offset++;
		}
		offset++;// skip comma
		
		//////////////////////////////////////////
		//grab LAT if available
		if (gga_msg_buffer[offset] != ','){
			for (int i =0; i < GGA_LAT_BUFFER_SIZE; i++){
				latitude[i] = gga_msg_buffer[offset++];
			}
			
			//ds_print_string(latitude, GGA_LAT_BUFFER_SIZE, 0);
		}else{ //otherwise skip comma from if statement
			offset++;
		}
		offset++; // skip comma
		
		//////////////////////////////////////////
		//grab NS indicator if available
		if (gga_msg_buffer[offset] != ','){
			ns_indicator[0] = gga_msg_buffer[offset++];
			//ds_print_string(ew_indicator, GGA_INDICATOR_SIZE, 1);
		}else{ //otherwise skip comma from if statement
			offset++;
		}
		offset++; // skip comma
		
		
		//////////////////////////////////////////
		//grab LONG if available
		if (gga_msg_buffer[offset] != ','){
			for (int i =0; i < GGA_LONG_BUFFER_SIZE; i++){
				longitude[i] = gga_msg_buffer[offset++];
			}
			//ds_print_string(longitude, GGA_LONG_BUFFER_SIZE, 1);
		}else{ //otherwise skip comma from if statement
			offset++;
		}
		offset++; // skip comma
		
		//////////////////////////////////////////
		//grab EW indicator if available
		if (gga_msg_buffer[offset] != ','){
			for (int i =0; i < GGA_INDICATOR_SIZE; i++){
				ew_indicator[i] = gga_msg_buffer[offset++];
			}
			//ds_print_string(ew_indicator, GGA_INDICATOR_SIZE, 1);
			}else{ //otherwise skip comma from if statement
			offset++;
		}
		offset++; // skip comma
		
		
		//////////////////////////////////////////
		//grab FIX indicator if available
		if (gga_msg_buffer[offset] != ','){
			for (int i =0; i < GGA_INDICATOR_SIZE; i++){
				position_fix_indicator[i] = gga_msg_buffer[offset++];
			}
			//ds_print_string(position_fix_indicator, GGA_INDICATOR_SIZE, 1);
			}else{ //otherwise skip comma from if statement
			offset++;
		}
		offset++; // skip comma
		
		//////////////////////////////////////////
		//grab NUM_SV if available
		if (gga_msg_buffer[offset] != ','){
			for (int i =0; i < GGA_SV_USD_BUFFER_SIZE; i++){
				satellites_used[i] = gga_msg_buffer[offset++];
			}
			//ds_print_string(satellites_used, GGA_SV_USD_BUFFER_SIZE, 1);
			}else{ //otherwise skip comma from if statement
			offset++;
		}
	    offset++; // skip comma
		
		//////////////////////////////////////////
		//grab HDOP if available
		if (gga_msg_buffer[offset] != ','){
			for (int i =0; i < GGA_HDOP_BUFFER_SIZE; i++){
				hdop[i] = gga_msg_buffer[offset++];
			}
			//ds_print_string(hdop, GGA_HDOP_BUFFER_SIZE, 0);
			}else{ //otherwise skip comma from if statement
			offset++;
		}
		offset++; // skip comma
		
	} //END GGA
}




// Function to convert NMEA format coordinates to LLA form in degrees
void convertNMEAtoLLA() {
	// Convert latitude from NMEA format to degrees
	int deg = 0;
	float min = 0.0f;
	
	//convert lat char string to deg and min (more efficient than loop)
	min += (0.00001 * (latitude[GGA_LAT_BUFFER_SIZE - 1] - '0'));
	min += (0.0001 * (latitude[GGA_LAT_BUFFER_SIZE - 2] - '0'));
	min += (0.001 * (latitude[GGA_LAT_BUFFER_SIZE - 3] - '0'));
	min += (0.01 * (latitude[GGA_LAT_BUFFER_SIZE - 4] - '0'));
	min += (0.1 * (latitude[GGA_LAT_BUFFER_SIZE - 6] - '0')); //skip 5 because decimal point char
	min += (1 * (latitude[GGA_LAT_BUFFER_SIZE - 7] - '0'));
	min += (10 * (latitude[GGA_LAT_BUFFER_SIZE - 8] - '0'));
	deg += (100 * (latitude[GGA_LAT_BUFFER_SIZE - 9] - '0'));
	deg += (1000 * (latitude[GGA_LAT_BUFFER_SIZE - 10] - '0'));

	
	latitudeLLA_float = deg + min / 60.0;
	if (ns_indicator[0] == 'S') {
		latitudeLLA_float *= -1; // If south, make latitude negative
	}
	

	// Convert longitude from NMEA format to degrees
	deg = 0;
	min = 0.0f;
	
	//convert longitude char string to deg and min (more efficient than loop)
	min += (0.00001 * (longitude[GGA_LONG_BUFFER_SIZE - 1] - '0'));
	min += (0.0001 * (longitude[GGA_LONG_BUFFER_SIZE - 2] - '0'));
	min += (0.001 * (longitude[GGA_LONG_BUFFER_SIZE - 3] - '0'));
	min += (0.01 * (longitude[GGA_LONG_BUFFER_SIZE - 4] - '0'));
	min += (0.1 * (longitude[GGA_LONG_BUFFER_SIZE - 5] - '0'));
	min += (0.1 * (longitude[GGA_LONG_BUFFER_SIZE - 6] - '0')); //skip 5 because decimal point char
	min += (1 * (longitude[GGA_LONG_BUFFER_SIZE - 7] - '0'));
	min += (10 * (longitude[GGA_LONG_BUFFER_SIZE - 8] - '0'));
	deg += (100 * (longitude[GGA_LONG_BUFFER_SIZE - 9] - '0'));
	deg += (1000 * (longitude[GGA_LONG_BUFFER_SIZE - 10] - '0'));
	deg += (10000 * (longitude[GGA_LONG_BUFFER_SIZE - 11] - '0'));

	longitudeLLA_float = deg + min / 60.0;
	if (ew_indicator[0] == 'W') {
		longitudeLLA_float *= -1; // If west, make longitude negative
	}

	// Assign altitude
	//altitudeLLA_float = atof(altitudeLLA_str);
	
	//get string rep
	int integer_part = 0;
	int decimal_part = 0;
	integer_part = (int)latitudeLLA_float;
	decimal_part = (int)((latitudeLLA_float - integer_part) * 100000);

	if (integer_part < 0) {
		latitudeLLA_str[0] = '-';
	} else{
		latitudeLLA_str[0] = '+';
	}
	latitudeLLA_str[1] = (integer_part / 10) % 10;	//tens
	latitudeLLA_str[2] = integer_part % 10;			//ones
	latitudeLLA_str[3] = '.';						//dec
	latitudeLLA_str[4] = (decimal_part / 10000);	//tenths
	latitudeLLA_str[5] = (decimal_part / 1000);	//hundredths
	latitudeLLA_str[6] = (decimal_part / 100);	//thousandths
	latitudeLLA_str[7] = (decimal_part / 10) % 10;	//ten-thousandths
	latitudeLLA_str[8] = (decimal_part % 10);	//hundred-thousandths


	
	
	//snprintf(longitudeLLA_str, LLA_LAT_BUFFER_SIZE, "%f", longitudeLLA_float);

}