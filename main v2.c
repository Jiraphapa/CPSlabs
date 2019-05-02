#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"
#include "led.h"


#define LEDTRIS TRISA
#define LED1_TRIS TRISAbits.TRISA4
#define LED1_PORT PORTAbits.RA4
#define LED1 4

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT); 

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);  

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
	//Init LCD
	__C30_UART=1;	
	lcd_initialize();
	lcd_clear();
	lcd_locate(0,0);
	printf("Patipon \n");
 
    
    lcd_locate(0,4);
    printf("Jiraphapa \n");
    
    //CLEARBIT(LED1_TRIS)
    //SETBIT(LED1_TRIS);
    CLEARLED(LED1_TRIS);
    SETLED(LED1_PORT);
   
	
	while(1){
		
	}
}

