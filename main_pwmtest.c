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
	int INA,INB;
	char recuN[6],vitesse, recuY,recuX,X;
	signed char Y;
	int PWM;
	while(1)
	{
		Driver_USART1.Receive(&recuY,1);
		while (Driver_USART1.GetRxCount() < 1);
		
		Driver_USART1.Receive(&recuX,1);
		while (Driver_USART1.GetRxCount() < 1);
		
		GLCD_DrawString(1,1,GLCD);
		
		Y = recuY - 124;
		X = recuX;
		
		if(Y<0)
		{
		GPIO_PinWrite(0,16,false); //INA
		GPIO_PinWrite(0,17,true); //INB	
		direction[0]='r';
		}
		else if (Y>0)
		{
		GPIO_PinWrite(0,16,true); //INA
		GPIO_PinWrite(0,17,false); //INB	
		direction[0]='a';
		}
		else
		{
			Y=0;
		}
		
		if (Y > 95)
		{
			Y = 95;
		}
		else if (Y < -95)
		{
			Y = -95;
		}
		vitesse=abs(Y);
		PWM = (int) (vitesse*2499)/95;
		if (PWM > 2499)
		{
			PWM = 2499;
		}
		
		LPC_PWM1->MR2 = PWM/2;    // ceci ajuste la duree de l'etat haut ; mr2 pour P3.25
		
//		sprintf(GLCD,"Y :%3d   rc:%4d",Y,PWM);
//		GLCD_DrawString(1,1,GLCD);
		sprintf(GLCD,"Y :%3d   X: %3d",Y,X);
		GLCD_DrawString(1,1,GLCD);
		GLCD_DrawString(1,50,direction);
		osDelay(1000);

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
	
	GPIO_SetDir(0,16,GPIO_DIR_OUTPUT); //INA (0.16)et INB(0.18) en sortie
	GPIO_SetDir(0,17,GPIO_DIR_OUTPUT);

LPC_SC->PCONP = LPC_SC->PCONP | 0x00000040;   // enable PWM1
	LPC_PWM1->PR = 0;  // prescaler
		

  LPC_PWM1->MR0 = 2499;    // Ceci ajuste la p?riode de la PWM ? 48 us
	LPC_PWM1->MR2 = 0;    // ceci ajuste la duree de l'etat haut ; mr2 pour P3.25
	
	LPC_PINCON->PINSEL7 |= (3<<18); //  P3.25 est la sortie PWM Channel 3 de Timer 1
	
	LPC_PWM1->MCR = LPC_PWM1->MCR | 0x00000002; // Timer relance quand MR0 repasse à 0
	LPC_PWM1->LER = LPC_PWM1->LER | 0x00000005;  // ceci donne le droit de modifier dynamiquement la valeur du rapport cyclique
	                                             // bit 0 = MR0    bit2 = MR2
	LPC_PWM1->PCR |= (1<<10);  // autorise la sortie PWM
	                                
  LPC_PWM1->TCR = 1;  /*validation de timer 1 et reset counter */


ID_tache1 = osThreadCreate(osThread(tache1),NULL);


  osKernelStart ();                         // start thread execution 
	osDelay(osWaitForever);
}
