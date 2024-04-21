#include "utilities.h"

boolean_t ut_mode;
uint8_t ut_operation;
uint8_t ut_memory_0idx;

/*
 *@brief ut_init initializes what ut needs on startup
 *
 *@return none
*/
void ut_init()
{
	ut_mode = NAV_MODE;
	ut_operation = SAVE_OP;
	ut_memory_0idx = 0;
	return;
}