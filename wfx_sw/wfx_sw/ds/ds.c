#include "ds.h"
#include "../lib/lcd.h"
#include "../ut/ut_types.h"
#include <avr/io.h>


/**
 * @brief Prints a string on the LCD display.
 * 
 * @param inputString Pointer to the string to be printed.
 * @param size Size of the string to be printed.
 * @param row indicating what row to place the cursor
 */
void ds_print_string(char * inputString, int size, uint8_t row)
{
	//Make sure size is legit
   if ((size >= 0) && (size <= MAX_COL) && (row >= 0) && (row < MAX_ROWS))
	{
		lcd_gotoxy((uint8_t)0, row); //send to second line
		
		for (int i = 0; i < size; i++)
		{
			lcd_putc(inputString[i]); //put i'th char on display
		}
	}
	return;
}

/**
 * @brief Initializes the LCD display.
 * This function initializes the LCD display and turns on the display.
 */
void ds_init()
{
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	return;
}

/**
 * @brief Clears the LCD display.
 * This function clears the LCD display.
 */
void ds_clear(){
	lcd_clrscr();
}