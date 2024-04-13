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

//local static
char nmea_msg_id_buffer[NMEA_MSG_ID_SIZE];
char gga_msg_buffer[GGA_SIZE];

gga_struct_type nf_gga_msg;

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
	memset(&nf_gga_msg, 0, sizeof(gga_struct_type));
		
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

void process_raw_gga(int* offset, const int data_size, char* buffer){
		for(int i = 0; i < data_size; i ++){
			nf_gga_msg.utc_time[i] = gga_msg_buffer[i + *offset];
		}
	    *offset += data_size;
		*offset += 1; //eat comma
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
		//memset(gga_msg_buffer, ' ',sizeof(char)*GGA_SIZE); //zeroize msg buffer

		//scrape raw
		for (counter = 0; counter < MAX_COL; counter ++){
			get_serial_char(gga_msg_buffer+counter);
		}
		/*
		//process
		int offset = 0;
		if (gga_msg_buffer[offset] != ','){
			process_raw_gga(&offset, GGA_UTC_BUFFER_SIZE, nf_gga_msg.utc_time);
		}else{
			offset++;
		}
		*/
		
		ds_print_string(gga_msg_buffer, MAX_COL, 1);

	}
}