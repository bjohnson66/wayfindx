/**
 * @file utilities.h
 * @brief Header file containing common utility functions and definitions.
 *
 * This file provides declarations for common utility functions and definitions
 * that can be used throughout the project.
 * @version 1.0
 * @copyright (C) 2024 Bradley Johnson and Abele Atresso
 */

#ifndef UTILITIES_H
#define UTILITIES_H

#include "ut_types.h"
#include "../nf/nf_types.h"

#define NOP asm("nop"); /**< No operation macro. */

#define STAT_MODE 1 /**< Mode indicating status. */
#define NAV_MODE 0  /**< Mode indicating navigation. */

#define NUM_OPERATIONS 3 /**< Number of available operations. */
#define SAVE_OP 0  /**< Save operation index. */
#define CLEAR_OP 1 /**< Clear operation index. */
#define RESET_OP 2 /**< Reset operation index. */

#define MAX_MEM_INDEX 10 /**< Maximum memory index. */
#define SAVE_STR  " SAVE" /**< Save operation string. */
#define CLEAR_STR "CLEAR" /**< Clear operation string. */
#define RESET_STR "RESET" /**< Reset operation string. */

#define NUM_BUTTONS 4 /**< Number of buttons. */
#define MODE_SELECT_BTN PINC0 /**< Mode select button pin. */
#define MEM_SELECT_BTN PINC1 /**< Memory select button pin. */
#define OP_SELECT_BTN PINC2 /**< Operation select button pin. */
#define ACTION_BTN PINC3 /**< Action button pin. */

#define ON_TIME_THRESHHOLD (uint16_t)100   /**< Button press threshold time. */
#define RESET_TIME_THRESHHOLD (uint16_t)30  /**< Button release threshold time. */

#define DDR_SPI DDRB
#define PORT_SPI PORTB
#define CS PINB2
#define MOSI PINB4
#define MISO PINB5
#define SCK PINB3

// macros
#define  CS_ENABLE()	PORT_SPI &= ~(1 << CS)
#define  CS_DISABLE()   PORT_SPI |= (1<< CS)

extern boolean_t ut_mode; /**< Current mode indicator. */
extern uint8_t ut_operation; /**< Current operation index. */
extern uint8_t ut_memory_0idx; /**< Current memory index. */
extern float ut_lat_mem_floats[MAX_MEM_INDEX]; /**< Array to store latitude memory floats. */
extern float ut_long_mem_floats[MAX_MEM_INDEX]; /**< Array to store longitude memory floats. */
extern char ut_lat_mem_str[LLA_LAT_BUFFER_SIZE]; /**< Array to store latitude memory strings. */
extern char ut_long_mem_str[LLA_LONG_BUFFER_SIZE]; /**< Array to store longitude memory strings. */

/**
 * @brief Initializes the pins for buttons, loads from SD card, and initializes stored locations on startup.
 */
void ut_init();

/**
 * @brief Checks status of each button sequentially.
 * Action triggers on button release. Is to be called in background via interrupt.
 * Post-polling action takes place in the following order: MODE_SELECT_BTN, MEM_SELECT_BTN, OP_SELECT_BTN.
 */
void ut_poll_btns();


#endif /* UTILITIES_H */
