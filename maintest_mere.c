#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions

#include "LPC17xx.h"                    // Device header
#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN
#include "stdio.h"
#include "cmsis_os.h"
#include "GPIO.h"
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD

osThreadId id_CANthreadR;

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

extern   ARM_DRIVER_CAN         Driver_CAN1;
extern   ARM_DRIVER_CAN         Driver_CAN2;

// CAN1 utilisé pour réception
void myCAN1_callback(uint32_t obj_idx, uint32_t event)
{
    switch (event)
    {
    case ARM_CAN_EVENT_RECEIVE:
        /*  Message was received successfully by the obj_idx object. */
       osSignalSet(id_CANthreadR, 0x01);
        break;
    }
}

// CAN1 utilisé pour réception
void InitCan1 (void) 
	{
	Driver_CAN1.Initialize(NULL,myCAN1_callback);
	Driver_CAN1.PowerControl(ARM_POWER_FULL);
	
	Driver_CAN1.SetMode(ARM_CAN_MODE_INITIALIZATION);
	Driver_CAN1.SetBitrate( ARM_CAN_BITRATE_NOMINAL,
													125000,
													ARM_CAN_BIT_PROP_SEG(5U)   |         // Set propagation segment to 5 time quanta
                          ARM_CAN_BIT_PHASE_SEG1(1U) |         // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
                          ARM_CAN_BIT_PHASE_SEG2(1U) |         // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
                          ARM_CAN_BIT_SJW(1U));                // Resynchronization jump width is same as phase segment 2
                          
	// Mettre ici les filtres ID de réception sur objet 0
	//....................................................
		
	Driver_CAN1.ObjectConfigure(0,ARM_CAN_OBJ_RX);				// Objet 0 du CAN1 pour réception
	
	Driver_CAN1.ObjectSetFilter(0, ARM_CAN_FILTER_ID_EXACT_ADD,ARM_CAN_STANDARD_ID(0x001),0) ; // Objet 0 : réception Status capteur Ultrason
	Driver_CAN1.ObjectSetFilter(0, ARM_CAN_FILTER_ID_EXACT_ADD,ARM_CAN_STANDARD_ID(0x002),0) ; // Objet 0 : réception capteur luminosité + Leds
	Driver_CAN1.ObjectSetFilter(0, ARM_CAN_FILTER_ID_EXACT_ADD,ARM_CAN_STANDARD_ID(0x004),0) ; // Objet 0 : réception GPS
	
	Driver_CAN1.SetMode(ARM_CAN_MODE_NORMAL);					// fin init
}



// tache reception
void CANthreadR(void const *argument)
{
	ARM_CAN_MSG_INFO  rx_msg_info;
	uint8_t data_buf[8];
	char taille,ID;
	char chaine[20];
	char chaine1[20];
	
	while(1)
	{	
		osSignalWait(0x01, osWaitForever);		// sommeil en attente réception
	
		Driver_CAN1.MessageRead(0, &rx_msg_info, data_buf, 8); // 8 data max
		ID = rx_msg_info.id; // (int)
		taille = rx_msg_info.dlc; // nb data (char)
		
		sprintf(chaine,"ID recu : %d",ID);
		GLCD_DrawString(1,1,(unsigned char*)chaine);
		
		sprintf(chaine1,"DATA recu : 0x%X",data_buf[0]);
		GLCD_DrawString(1,30,(unsigned char*)chaine1);
	}
}

osThreadDef(CANthreadR,osPriorityNormal, 1,0);

int main (void) {
	
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);	// initialize CMSIS-RTOS
	
	// Initialisation des 2 périphériques CAN
	InitCan1();
	osKernelInitialize ();
	
	id_CANthreadR = osThreadCreate (osThread(CANthreadR), NULL);

  osKernelStart ();                         // start thread execution 
	osDelay(osWaitForever);
}
