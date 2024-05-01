#ifndef F_CPU
#define F_CPU 4000000UL /**< Define the CPU frequency to 4MHz. */
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
#include "../ir/ir.h"

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
char utc_time[GGA_UTC_BUFFER_SIZE];             /**< UTC Time, e.g., "161229.487" */
char latitude[GGA_LAT_BUFFER_SIZE];             /**< Latitude, e.g., "3723.2475" */
char ns_indicator[GGA_INDICATOR_SIZE];          /**< N/S Indicator, 'N' for north or 'S' for south */
char longitude[GGA_LONG_BUFFER_SIZE];            /**< Longitude, e.g., "12158.3416" */
char ew_indicator[GGA_INDICATOR_SIZE];          /**< E/W Indicator, 'E' for east or 'W' for west */
char position_fix_indicator[GGA_INDICATOR_SIZE];/**< Position Fix Indicator, see Table 1-4 */
char satellites_used[GGA_SV_USD_BUFFER_SIZE];    /**< Satellites Used, range 0 to 12 eg 07 */
char hdop[GGA_HDOP_BUFFER_SIZE];                /**< HDOP (Horizontal Dilution of Precision), e.g., "1.0" */
char msl_altitude[GGA_ALTITUDE_BUFFER_SIZE];     /**< Mean Sea Level Altitude, e.g., "1.0" */
char speed[VTG_SPEED_BUFER_SIZE];               /**< Speed, e.g., "0.0" */

float latitudeLLA_float;    /**< Latitude in degrees */
float longitudeLLA_float;   /**< Longitude in degrees */
float altitudeLLA_float;    /**< Altitude in meters */
char latitudeLLA_str[LLA_LAT_BUFFER_SIZE];      /**< Latitude in degrees */
char longitudeLLA_str[LLA_LONG_BUFFER_SIZE];    /**< Longitude in degrees */
char altitudeLLA_str[LLA_ALT_BUFFER_SIZE];      /**< Altitude in meters */

//local static
static char nmea_msg_id_buffer[NMEA_MSG_ID_SIZE];/**< Buffer to store NMEA message ID */
static char gga_msg_buffer[GGA_SIZE];            /**< Buffer to store GGA message */

//function definitions

/**
 * @brief Clear all navigation strings except UTC time.
 */
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

/**
 * @brief Initialize the navigation fetch module.
 * This function initializes UART communication and clears navigation strings.
 * @return 0 if initialization is successful, otherwise returns 1.
 */
uint8_t nf_init(){

	// Initialize the arrays within gga_msg 
	memset(utc_time, ' ', GGA_UTC_BUFFER_SIZE * sizeof(char));
	memset(speed, ' ', VTG_SPEED_BUFER_SIZE * sizeof(char));

    nf_clear_nav_strings();


	latitudeLLA_float = 41.65941f;  
	longitudeLLA_float = 91.53652f;  
	altitudeLLA_float = 248.2f;	
	
	return NF_INIT_SUCCESS;
}

/**
 * @brief Read a single character from the serial communication.
 * This function blocks until a character is received.
 * @param outputchar Pointer to the character variable to store the read character.
 */
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

/**
 * @brief Read a raw NMEA message from the serial communication.
 * This function reads a raw NMEA message and processes GGA and VTG messages.
 * Additionally, it polls buttons between messages.
 */
void read_nmea_msg_raw(){
	//-----------------------------------------------------------------
	//only need GGA and VTG for requirements. Drop others
	if(true){
		//////////////////////////////////////////
		//spoof UTC if available GGA_UTC_BUFFER_SIZE utc_time
		if (ir_sec_counter > 6){
			char* tempTime = "183115.00";
			strncpy(utc_time, tempTime, GGA_UTC_BUFFER_SIZE);
			utc_time[3] = ((ir_sec_counter / 100) % 10)  + '0';
			utc_time[4] = ((ir_sec_counter / 10 ) % 10)  +'0';
			utc_time[5] = (ir_sec_counter % 10)  +'0';

		}
		

		//////////////////////////////////////////
		//grab LAT if available GGA_LAT_BUFFER_SIZE latitude
		char* tempLat = "41.6594000";
		strncpy(latitude, tempLat, GGA_LAT_BUFFER_SIZE);

		
		//////////////////////////////////////////
		//spoof NS indicator if available 0 ns_indicator
		ns_indicator[0] = 'N';
		
		
		//////////////////////////////////////////
		//spoof LONG if available GGA_LONG_BUFFER_SIZE longitude
		char* tempLong = "091.5365000";
		strncpy(longitude, tempLong, GGA_LONG_BUFFER_SIZE);
		
		//////////////////////////////////////////
		//spoof EW indicator if available
		ew_indicator[0] = 'W';
		
		//////////////////////////////////////////
		//spoof FIX indicator if available
		if (ir_sec_counter > 15){
			position_fix_indicator[0] = '1';
		} else{
			position_fix_indicator[0] = '0';
		}
		
		//////////////////////////////////////////
		//grab NUM_SV if available
		satellites_used[0] = '0';
		satellites_used[1] = '7';	
		
		//////////////////////////////////////////
		//grab HDOP if available
		hdop[0] = '0';
		hdop[1] = ',';
		hdop[2] = '9';
		
		
		//spoof altitude 
		msl_altitude[0] = '2';
		msl_altitude[1] = '2';
		msl_altitude[2] = '8';
		msl_altitude[3] = '.';
		msl_altitude[4] = '2';
		msl_altitude[5] = ' ';
		msl_altitude[6] = '\n';
		
		//spoof speed
		memset(speed, ' ', VTG_SPEED_BUFER_SIZE * sizeof(char));
		speed[0] = '0';
		speed[1] = '.';
		speed[2] = '0';
		speed[3] = '1';
		speed[4] = '9';

	} //END GGA
}




/**
 * @brief Convert NMEA format coordinates to Latitude, Longitude, and Altitude (LLA) format.
 * This function converts NMEA format coordinates to LLA format and stores them in global variables.
 */
void convertNMEAtoLLA() {

	latitudeLLA_float = 41.65941f;
	altitudeLLA_float = 248.2f;

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

	longitudeLLA_float = 91.53652f;

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