/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "Driver_USART.h" 
#define sonderapage 4
#define sonklaxon1 1
#define sondemarrage 2


extern ARM_DRIVER_USART Driver_USART1;
char num;
void DFplayer (void const *argument);                             // thread function
osThreadId tid_DFplayer;                                          // thread id
osThreadDef (DFplayer, osPriorityNormal, 1, 0);                   // thread object

void DFplayer (void const *argument) {
	short checksum;
	char checksumH,checksumL;
	osEvent audio;
	int titre;
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
  while (1) {
	 audio=osSignalWait(0,osWaitForever);
	 titre=audio.value.signals;
	 if (titre==1){
		 num=2;
	 }
	 if (titre==2){
		 num=3;
	 }
	 if (titre==4){
		 num=1;
	 }
		
		uint8_t tab[10]={0x7E,0xFF,0x06,0x09,0x00,0x00,0x01,0x00,0x00,0xEF};
				
		
	checksum=0-(tab[1]+tab[2]+tab[3]+tab[4]+tab[5]+tab[6]);
	checksumH=(char)((checksum & 0xFF00)>>8);
	checksumL=(char)checksum;
		
	tab[7]=checksumH;
	tab[8]=checksumL;
		
		while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
		Driver_USART1.Send(tab,80);	
		
		tab[3]=0x06;
	 tab[6]=0x15;

		checksum=0-(tab[1]+tab[2]+tab[3]+tab[4]+tab[5]+tab[6]);
	checksumH=(char)((checksum & 0xFF00)>>8);
	checksumL=(char)checksum;
		
	tab[7]=checksumH;
	tab[8]=checksumL;
		
		while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
		Driver_USART1.Send(tab,80);	
		
				tab[3]=0x03;
	  tab[6]=num;

		checksum=0-(tab[1]+tab[2]+tab[3]+tab[4]+tab[5]+tab[6]);
	checksumH=(char)((checksum & 0xFF00)>>8);
	checksumL=(char)checksum;
		
	tab[7]=checksumH;
	tab[8]=checksumL;
		
		while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
		Driver_USART1.Send(tab,80);	
		
		tab[3]=0x0D;
	  tab[6]=0x00;

		checksum=0-(tab[1]+tab[2]+tab[3]+tab[4]+tab[5]+tab[6]);
	checksumH=(char)((checksum & 0xFF00)>>8);
	checksumL=(char)checksum;
		
	tab[7]=checksumH;
	tab[8]=checksumL;
		
		while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
		Driver_USART1.Send(tab,80);

  }
}
/*
 * main: initialize and start the system
 */
int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS
  tid_DFplayer = osThreadCreate (osThread(DFplayer), NULL);
  // initialize peripherals here
  osSignalset(tid_DFplayer,sonklaxon1);
  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);

  osKernelStart ();                         // start thread execution 
	osDelay(osWaitForever);
}
