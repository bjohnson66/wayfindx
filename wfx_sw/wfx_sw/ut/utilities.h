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

/**
 * @brief Initializes the pins on startup.
 */
void ut_init();

#endif /* UTILITIES_H */
