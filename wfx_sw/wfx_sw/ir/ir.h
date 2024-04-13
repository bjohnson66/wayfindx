/**
 * @file ir.h
 * @brief Header file containing common functions and definitions for the interrupt system 'computer software component" or CSC.
 *
 * This file provides declarations for common utility functions and definitions associated with the interrupt system
 */

#ifndef IR_H_
#define IR_H_

#include "../ut/ut_types.h"

//globals
extern boolean_t ir_trigger_1hz_flag_g;
extern uint16_t ir_test_counter;

/**
 * @brief Initializes interrupt system functionality.
 * - Configures Timer2 for time management.
 */
void ir_init();


#endif /* IR_H_ */