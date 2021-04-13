
#define osObjectsPublic         
#include "osObjects.h"                 
#include "Driver_USART.h" 
#include "Board_GLCD.h"             
#include "GLCD_Config.h"         
#include "stdio.h"
#include "stdlib.h"

extern ARM_DRIVER_USART Driver_USART2;
extern ARM_DRIVER_USART Driver_USART1;
extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;
int32_t GLCD_DrawVLine (uint32_t x, uint32_t y, uint32_t length);
int32_t GLCD_DrawRectangle (uint32_t x, uint32_t y, uint32_t width, uint32_t height);


void gps (void const *argument);                    
osThreadId tid_gps;                                
osThreadDef (gps, osPriorityNormal, 1, 0);          

void gps (void const *argument) {
	
	char chainerecu[51],i;																	//D�claration des variables
	int latint=0, longint=0;																
	
	GLCD_Initialize();																			//Initialisation des p�riph�riques
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	 
		Driver_USART2.Initialize(NULL);
	Driver_USART2.PowerControl(ARM_POWER_FULL);
	Driver_USART2.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							9600);
	Driver_USART2.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART2.Control(ARM_USART_CONTROL_RX,1);
	
	Driver_USART1.Initialize(NULL);
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	Driver_USART1.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							9600);
	Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART1.Control(ARM_USART_CONTROL_RX,1);
	
  GLCD_DrawRectangle (35,240-148, 27, 99);									//Plan de l'IUT
	GLCD_DrawRectangle (101,240-70, 108, 64);
 
 while (1) {
	 
	Driver_USART2.Receive(chainerecu,1);										//R�ception d'un caract�re
			while( Driver_USART2.GetRxCount()<1);
	 
	 if(chainerecu[0]=='$')																	//Analyse du caract�re re�u: Est-ce un $? Si oui, suite du traitement. Sinon, on recommence.
			{ 
				Driver_USART1.Send(chainerecu,1);									//Affichage du $ sur Hyperterminal (facultatif)
				while(Driver_USART1.GetStatus().tx_busy==1);
				
				Driver_USART2.Receive(chainerecu,50);							//R�ception des 50 caract�res suivants le $
				while( Driver_USART2.GetRxCount()<1);

				if(chainerecu[4]=='C')														//Analyse de la chaine re�u: Est-ce une trame GPRMC? Si oui, suite du traitement. Sinon, Retour � l'analyse du $.
					{ 
					for (i=0;i<=51;i++)															//Suppression des points et virgules pour permettre la r�cup�ration des donn�es cibl�es avec le sscanf
							{
								if(chainerecu[i]==',')
								{
									chainerecu[i]=' ';
								}
								if(chainerecu[i]=='.')
								{
									chainerecu[i]=' ';
								}
							}
							
					while(Driver_USART1.GetStatus().tx_busy==1);			//Affichage de la chaine sur Hyperterminal (facultatif)
					Driver_USART1.Send(chainerecu,50);
							
					GLCD_SetForegroundColor(GLCD_COLOR_WHITE);				//Suppression de l'affichage de la localisation pr�c�dente
					GLCD_DrawRectangle (longint, 240-latint, 5, 5);
							
					sscanf (chainerecu,"%*s %*s %*s %*s %*s %i %*s %*s %i %*s %*s %*s",&latint,&longint);    	//R�cup�ration des donn�es cibl�es: Latitude Longitude
							
					latint=latint/100;																//Conversion des positions GPS en positions LCD
					longint=longint/100;
					latint=(latint-203)*2.6;
					longint=(longint-628)*1.83;
					
					GLCD_SetForegroundColor(GLCD_COLOR_BLACK);										//Affichage de la localisation
					GLCD_DrawRectangle (longint, 240-latint, 5, 5);

					osDelay(100);																			//Passage de la t�che en sommeil pour 100ms											
					}
			}
  }
}

int main (void) {
	
  osKernelInitialize ();  																
	
  tid_gps = osThreadCreate (osThread(gps), NULL);

  osKernelStart ();     
	osDelay(osWaitForever);
	
}
