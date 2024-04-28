/**
 * @file nf.h
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


/**
 * @brief Initialize the navigation fetch module.
 * @return 0 if initialization is successful
 */
uint8_t nf_init();

/**
 * @brief Read a single character from the serial communication.
 * @param outputchar Pointer to the character variable to store the read character.
 */
void get_serial_char(char* outputchar);

/**
 * @brief Read a raw NMEA message from the serial communication and extracts relevant data.
 */
void read_nmea_msg_raw();

/**
 * @brief Convert NMEA format coordinates to Latitude, Longitude, and Altitude (LLA) format.
 */
void convertNMEAtoLLA();


#endif /* NF_H_ */