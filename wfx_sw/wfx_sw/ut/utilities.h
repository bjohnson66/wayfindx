/**
 * @file utilities.h
 * @brief Header file containing common utility functions and definitions.
 *
 * This file provides declarations for common utility functions and definitions
 * that can be used throughout the project.
 */
#ifndef UTILITIES_H
#define UTILITIES_H

#include "ut_types.h"

#define NOP asm("nop");

#define STAT_MODE 1
#define NAV_MODE 0

#define NUM_OPERATIONS 3
#define SAVE_OP 0
#define CLEAR_OP 1
#define RESET_OP 2

#define MAX_MEM_INDEX 10
#define SAVE_STR  " SAVE"
#define CLEAR_STR "CLEAR"
#define RESET_STR "RESET"

#define NUM_BUTTONS 4
#define MODE_SELECT_BTN PIND5
#define MEM_SELECT_BTN PIND4
#define OP_SELECT_BTN PIND3
#define ACTION_BTN PIND2

#define MIN_THRESHOLD (uint16_t)260   
#define DEBOUNCE_TIME (uint16_t)300   //debounce time = 4*DEBOUNCE_TIME/FCPU

#define DEBUG_LED_PIN PB0
#define DEBUG_LIGHT_ON PORTB |= (1 << DEBUG_LED_PIN);
#define DEBUG_LIGHT_OFF PORTB &= ~(1 << DEBUG_LED_PIN);

extern boolean_t ut_mode;
extern uint8_t ut_operation;
extern uint8_t ut_memory_0idx;

/**
 * @brief Initializes the pins on startup.
 */
void ut_init();

/**
 * @brief Checks status of each button sequentially. Will only take one action if more than one button is pressed.
		  Post polling action takes place in the following order: MODE_SELECT_BTN, MEM_SELECT_BTN, OP_SELECT_BTN, ACTION_BTN
		  The first action to trigger is the only action taken (a virtual short).
 */
void ut_poll_btns();


#endif /* UTILITIES_H */
