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

#define STAT_MODE 2
#define NAV_MODE 1

#define SAVE_OP 0
#define CLEAR_OP 1
#define RESET_OP 3

#define SAVE_STR  " SAVE"
#define CLEAR_STR "CLEAR"
#define RESET_STR "RESET"


extern boolean_t ut_mode;
extern uint8_t ut_operation;
extern uint8_t ut_memory_0idx;

/**
 * @brief Initializes the pins on startup.
 */
void ut_init();

#endif /* UTILITIES_H */
