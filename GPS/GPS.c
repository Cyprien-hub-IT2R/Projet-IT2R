#include "Driver_USART.h" 
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "stdio.h"
#include "stdlib.h"





extern ARM_DRIVER_USART Driver_USART2;
extern ARM_DRIVER_USART Driver_USART1;
extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;
int32_t GLCD_DrawVLine (uint32_t x, uint32_t y, uint32_t length);
int32_t GLCD_DrawRectangle (uint32_t x, uint32_t y, uint32_t width, uint32_t height);

int main(void)
{	
	char chainerecu[51],chaineenvoi[10],chaineenvoi2[11], clat[10],clat2[10],clon[11],clon2[10],cvit[5],cvit2[10];
	unsigned int latitude1,latitude2,longitude1,longitude2,vitesse1,vitesse2;
	char i;
	int latint=0, longint=0;
	float longitude,vitesse,latitude;
	GLCD_Initialize();
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

	while(1)
	{
		GLCD_DrawRectangle (18,240-140, 59, 83);
		
		
		Driver_USART2.Receive(chainerecu,1);
	  while( Driver_USART2.GetRxCount()<1);
		if(chainerecu[0]=='$')
			{                   // detection $
//				Driver_USART1.Send(chainerecu,1);
//				while(Driver_USART1.GetStatus().tx_busy==1);
				Driver_USART2.Receive(chainerecu,50);
				while( Driver_USART2.GetRxCount()<1);

				if(chainerecu[4]=='C')
					{ 							  //detection GPRMC
					//sprintf (chainerecu,"GPRMC,030742.00,A,4847.22403,N,0219.70012,E,0.356");
					for (i=0;i<=51;i++)
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
					while(Driver_USART1.GetStatus().tx_busy==1);
					Driver_USART1.Send(chainerecu,50);
		
					//sscanf (chainerecu,"%*s %*s %*s %s %*s %s %*s %s",clat,clon,cvit);

					sscanf (chainerecu,"%*s %*s %*s %*s %*s %i %*s %*s %i %*s %*s %*s",&latint,&longint);
							
					latint=latint/100;
					longint=longint/100;
					latint=(latint-203)*2.6;
					longint=(longint-628)*1.83;
					GLCD_DrawRectangle (longint, 240-latint, 5, 5);    // x, y
					latitude=atof(clat);
					sprintf (chaineenvoi,"\r\nlatitude=%f",latitude);
							
					while(Driver_USART1.GetStatus().tx_busy==1);
					Driver_USART1.Send(clat,10);
						
					longitude=atof(clon);
					sprintf (chaineenvoi,"\r\nlongitude=%f",longitude);
					while(Driver_USART1.GetStatus().tx_busy==1);
					Driver_USART1.Send(chaineenvoi,23);
					
					vitesse=atof(cvit);
					sprintf (chaineenvoi,"\r\nVitesse=%f\r\n",vitesse);
					while(Driver_USART1.GetStatus().tx_busy==1);
					Driver_USART1.Send(chaineenvoi,20);


	}
  }
	}
	
	return 0;
}

