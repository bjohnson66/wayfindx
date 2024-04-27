#ifndef F_CPU
#define F_CPU 4000000UL /**< Define the CPU frequency to 8MHz. */
#endif

#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <stdint-gcc.h>
#include "nf.h"
#include "nf_types.h"
#include "../lib/uart.h"
#include "../ds/ds.h"
#include "../ut/utilities.h"

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
char msl_altitude[GGA_ALTITUDE_BUFFER_SIZE];					// HDOP (Horizontal Dilution of Precision), e.g., "1.0"
char speed[VTG_SPEED_BUFER_SIZE];

float latitudeLLA_float;   // Latitude in degrees
float longitudeLLA_float;  // Longitude in degrees
float altitudeLLA_float;   // Altitude in meters
char latitudeLLA_str[LLA_LAT_BUFFER_SIZE];   // Latitude in degrees
char longitudeLLA_str[LLA_LONG_BUFFER_SIZE];  // Longitude in degrees
char altitudeLLA_str[LLA_ALT_BUFFER_SIZE];   // Altitude in meters

//local static
static char nmea_msg_id_buffer[NMEA_MSG_ID_SIZE];
static char gga_msg_buffer[GGA_SIZE];


//function definitions

//Clear all strings but utc time
void nf_clear_nav_strings(){
	    memset(nmea_msg_id_buffer,0,sizeof(char)*NMEA_MSG_ID_SIZE); //zeroize
	    memset(gga_msg_buffer, ' ',sizeof(char)*GGA_SIZE); //zeroize msg buffer
	    memset(latitude, ' ', GGA_LAT_BUFFER_SIZE * sizeof(char));
	    memset(ns_indicator, ' ', GGA_INDICATOR_SIZE * sizeof(char));
	    memset(longitude, ' ', GGA_LONG_BUFFER_SIZE * sizeof(char));
	    memset(ew_indicator, ' ', GGA_INDICATOR_SIZE * sizeof(char));
	    memset(position_fix_indicator, ' ', GGA_INDICATOR_SIZE * sizeof(char));
	    memset(satellites_used, ' ', GGA_SV_USD_BUFFER_SIZE * sizeof(char));
	    memset(hdop, ' ', GGA_HDOP_BUFFER_SIZE * sizeof(char));
	    memset(msl_altitude, ' ', GGA_ALTITUDE_BUFFER_SIZE * sizeof(char));

	    memset(latitudeLLA_str, ' ', LLA_LAT_BUFFER_SIZE * sizeof(char));
	    memset(longitudeLLA_str, ' ', LLA_LONG_BUFFER_SIZE * sizeof(char));
	    memset(altitudeLLA_str, ' ', LLA_ALT_BUFFER_SIZE * sizeof(char));
}

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

	// Initialize the arrays within gga_msg 
	memset(utc_time, ' ', GGA_UTC_BUFFER_SIZE * sizeof(char));
	memset(speed, ' ', VTG_SPEED_BUFER_SIZE * sizeof(char));

    nf_clear_nav_strings();


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
			#ifdef __DEBUG__
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
			#endif

			/* 
			* send received character back
			*/
			*outputchar = (unsigned char)c;
			return;
		}
	}
	
}


void read_nmea_msg_raw(){
	//nf_clear_nav_strings();
	char tempChar;
	do {
		get_serial_char(&tempChar);
	} while (tempChar != '$');

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
		offset++;
		//end of known sizes. Alt is dynamic
		tempChar = 0;
		int i = 0;
		get_serial_char(&tempChar);//eat comma
		get_serial_char(&tempChar);
		while ((tempChar != ',') && (i < GGA_ALTITUDE_BUFFER_SIZE)){
			msl_altitude[i++] = tempChar;
			get_serial_char(&tempChar);
		}
		
		
	} //END GGA
	else if (strcmp(nmea_msg_id_buffer, VTG_TYPE) == 0){
			for (uint8_t comma_counter = 6; comma_counter >0; comma_counter--){
				do{
					get_serial_char(&tempChar);
				} while (tempChar != ',');
			}
			memset(speed, ' ', VTG_SPEED_BUFER_SIZE * sizeof(char));
			//should be at speed now
			int i = 0;
			get_serial_char(&tempChar);//eat comma
			while ((tempChar != ',') && (i < VTG_SPEED_BUFER_SIZE)){
				speed[i++] = tempChar;
				get_serial_char(&tempChar);
			}

	}//end VTG msg

	//poll buttons between messages
	//ut_poll_btns();
}




// Function to convert NMEA format coordinates to LLA form in degrees
void convertNMEAtoLLA() {
	// Convert latitude from NMEA format to degrees
	double deg = 0.0;
	double min = 0.0;

	// Convert the latitude from degrees and minutes to degree decimal
	char tempBuffer[9] = {0};
	for (int i = 0; i < 8; i++){
		tempBuffer[i] = latitude[2+i];
	}
	min = atof(tempBuffer);
	
	deg += (1.0 * (latitude[GGA_LAT_BUFFER_SIZE - 9] - '0'));
	deg += (10.0 * (latitude[GGA_LAT_BUFFER_SIZE - 10] - '0'));

	latitudeLLA_float = deg + (min / 60.0f); // Combine degrees and minutes


	// Extract integer and decimal parts
	uint16_t integer_part = (uint16_t)latitudeLLA_float;
    float fractional_part = latitudeLLA_float - integer_part;
    uint32_t decimal_part = (uint32_t)(fractional_part * 100000);
	
	// Convert integer part to string
	latitudeLLA_str[1] = '0' + ((integer_part / 10) % 10); // Tens
	latitudeLLA_str[2] = '0' + (integer_part % 10); // Ones

	// Decimal point
	latitudeLLA_str[3] = '.';

    // Convert decimal part to string
    latitudeLLA_str[4] = '0' + ((decimal_part / 10000) % 10); // Ten-thousands
    latitudeLLA_str[5] = '0' + ((decimal_part / 1000)% 10); // Thousands
    latitudeLLA_str[6] = '0' + ((decimal_part / 100) % 10); // Hundreds
    latitudeLLA_str[7] = '0' + ((decimal_part / 10) % 10); // Tens
    latitudeLLA_str[8] = '0' + (decimal_part % 10); // Ones
	
	if (ns_indicator[0] =='S'){
		latitudeLLA_str[0] = '-';
		latitudeLLA_float *= -1;
		} else{
		latitudeLLA_str[0] = '+';
	}

	
	//long
	deg = 0.0;
	min = 0.0;

	// Convert the longitude from degrees and minutes to degree decimal
	char tempBuffer2[9] = {0};
	for (int i = 0; i < 8; i++){
		tempBuffer2[i] = longitude[3+i];
	}
	min = atof(tempBuffer2);
	
	deg += (1.0 * (longitude[2] - '0'));
	deg += (10.0 * (longitude[1] - '0'));
	deg += (100.0 * (longitude[0] - '0'));

	longitudeLLA_float = deg + (min / 60.0f); // Combine degrees and minutes

	// Extract integer and decimal parts
	integer_part = (uint16_t)longitudeLLA_float;
	fractional_part = longitudeLLA_float - integer_part;
	decimal_part = (uint32_t)(fractional_part * 100000);

	// Convert integer part to string
	longitudeLLA_str[1] = '0' + ((integer_part / 100) % 10); // Hundreds
	longitudeLLA_str[2] = '0' + ((integer_part / 10) % 10); // Tens
	longitudeLLA_str[3] = '0' + (integer_part % 10); // Ones

	// Decimal point
	longitudeLLA_str[4] = '.';

	// Convert decimal part to string
	longitudeLLA_str[5] = '0' + ((decimal_part / 10000) % 10); // Ten-thousands
	longitudeLLA_str[6] = '0' + ((decimal_part / 1000)% 10); // Thousands
	longitudeLLA_str[7] = '0' + ((decimal_part / 100) % 10); // Hundreds
	longitudeLLA_str[8] = '0' + ((decimal_part / 10) % 10); // Tens
	longitudeLLA_str[9] = '0' + (decimal_part % 10); // Ones

	if (ns_indicator[0] =='S'){
		longitudeLLA_str[0] = '-';
		latitudeLLA_float *= -1;
		} else{
		longitudeLLA_str[0] = '+';
	}

	altitudeLLA_float = atof(msl_altitude); 
}