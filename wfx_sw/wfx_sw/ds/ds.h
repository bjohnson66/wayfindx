/**
 * @file ds.h
 * @brief Header file containing common functions and definitions for the display 'computer software component" or CSC.
 *
 * This file provides declarations for common utility functions and definitions associated with the LCD display.
 * @version 1.0
 * @copyright (C) 2024 Bradley Johnson and Abele Atresso
 */

#ifndef DS_H_
#define DS_H_

#define MAX_ROWS 4 /**< Maximum number of rows on the LCD display */
#define MAX_COL 20 /**< Maximum number of columns on the LCD display */
#define SPACES "                    " /**< String of spaces used for clearing the LCD display */


#include "../ut/ut_types.h"

/**
 * @brief Prints a string on the LCD display.
 * 
 * @param inputString Pointer to the string to be printed.
 * @param size Size of the string to be printed.
 * @param row Row index where the cursor will be placed.
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