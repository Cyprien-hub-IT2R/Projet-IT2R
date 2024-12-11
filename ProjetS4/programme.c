#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "GPIO.h"
#include "LPC17xx.h"                    // Device header
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include <stdio.h>
#include <stdlib.h>


// Traduire les trames du LIDAR
// puis faire une interruption lorsque la distance est a moins de 10cm dun obstacle


extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;
extern ARM_DRIVER_USART Driver_USART1;

void Lidar_PWM(void);
void Init_UART(void);
void delaitps(int tps);

osThreadId ID_Tache1;
osThreadId ID_Tache2;
osThreadId ID_Tache3;

void tache1(void const * argument) ;
void tache2(void const * argument) ;
void tache3(void const * argument) ;

osThreadDef (tache1, osPriorityNormal,1,0) ;
osThreadDef (tache2, osPriorityNormal,1,0) ;
osThreadDef (tache3, osPriorityBelowNormal,1,0) ;

char tab[2]={0xA5,0x20};






int main(void)
{
//char data[100];
int dataAngle1, dataAngle1Inv;
double dataAngle1Final;
int dataAngle2;
int dataDist1;
int dataDist2;
	
//char LCD[50],AG[50],DS[50];
 		char data[20];
	char LCD[2],LCD2[25];
	char i,a;
	int distance,angle;

	
	Init_UART();
	
//	osKernelInitialize() ;
	
	Initialise_GPIO();
	
  Lidar_PWM();
	
	GLCD_Initialize();
	
	GLCD_ClearScreen();
	
	GLCD_SetFont(&GLCD_Font_16x24);
	
	
//	ID_Tache1 = osThreadCreate(osThread(tache1), NULL) ;
//	ID_Tache2 = osThreadCreate(osThread(tache2), NULL) ;
//	ID_Tache3 = osThreadCreate(osThread(tache3), NULL) ;

	while(Driver_USART1.GetStatus().tx_busy == 1);
	Driver_USART1.Send((const void*)tab,2);
	
//	osKernelStart() ;
//	osDelay(osWaitForever) ;
	

	while(1)
  {
		Driver_USART1.Receive(data,10);
		while(Driver_USART1.GetRxCount() < 1);
		for (i=0;i<10;i++)
		{
			if (( (data[i]&0x3E) ==0x3E) && ( (data[i+1]&0x01) == 0x01))
			{
				sprintf(LCD2,"data non traitee : ");
		GLCD_DrawString(0,0,LCD2);		
		sprintf(LCD,"%0x",data[i]);
		GLCD_DrawString(0,30,LCD);	
		sprintf(LCD,"%0x",data[i+1]);
		GLCD_DrawString(40,30,LCD);	
		sprintf(LCD,"%0x",data[i+2]);
		GLCD_DrawString(80,30,LCD);	
		sprintf(LCD,"%0x",data[i+3]);
		GLCD_DrawString(120,30,LCD);	
		sprintf(LCD,"%0x",data[i+4]);
		GLCD_DrawString(160,30,LCD);	
				
		sprintf(LCD2,"data traitee : ");
		GLCD_DrawString(0,60,LCD2);
//traitement
				data[i+1]=(data[i+1]-1)>1;
				angle = (data[i+1] + (data[i+2]<<7)) /64;
				distance = (data[i+3] + (data[i+4]<<8)) /4;
				
				sprintf(LCD2,"ang: %d ",angle);
				GLCD_DrawString(0,90,LCD2);
				sprintf(LCD2,"dis: %d mm",distance);
				GLCD_DrawString(0,120,LCD2);
				
				
				delaitps(1000);
				}
			}
			
		
		
	 
	}
	
return 0;
}



extern void tache1(void const * argument) 
{
	char data[20];
	char LCD[2],LCD2[25];
	char i,a;
	int distance,angle;
	while(1)
  {
		Driver_USART1.Receive(data,10);
		while(Driver_USART1.GetRxCount() < 1);
		for (i=0;i<10;i++)
		{
			if (( (data[i]&0x3E) ==0x3E) && ( (data[i+1]&0x01) == 0x01))
			{
				sprintf(LCD2,"data non traitee : ");
		GLCD_DrawString(0,0,LCD2);		
		sprintf(LCD,"%0x",data[i]);
		GLCD_DrawString(0,30,LCD);	
		sprintf(LCD,"%0x",data[i+1]);
		GLCD_DrawString(40,30,LCD);	
		sprintf(LCD,"%0x",data[i+2]);
		GLCD_DrawString(80,30,LCD);	
		sprintf(LCD,"%0x",data[i+3]);
		GLCD_DrawString(120,30,LCD);	
		sprintf(LCD,"%0x",data[i+4]);
		GLCD_DrawString(160,30,LCD);	
				
		sprintf(LCD2,"data traitee : ");
		GLCD_DrawString(0,60,LCD2);
//traitement
				data[i+1]=data[i+1]-1;
				angle = (data[i+1] + (data[i+2]<<7)) /64;
				distance = (data[i+3] + (data[i+4]<<8)) /4;
				
				sprintf(LCD2,"ang: %d deg",angle);
				GLCD_DrawString(0,90,LCD2);
				sprintf(LCD2,"dis: %d mm",distance);
				GLCD_DrawString(0,120,LCD2);
				}
			}
			
		
		
	 
	}
	osDelay(10000);
}

extern void tache2(void const * argument) 
{

	while(1)
  {
		
	
	}
	osDelay(100); 
}

extern void tache3(void const * argument) 
{
	while(1)
  {
		
		
	osDelay(1); 
	}
	
}


void Lidar_PWM() //P2.5   60%     25 kHz/*
{

	LPC_SC->PCONP |= (1<<6);
	LPC_PINCON->PINSEL4 |= (1<<10);      //PWM2.5
	LPC_PWM1->CTCR = 0;									 //timer
	LPC_PWM1->PR = 0;										 //prescaler=0
	LPC_PWM1->MR0 = 999;								 //25 Khz
	LPC_PWM1->MCR |= (1<<1);						 //RAZ compteur = MR0
	LPC_PWM1->PCR |= (1<<14);						 //start PWM
	LPC_PWM1->MR6 = 599;								 //PWM 60%
	LPC_PWM1->TCR = 1;									 //Start Timer
	
}

void Init_UART()
{

	Driver_USART1.Initialize(NULL);
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	Driver_USART1.Control( ARM_USART_MODE_ASYNCHRONOUS |
												 ARM_USART_DATA_BITS_8 |
	ARM_USART_STOP_BITS_1 |
												 ARM_USART_PARITY_NONE |
												 ARM_USART_FLOW_CONTROL_NONE, 115200);
	Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART1.Control(ARM_USART_CONTROL_RX,1);
}

void delaitps(int tps){ //1428 = 1s
	int i, y,iy;
	y = tps * 20000;
	
	for(i=0; i<y ;i++);	
}



/*


		sprintf(LCD,"A1: %0x",data[1]);
		GLCD_DrawString(0,30,LCD);	
		sprintf(LCD,"A2: %0x",data[2]);
		GLCD_DrawString(0,60,LCD);	
		sprintf(LCD,"D1: %0x",data[3]);
		GLCD_DrawString(0,90,LCD);	
		sprintf(LCD,"D2: %0x",data[4]);
		GLCD_DrawString(0,120,LCD);	
		
		for(i=0;i<5;i++){
			if (i==1){				dataAngle1 = data[i];}
			if (i==2){				dataAngle2 = data[i];}
			if (i==3){				dataDist1 = data[i];}
			if (i==4){				dataDist2 = data[i];}
		}

		sprintf(LCD,"%0x",dataAngle1);
		GLCD_DrawString(150,30,LCD);	
		sprintf(LCD,"%0x",dataAngle2);
		GLCD_DrawString(150,60,LCD);	
		sprintf(LCD,"%0x",dataDist1);
		GLCD_DrawString(150,90,LCD);	
		sprintf(LCD,"%0x",dataDist2);
		GLCD_DrawString(150,120,LCD);		


*/
