#include "LPC17xx.h"                    // Device header
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include <stdio.h>
#include <stdlib.h>

#define Li_START_SCAN =0x20;			//Pour la fonction change_mod
#define Li_STOP =0x25;
#define Li_RESET =0x40;


// Traduire les trames du LIDAR
// puis faire une interruption lorsque la distance est a moins de 10cm dun obstacle


extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;
extern ARM_DRIVER_USART Driver_USART1;

void Lidar_PWM(void);
void Init_UART(void);
void change_mod(char type);
unsigned reverse(unsigned int v);

char tab[2]={0xA5,0x20};

int main(void)
{
char data[16];
int dataAngle1, dataAngle1Inv;
double dataAngle1Final;
double dataAngle2;
double dataDist1;
double dataDist2;
	
char LCD[50],AG[50],DS[50];
	int i,a=1,angle;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
	Init_UART();
  Lidar_PWM();
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
	while(Driver_USART1.GetStatus().tx_busy == 1);
	Driver_USART1.Send((const void*)tab,2);

		
	
	while(1)
	{

		
Driver_USART1.Receive(data,5);
		while(Driver_USART1.GetRxCount() < 1);
		
		
		
	//	sprintf(LCD,"data : %s",data);	// du blabla
	//	GLCD_DrawString(0,0,LCD);	// colonne puis ligne en pixel
		
		
		sprintf(LCD,"Qualite : %0x",data[0]);
		GLCD_DrawString(0,0,LCD);	
		sprintf(LCD,"Angle1 : %0x",data[1]);
		GLCD_DrawString(0,30,LCD);	
		sprintf(LCD,"Angle2 : %0x",data[2]);
		GLCD_DrawString(0,60,LCD);	
		sprintf(LCD,"Dist1 : %0x",data[3]);
		GLCD_DrawString(0,90,LCD);	
		sprintf(LCD,"Dist2 : %0x",data[4]);
		GLCD_DrawString(0,120,LCD);	
		
		for(i=0;i<5;i++){
			if (i==0){				dataAngle1 = data[i] ;}
			if (i==1){				dataAngle2 = data[i];}
			if (i==2){				dataAngle1 = data[i];}
			if (i==3){				dataDist1 = data[i];}
			if (i==4){				dataAngle2 = data[i];}
		}
		
		sprintf(LCD,"Angle : %d",dataAngle1);
		GLCD_DrawString(0,160,LCD);	
		
		dataAngle1Inv = reverse(dataAngle1);
		
		sprintf(LCD,"Angle : %d",dataAngle1Inv);
		GLCD_DrawString(0,190,LCD);	
		
		//dataAngle1Final = (dataAngle1Inv-128)/64 ;

		
	}
	
return 0;
}

unsigned reverse(unsigned int valeur) {
    unsigned int result = valeur;                
    int taille = sizeof(valeur) * 8 - 1;   // 8 taille bit
     
    for (valeur >>= 1; valeur; valeur >>= 1) {  
        result <<= 1;
        result |= valeur & 1;
        taille--;
    }
    result <<= taille;
     
    return(taille);
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

void change_mod(char type)                                                           
{
switch(type)
{
	case 0x20 : //	Start/Scan
	{
	tab[1] = 0x20;
	}
		
	case 0x25 : //	Stop
	{
	tab[1] = 0x25;
	}
	
case 0x40 : //	Reset
	{
	tab[1] = 0x40;
	}


}
}





