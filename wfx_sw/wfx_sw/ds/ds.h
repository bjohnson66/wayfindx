/**
 * @file ds.h
 * @brief Header file containing common functions and definitions for the display 'computer software component" or CSC.
 *
 * This file provides declarations for common utility functions and definitions associated with the LCD display.
 */

#ifndef DS_H_
#define DS_H_

#define MAX_ROWS 2
#define MAX_COL 16

#include "../ut/ut_types.h"

/**
 * @brief Prints a string on the LCD display.
 * 
 * @param inputString Pointer to the string to be printed.
 * @param size Size of the string to be printed.
 * @param row indicating what row to place the cursor
 */
void ds_print_string(char * inputString, int size, uint8_t row);

/**
 * @brief Uses library to initialize display. If DEBUG is defined, cursor will blink. 
 *		  Otherwise, cursor will be off.
 *
 * @return none
 */
void ds_init();

/**
 * @brief Clears the display.
 */
void ds_clear();


#endif /* DS_H_ */