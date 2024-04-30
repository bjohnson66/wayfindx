/**
 * @file sd.h
 * @brief Header file containing common functions and definitions for the SD card module.
 *
 * This file provides declarations for common utility functions and definitions associated with the SD card module.
 */

#ifndef SD_H_
#define SD_H_

#include "../ut/ut_types.h"

#define SD_INIT_SUCCESS 0
#define SD_INIT_FAILURE 1

/**
 * @brief Initialize the SD card module.
 * @return 0 if initialization is successful
 */
uint8_t sd_init();

/**
 * @brief Read a position from the SD card.
 * @param index The index of the position to read.
 * @param longitude Pointer to the variable to store the longitude.
 * @param latitude Pointer to the variable to store the latitude.
 */
void sd_read_position(uint8_t index, float *longitude, float *latitude);

/**
 * @brief Write a position to the SD card.
 * @param index The index to write the position to.
 * @param longitude The longitude of the position.
 * @param latitude The latitude of the position.
 */
void sd_write_position(uint8_t index, float longitude, float latitude);

#endif /* SD_H_ */
