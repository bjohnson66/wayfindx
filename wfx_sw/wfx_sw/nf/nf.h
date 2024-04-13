/**
 * @file nv.h
 * @brief Header file containing common functions and definitions for the navigation fetch (NV) CSC.
 *
 * This file provides declarations for common utility functions and definitions associated with NV
 */

#ifndef NF_H_
#define NF_H_

#include "../ut/ut_types.h"
#include "../ds/ds.h"

#define NF_INIT_SUCCESS 0
#define NF_INIT_FAILURE 1

//globals
extern char nf_output_message_line1[MAX_COL];


/**
 * @brief Initializes serial for NMEA inputs from NEO6-M GPS receiver Module
 *		  to facilitate navigation fetch
 */
uint8_t nf_init();

void get_serial_char(char* outputchar);

void read_nmea_msg_raw();


#endif /* NF_H_ */