#ifndef F_CPU
#define F_CPU 4000000UL /**< Define the CPU frequency to 8MHz. */
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
gga_struct_type nf_gga_msg = {0};

//local function declarations
void gga_init_from_buffer(const char *buffer);

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
	/*
    * Get received character from ringbuffer
    * uart_getc() returns in the lower byte the received character and 
    * in the higher byte (bitmask) the last receive error
    * UART_NO_DATA is returned when no data is available.
    *
    */
	c = uart_getc();
	if ( c & UART_NO_DATA )
    {
        /* 
            * no data available from UART 
        */
		//char * output = "NF No Data!     ";
		//ds_print_string(output, MAX_COL, 1);
    }
	else
    {
        /*
            * new data available from UART
            * check for Frame or Overrun error
            */
        if ( c & UART_FRAME_ERROR )
        {
            /* Framing Error detected, i.e no stop bit detected */
			//char* output = "NF Frame Error! ";
			//ds_print_string(output, MAX_COL, 1);
        }
        if ( c & UART_OVERRUN_ERROR )
        {
            /* 
                * Overrun, a character already present in the UART UDR register was 
                * not read by the interrupt handler before the next character arrived,
                * one or more received characters have been dropped
                */
			char* output = "           OR ER";
			ds_print_string(output, MAX_COL, 1);
        }
        if ( c & UART_BUFFER_OVERFLOW )
        {
            /* 
                * We are not reading the receive buffer fast enough,
                * one or more received character have been dropped 
                */
			char* output = "OF ER           ";
			ds_print_string(output, MAX_COL, 1);
        }
        /* 
            * send received character back
            */
		*outputchar = (unsigned char)c;
    }
	
}

void read_nmea_msg(){
	memset(nmea_msg_id_buffer,0,sizeof(char)*NMEA_MSG_ID_SIZE); //zeroize buffer
	char tempChar = 0;
	get_serial_char(&tempChar);
	if (tempChar == '$'){ //if start of id
		//read ID
		for (int i = 0; i < NMEA_MSG_ID_SIZE; i++){
			get_serial_char(nmea_msg_id_buffer + i);
		}
		//only need GCA and VTG for requirements. Drop others
		if(strcmp(nmea_msg_id_buffer, GGA_TYPE)){
			//debug print GCA on bottom right
			{
				char debug_msg[16] = "             GGA";
				ds_print_string(debug_msg, 16, 1);
			}
			char buffer[GGA_SIZE] = {0};
			for(int i = 0; i < GGA_SIZE; i++){
				get_serial_char(buffer +i); //load bytes into struct
			}
			gga_init_from_buffer(buffer);
			ds_print_string(nf_gga_msg.position_fix_indicator, 1, 0);
			ds_print_string(nf_gga_msg.longitude, 10, 1);
			//read rest of message
			char tempChar = 0;
			while (tempChar != '\r') {
				get_serial_char(&tempChar);
			}

		}else if(strcmp(nmea_msg_id_buffer, VTG_TYPE)){
			//debug print GCA on bottom right
			{
				char debug_msg[16] = "          VTG   ";
				ds_print_string(debug_msg, 16, 1);
			}
						
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

void scanBuffer(int value, const char* bufferIn, char* bufferOut){
	for (int i =0; i < value; i++){
		bufferOut[i] = bufferIn[i];
	}
}

// Function to initialize struct members from a character buffer
void gga_init_from_buffer(const char *buffer) {
	// Use sscanf to parse the buffer and assign values to struct members
	scanBuffer(10, buffer, nf_gga_msg.utc_time);
	scanBuffer(9, buffer+10, nf_gga_msg.latitude);
	scanBuffer(1, buffer+19, nf_gga_msg.ns_indicator);
	scanBuffer(10, buffer+20,nf_gga_msg.longitude);
	scanBuffer(1, buffer+30, nf_gga_msg.ew_indicator);
	scanBuffer(1, buffer+31, nf_gga_msg.position_fix_indicator);
	scanBuffer(2, buffer+32, nf_gga_msg.satellites_used);
	scanBuffer(3, buffer+34, nf_gga_msg.hdop);	
}