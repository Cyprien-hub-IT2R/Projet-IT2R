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

void Timer_Init(void);



float a;
char etat=0;
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
	char vitesseX, recuY,recuX,X2;
	int PWM_X;
	char etat = 0;
	LPC_TIM0->MR0 = 500000;    // Ceci ajuste la p?riode de la PWM ? 20 ms
	
	while(1)
	{
		
	
		
		Driver_USART1.Receive(&recuY,1);
		while (Driver_USART1.GetRxCount() < 1);
		
		Driver_USART1.Receive(&recuX,1);
		while (Driver_USART1.GetRxCount() < 1);
		recuX=234-recuX+32;
		a=((recuX-31.0)/4000.0)+0.05;
		
		sprintf(GLCD, "%d %f",recuX,a);
		GLCD_DrawString(1,1,GLCD);
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
	
	Timer_Init();
	
  NVIC_SetPriority(TIMER0_IRQn,0);
	NVIC_EnableIRQ(TIMER0_IRQn);
	
  ID_tache1 = osThreadCreate(osThread(tache1),NULL);


  osKernelStart ();                         // start thread execution 
	osDelay(osWaitForever);
}

void Timer_Init(void)
	{
	LPC_SC->PCONP |= (1<<1);   // enable MAT0
	LPC_TIM0->PR = 0;  // prescaler
	LPC_PINCON->PINSEL7 |= (1<<21); //  P3.26 est la sortie MAT0.1
	LPC_TIM0->MCR |= 3;     //RAZ du compteur MR si correspondence avec MR0 et MR1, et interruption
	LPC_TIM0->EMR |= (3<<6) ;    //inverse la sortie MAT0.1, mettre 11 dans les bits 6 et 7 
  LPC_TIM0->TCR = 1;        /*validation de timer 1 et reset counter */
} 

void TIMER0_IRQHandler(void)
{
	static int mr0;
	Allumer_1LED(1);
	LPC_TIM0->IR=1;
	if (etat==0){
			mr0=500000*a;
			etat=1;
	}
	else if (etat==1){
		mr0=500000-(500000*a);
		etat=0;
	}
	LPC_TIM0->MR0 = mr0;
}
