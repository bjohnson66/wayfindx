#ifndef F_CPU
#define F_CPU 8000000UL /**< Define the CPU frequency to 8MHz. */
#endif

#include <avr/interrupt.h>
#include <string.h>
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
char nf_output_message_line1[MAX_COL] = {' '};

//local static
char nmea_msg_id_buffer[NMEA_MSG_ID_SIZE];
gga_struct_type nf_gga_msg = {0};

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
	
	sei(); //UART is interrupt based;	
	memset(nmea_msg_id_buffer,0,sizeof(char)*NMEA_MSG_ID_SIZE); //zeroize
	
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

void read_nmea_msg(){	
	char tempChar;
    do {
	    get_serial_char(&tempChar);
    } while (tempChar != '$');
	
	if (tempChar == '$'){ //if start of id
		memset(nmea_msg_id_buffer,0,sizeof(char)*NMEA_MSG_ID_SIZE); //zeroize buffer
		//read ID
		for (int i = 0; i < NMEA_MSG_ID_SIZE; i++){
			get_serial_char(nmea_msg_id_buffer + i);
		}
		get_serial_char(&tempChar); //eat comma after msg id
		int counter = 0; //common counter to save memory alloc time

		//-----------------------------------------------------------------
		//only need GGA and VTG for requirements. Drop others
		if(strcmp(nmea_msg_id_buffer, GGA_TYPE) == 0){
			memset(nf_output_message_line1,' ',sizeof(char)*MAX_COL); //zeroize debug buffer
			//Pack message
			//--------------------------------------------------
			//load utc data
			get_serial_char(&tempChar); //read first char to see if it is a comma
			if (! (tempChar == ',')){
				nf_gga_msg.utc_time[0] = tempChar;
				for (counter = 1; counter < 10; counter ++){
					get_serial_char(nf_gga_msg.utc_time + counter);
				}
				get_serial_char(&tempChar); //eat comma after utc
			}
			
			//--------------------------------------------------
			//load latitude
			get_serial_char(&tempChar); //read first char to see if it is a comma
			if (! (tempChar == ',')){
				nf_gga_msg.latitude[0] = tempChar;
				for (counter = 1; counter < 9; counter ++){
					get_serial_char(nf_gga_msg.latitude + counter);
				}
				get_serial_char(&tempChar); //eat comma after latitude
			}
			
			//--------------------------------------------------
			//load ns
			get_serial_char(&tempChar); //read first char to see if it is a comma
			if (! (tempChar == ',')){
				nf_gga_msg.ns_indicator[0] = tempChar;
				get_serial_char(&tempChar); //eat comma after ns_indicator
				nf_output_message_line1[0] = nf_gga_msg.ns_indicator[0];
			}
			
			//--------------------------------------------------
			//load longitude
			get_serial_char(&tempChar); //read first char to see if it is a comma
			if (! (tempChar == ',')){
				nf_gga_msg.longitude[0] = tempChar;
				for (counter = 1; counter < 10; counter ++){
					get_serial_char(nf_gga_msg.longitude + counter);
				}
				get_serial_char(&tempChar); //eat comma after longitude
			}
			
			//--------------------------------------------------
			//load ew
			get_serial_char(&tempChar); //read first char to see if it is a comma
			if (! (tempChar == ',')){
				nf_gga_msg.ew_indicator[0] = tempChar;
				get_serial_char(&tempChar); //eat comma after ew_indicator
				nf_output_message_line1[1] = nf_gga_msg.ew_indicator[0];
			}
			
			//--------------------------------------------------
			//load fix indicator
			get_serial_char(&tempChar); //read first char to see if it is a comma
			if (! (tempChar == ',')){
				nf_gga_msg.position_fix_indicator[0] = tempChar;
				get_serial_char(&tempChar); //eat comma after position_fix_indicator
				nf_output_message_line1[2] = nf_gga_msg.position_fix_indicator[0];

			}
			
			//--------------------------------------------------
			//load SVs used
			get_serial_char(&tempChar); //read first char to see if it is a comma
			if (! (tempChar == ',')){
				nf_gga_msg.satellites_used[0] = tempChar; //pos 1
				get_serial_char(nf_gga_msg.satellites_used + 1); //pos 2
				get_serial_char(&tempChar); //eat comma after satellites_used
				nf_output_message_line1[3] = nf_gga_msg.satellites_used[0];
			}
			
			//--------------------------------------------------
			//load HDOP
			get_serial_char(&tempChar); //read first char to see if it is a comma
			if (! (tempChar == ',')){
				nf_gga_msg.hdop[0] = tempChar; //pos 1
				get_serial_char(nf_gga_msg.hdop + 1); //pos 2
				get_serial_char(nf_gga_msg.hdop + 2); //pos 2
				get_serial_char(&tempChar); //eat comma after HDOP
				
			    nf_output_message_line1[4] = nf_gga_msg.hdop[0];
				nf_output_message_line1[5] = nf_gga_msg.hdop[1];
				nf_output_message_line1[6] = nf_gga_msg.hdop[2];
			}
			
			//--------------------------------------------------
			//discard rest of message
			char tempChar = 0;
			while (tempChar != '\r') {
				get_serial_char(&tempChar);
			}
		
		}else if(strcmp(nmea_msg_id_buffer, VTG_TYPE) == 0){				
			char tempChar = 0; //read rest of msg
			while (tempChar != '\r') {
				get_serial_char(&tempChar);
			}
		}else {
			char tempChar = 0;
			//wait for end of message with <CR>
			while (tempChar != '\r') {
				get_serial_char(&tempChar);
			} 
			//ignore <LF>
			
		}
	}
}