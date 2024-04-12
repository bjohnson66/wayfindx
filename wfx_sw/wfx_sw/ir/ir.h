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
 * @brief Initializes infrared sensor functionality.
 * - Enables button interrupt (INT0) with falling edge trigger.
 * - Initializes fan output and timer for PWM fan control.
 * - Initializes pin change interrupt for RPG.
 * - Configures Timer1 for RPM measurement.
 * - Sets up fan input interrupts.
 */
void ir_init();


void test_ir_display();

#endif /* IR_H_ */