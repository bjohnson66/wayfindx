#ifndef F_CPU
#define F_CPU 8000000UL /**< Define the CPU frequency to 8MHz. */
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
char longitude[GGA_LONG_BUFFER_SIZE];				// Longitude, e.g., "12158.3416"
char ew_indicator[GGA_INDICATOR_SIZE];				// E/W Indicator, 'E' for east or 'W' for west
char position_fix_indicator[GGA_INDICATOR_SIZE];	// Position Fix Indicator, see Table 1-4
char satellites_used[GGA_SV_USD_BUFFER_SIZE];		// Satellites Used, range 0 to 12 eg 07
char hdop[GGA_HDOP_BUFFER_SIZE];					// HDOP (Horizontal Dilution of Precision), e.g., "1.0"
char ns_indicator[GGA_INDICATOR_SIZE];				// N/S Indicator, 'N' for north or 'S' for south


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
	memset(utc_time, 0, GGA_UTC_BUFFER_SIZE * sizeof(char));
	memset(latitude, 0, GGA_LAT_BUFFER_SIZE * sizeof(char));
	memset(longitude, 0, GGA_LONG_BUFFER_SIZE * sizeof(char));
	memset(ew_indicator, 0, GGA_INDICATOR_SIZE * sizeof(char));
	memset(position_fix_indicator, 0, GGA_INDICATOR_SIZE * sizeof(char));
	memset(satellites_used, 0, GGA_SV_USD_BUFFER_SIZE * sizeof(char));
	memset(hdop, 0, GGA_HDOP_BUFFER_SIZE * sizeof(char));
	memset(ns_indicator, 0, GGA_INDICATOR_SIZE * sizeof(char));

	
	
	
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
				char* output = "NF Frame Error! ";
				ds_print_string(output, MAX_COL, 0);
			}
			if ( c & UART_OVERRUN_ERROR )
			{
				/* 
					* Overrun, a character already present in the UART UDR register was 
					* not read by the interrupt handler before the next character arrived,
					* one or more received characters have been dropped
					*/
				char* output = "           OR ER";
				ds_print_string(output, MAX_COL, 0);
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
		
	}
}