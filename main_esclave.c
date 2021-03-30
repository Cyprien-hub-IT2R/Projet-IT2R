/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions


/*
 * main: initialize and start the system
 */
/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN
#include "stm32f4xx.h"                  // Device header
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX

osThreadId id_CANthreadR;
osThreadId id_CANthreadT;

extern   ARM_DRIVER_CAN         Driver_CAN1;
extern   ARM_DRIVER_CAN         Driver_CAN2;

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

// CAN2 utilisé pour émission
void myCAN2_callback(uint32_t obj_idx, uint32_t event)
{
    switch (event)
    {
    case ARM_CAN_EVENT_SEND_COMPLETE:
        /* 	Message was sent successfully by the obj_idx object.  */
        osSignalSet(id_CANthreadT, 0x01);
        break;
    }
}

// CAN1 utilisé pour réception
void InitCan1 (void) {
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
	
	Driver_CAN1.SetMode(ARM_CAN_MODE_NORMAL);					// fin init
}

// CAN2 utilisé pour émission
void InitCan2 (void) {
	Driver_CAN2.Initialize(NULL,myCAN2_callback);
	Driver_CAN2.PowerControl(ARM_POWER_FULL);
	
	Driver_CAN2.SetMode(ARM_CAN_MODE_INITIALIZATION);
	Driver_CAN2.SetBitrate( ARM_CAN_BITRATE_NOMINAL,
													9600,
													ARM_CAN_BIT_PROP_SEG(5U)   |         // Set propagation segment to 5 time quanta
                          ARM_CAN_BIT_PHASE_SEG1(1U) |         // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
                          ARM_CAN_BIT_PHASE_SEG2(1U) |         // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
                          ARM_CAN_BIT_SJW(1U));                // Resynchronization jump width is same as phase segment 2
                          
	// Mettre ici les filtres ID de réception sur objet 0
	//....................................................
		
	Driver_CAN2.ObjectConfigure(1,ARM_CAN_OBJ_TX);				// Objet 1 du CAN2 pour émission
	
	Driver_CAN2.SetMode(ARM_CAN_MODE_NORMAL);					// fin init
}
void CANthreadT(void const *argument)
{
	while (1) {
		uint8_t data_buf[8];
		char identifiant,retour;
		ARM_CAN_MSG_INFO tx_msg_info;

		tx_msg_info.id = ARM_CAN_STANDARD_ID (0x002);
		tx_msg_info.rtr = 0; // 0 = trame DATA
		identifiant = tx_msg_info.id; // (int)
		data_buf [0] = 0x55; // data à envoyer à placer dans un tableau de char
		retour = data_buf [0] ; // 1ère donnée de la trame récupérée (char)
		Driver_CAN2.MessageSend(1, &tx_msg_info, data_buf, 1); // 1 data à envoyer	

		osSignalWait(0x01, osWaitForever);		// sommeil en attente fin emission
		osDelay(100);
	}		
}


// tache reception
void CANthreadR(void const *argument)
{
	ARM_CAN_MSG_INFO   rx_msg_info;
	uint8_t data_buf[8];
	char texte[10];
	
	while(1)
	{		
		osSignalWait(0x01, osWaitForever);		// sommeil en attente réception
		
		// Code pour reception trame + affichage Id et Data sur LCD
		//.............

	}
}


osThreadDef(CANthreadR,osPriorityNormal, 1,0);
osThreadDef(CANthreadT,osPriorityNormal, 1,0);
/*
 * main: initialize and start the system
 */
int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS
	
	// Initialisation des 2 périphériques CAN
	InitCan1();
	InitCan2();

  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);
	id_CANthreadR = osThreadCreate (osThread(CANthreadR), NULL);
	id_CANthreadT = osThreadCreate (osThread(CANthreadT), NULL);

  // initialize peripherals here

  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);

  osKernelStart ();                         // start thread execution 
	osDelay(osWaitForever);
}
