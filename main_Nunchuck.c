/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "GPIO.h"               // Keil::Device:GPIO
#include "LPC17xx.h"                    // Device header
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "stdio.h"
#include <string.h>
#define Adresse_Nunchuck 0xA4 >> 1

extern ARM_DRIVER_I2C Driver_I2C2;
extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;
extern ARM_DRIVER_USART Driver_USART1;

void tache1(void const *arg);

void Initialisation_Nunchuck(unsigned char composant, unsigned char registre, unsigned char valeur);
void Conversion_Nunchuck(unsigned char composant, unsigned char commande);
void Init_UART(void);
	
	
void delay_dirty(int n);
	


osThreadId ID_tache1;
osMutexId ID_mut_GLCD;

osThreadDef (tache1,osPriorityNormal,1,0);
osMutexDef (mut_GLCD);

void Init_I2C(void){
	Driver_I2C2.Initialize(NULL);
	Driver_I2C2.PowerControl(ARM_POWER_FULL);
	Driver_I2C2.Control(	ARM_I2C_BUS_SPEED,				// 2nd argument = débit
							ARM_I2C_BUS_SPEED_STANDARD  );	// 100 kHz
	Driver_I2C2.Control(	ARM_I2C_BUS_CLEAR,
							0 );
}

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

void tache1(void const *arg)
{
	char texte[100],envoi;
	char Y,X,C;
	uint8_t Nunchuck[6];
	while (1)
	{
		///conversion///
		delay_dirty(1);
		Conversion_Nunchuck(Adresse_Nunchuck,0x00);
		
		////////////////
		delay_dirty(1);
		Driver_I2C2.MasterReceive(Adresse_Nunchuck,Nunchuck,6,false);
		while (Driver_I2C2.GetStatus().busy == 1);	// attente fin transmission
		
    
		
		Y=Nunchuck[1];
		X=Nunchuck[0];
		
    sprintf(texte,"Y : %3d  X: %4d",Y,Nunchuck[0]);     
		GLCD_DrawString(1,100,texte);
		
		osMutexWait(ID_mut_GLCD,osWaitForever);
		while(Driver_USART1.GetStatus().tx_busy ==1);    //section critique
		Driver_USART1.Send(&Nunchuck[1],1);
		
		while(Driver_USART1.GetStatus().tx_busy ==1);    //section critique
		Driver_USART1.Send(&Nunchuck[0],1);
		osMutexRelease(ID_mut_GLCD);
	}
}


/*
 * main: initialize and start the system
 */
int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS
  Init_I2C();
	Init_UART();
	 GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
  // initialize peripherals here
  ID_tache1 = osThreadCreate(osThread(tache1),NULL);    //tache 1
  ID_mut_GLCD = osMutexCreate(osMutex(mut_GLCD));       //mutex GLCD
	
	delay_dirty(1);
	Initialisation_Nunchuck(Adresse_Nunchuck,0xF0,0x55);

	delay_dirty(1);
	Initialisation_Nunchuck(Adresse_Nunchuck,0xFB,0x00);
  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);
  osKernelStart ();                         // start thread execution 
	osDelay(osWaitForever);
}

void Initialisation_Nunchuck(unsigned char composant, unsigned char registre, unsigned char valeur)
{
	uint8_t tab[2];
	tab[0]=registre;
	tab[1]=valeur;
	
	Driver_I2C2.MasterTransmit (composant, tab, 2, false);		// false = avec stop
		while (Driver_I2C2.GetStatus().busy == 1);	// attente fin transmission

}
void Conversion_Nunchuck(unsigned char composant, unsigned char commande)
{
	Driver_I2C2.MasterTransmit (composant, &commande, 1, false);		// false = avec stop
	while (Driver_I2C2.GetStatus().busy == 1);	// attente fin transmission
		
}


void delay_dirty(int n)
{
 volatile int d;
 for (d=0; d<n*3000; d++){}
}

