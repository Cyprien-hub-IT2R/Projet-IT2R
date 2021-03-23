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
#define Adresse_Nunchuck 0xA4 >> 1

extern ARM_DRIVER_I2C Driver_I2C0;

void tache1(void const *arg);
void tache2(void const *arg);

void Initialisation_Nunchuck(unsigned char composant, unsigned char registre, unsigned char valeur);
void Lecture_Data_Nunchuck(void);
int Conversion_Nunchuck(unsigned char composant);
	
void delay_dirty(int n);
	
uint8_t Nunchuck[6];

osThreadId ID_tache1;

osThreadDef (tache1,osPriorityNormal,1,0);

void Init_I2C(void){
	Driver_I2C0.Initialize(NULL);
	Driver_I2C0.PowerControl(ARM_POWER_FULL);
	Driver_I2C0.Control(	ARM_I2C_BUS_SPEED,				// 2nd argument = débit
							ARM_I2C_BUS_SPEED_STANDARD  );	// 100 kHz
	Driver_I2C0.Control(	ARM_I2C_BUS_CLEAR,
							0 );
}

void tache1(void const *arg)
{
	char texte[20];
	int attente;
	while (1)
	{
		///conversion///
		attente=0;
		do{
		attente=Conversion_Nunchuck(Adresse_Nunchuck);
		}while(attente != 1);
		
		delay_dirty(10);
		////////////////
		Lecture_Data_Nunchuck();
		
//			sprintf(texte,"Y : %d",Nunchuck[1]); 
//		  GLCD_DrawString(1,150,texte);
	}
}

/*
 * main: initialize and start the system
 */
int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS
  Init_I2C();
	
  // initialize peripherals here
  ID_tache1 = osThreadCreate(osThread(tache1),NULL);
  
	delay_dirty(10);
	
	Initialisation_Nunchuck(Adresse_Nunchuck,0xF0,0x55);
	delay_dirty(10);
	Initialisation_Nunchuck(Adresse_Nunchuck,0xFB,0x00);
	delay_dirty(10);
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
	
	Driver_I2C0.MasterTransmit (composant, tab, 2, false);		// false = avec stop
		while (Driver_I2C0.GetStatus().busy == 1);	// attente fin transmission

	delay_dirty(10);
}
int Conversion_Nunchuck(unsigned char composant)
{
	int reponse=0;
	
	reponse=0;
	
	Driver_I2C0.MasterTransmit (composant, 0x00, 1, false);		// false = avec stop
	while (Driver_I2C0.GetStatus().busy == 1);	// attente fin transmission
	reponse=1;
		
	delay_dirty(10);
	return reponse;
}

void Lecture_Data_Nunchuck(void)
{
	Driver_I2C0.MasterReceive(Adresse_Nunchuck,Nunchuck,6,false);
	while (Driver_I2C0.GetStatus().busy == 1);	// attente fin transmission
}

void delay_dirty(int n)
{
 volatile int d;
 for (d=0; d<n*3000; d++){}
}