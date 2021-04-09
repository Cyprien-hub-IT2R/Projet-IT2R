/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "GPIO.h"          
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "stdio.h"
#include "GPIO_LPC17xx.h"
#include <LPC17xx.h>
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include <stdlib.h>
extern ARM_DRIVER_USART Driver_USART1;
extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

void tache1 (void const *arg);


osThreadId ID_tache1;


osThreadDef (tache1,osPriorityNormal,1,0);


void Init_UART(void){
	Driver_USART1.Initialize(NULL);
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	Driver_USART1.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							115200);
	Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART1.Control(ARM_USART_CONTROL_RX,1);
}

void tache1 (void const *arg)  //PWM
{
	char GLCD[20],direction[10];
	char vitesseX, recuY,recuX,X;
	int PWM_X;
	while(1)
	{
		Driver_USART1.Receive(&recuY,1);
		while (Driver_USART1.GetRxCount() < 1);
		
		Driver_USART1.Receive(&recuX,1);
		while (Driver_USART1.GetRxCount() < 1);
		
		GLCD_DrawString(1,1,GLCD);
		vitesseX = recuX;
		
		if ((int)vitesseX <35)
		{
			vitesseX=35;
		}
		else if((int)vitesseX > 230)
		{
			vitesseX=230;
		}
		PWM_X = (int) (vitesseX*(-125))+54375;
		
		LPC_PWM1->MR3 = PWM_X;    // ceci ajuste la duree de l'etat haut ; mr3 pour P3.26
		
//		sprintf(GLCD,"X: %3d, PWM: %5d",vitesseX,PWM_X);
//		GLCD_DrawString(1,1,GLCD);
	
	}
}

/*
 * main: initialize and start the system
 */
int main (void) {
	
  osKernelInitialize ();                    // initialize CMSIS-RTOS
  
	Init_UART();
  Initialise_GPIO();
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	

  LPC_SC->PCONP = LPC_SC->PCONP | 0x00000040;   // enable PWM1
	LPC_PWM1->PR = 0;  // prescaler
		

  LPC_PWM1->MR0 = 499999;    // Ceci ajuste la p?riode de la PWM ? 48 us
	
	LPC_PINCON->PINSEL7 = LPC_PINCON->PINSEL7 | 0x00300000; //  P3.26 est la sortie PWM Channel 3 de Timer 1
	
	LPC_PWM1->MCR = LPC_PWM1->MCR | 0x00000002; // Timer relance quand MR0 repasse à 0
	LPC_PWM1->LER = LPC_PWM1->LER | 0x00000009;  // ceci donne le droit de modifier dynamiquement la valeur du rapport cyclique
	                                             // bit 0 = MR0    bit3 = MR3
	LPC_PWM1->PCR = LPC_PWM1->PCR | 0x00000e00;  // autorise la sortie PWM
	                                
  LPC_PWM1->TCR = 1;  /*validation de timer 1 et reset counter */


ID_tache1 = osThreadCreate(osThread(tache1),NULL);


  osKernelStart ();                         // start thread execution 
	osDelay(osWaitForever);
}
