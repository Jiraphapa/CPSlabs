#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"
#include "led.h"

#include <unistd.h>




/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT); 

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);  


/**
  * @desc decimal to binary conversion
  * @return pointer array of int
  * @misc Input must be in [0,31] range
*/

//SHUBU note: decimal to binary function, return pointer array of int !!! HARM INPUT GERN dec of 31 !!!
int* decToBin(int decNum) {
	int temp = decNum;
	int count = 4;
	static int result[5] = { 0,0,0,0,0 };
	while (temp != 0) {
		if (temp % 2 == 0) {
			result[count] = 0;
		}
		else {
			result[count] = 1;
		}
		temp = temp / 2;
		count--;
	}

	
	return &result[0];
}

/* EXAMPLE of how to use dec to bin, Nai %d value ja ork ma tee la tua

	int* result;
	result = decToBin(4);

	int i;
	for (i = 0; i < 5; i++) {
		printf("%d", *(result + i));
	}
	
*/


void main(){

	/**
	 * Task 1
	*/

	//Init LCD
	__C30_UART=1;	
	lcd_initialize();
	lcd_clear();
	lcd_locate(0,0);
	printf("Patipon \n");
 
    
    lcd_locate(0,2);
    printf("Jiraphapa \n");
    
	/**
	 * Task 2 & 3
	*/

	led_initialize();

	lcd_locate(0,4);
	int counter = 0;
	int i;
	for (counter = 0 ; counter<32; counter++)
	{
		// Task 3: Print the current counter value to the LCD
		printf("%d \n", counter);
		// Task 2: Use a counter variable and print its current value to all the 5 LEDs.
		int* result;
		// Convert to binary then assign value to each LED
		result = decToBin(counter);

		/* Setting individual pins to either 0 or 1 */
		// LED1_PORT = *(result + 0);
		// Nop();
		led_set(1,*(result + 0));
		led_set(2,*(result + 1));
		led_set(3,*(result + 2));
		led_set(4,*(result + 3));
		led_set(5,*(result + 4));
		
		sleep(1);
		
	}

	
	while(1){
		
	}
}

