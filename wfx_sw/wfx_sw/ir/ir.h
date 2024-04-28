/**
 * @file ir.h
 * @brief Header file containing common functions and definitions for the interrupt system 'computer software component" or CSC.
 *
 * This file provides declarations for common utility functions and definitions associated with the interrupt system.
 */

#ifndef IR_H_
#define IR_H_

#include "../ut/ut_types.h"

//globals
extern boolean_t ir_trigger_1hz_flag_g;  /**< Global flag indicating 1Hz trigger */
extern uint16_t ir_sec_counter; /**< Seconds counter for indicating TTFF */

/**
 * @brief Initializes interrupt system functionality.
 * 
 * This function configures Timer2 for time management and Timer 0 for background button polling.
 */
void ir_init();


#endif /* IR_H_ */