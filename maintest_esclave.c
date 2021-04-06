#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions

#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN
#include "stdio.h"
#include "cmsis_os.h"

// STMf34 : 0 pour reception, 2 pour envoie
osThreadId id_CANthreadT;


extern   ARM_DRIVER_CAN         Driver_CAN2;

// CAN2 utilisé pour émission
void myCAN2_callback(uint32_t obj_idx, uint32_t event)
{
    switch (event)
    {
    case ARM_CAN_EVENT_SEND_COMPLETE:
        /* 	Message was sent successfully by the obj_idx object.  */
        osSignalSet(id_CANthreadT, 0x02);
        break;
    }
}

// CAN2 utilisé pour émission
void InitCan2 (void) 
	{
	Driver_CAN2.Initialize(NULL,myCAN2_callback);
	Driver_CAN2.PowerControl(ARM_POWER_FULL);
	
	Driver_CAN2.SetMode(ARM_CAN_MODE_INITIALIZATION);
	Driver_CAN2.SetBitrate( ARM_CAN_BITRATE_NOMINAL,
													125000,
													ARM_CAN_BIT_PROP_SEG(5U)   |         // Set propagation segment to 5 time quanta
                          ARM_CAN_BIT_PHASE_SEG1(1U) |         // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
                          ARM_CAN_BIT_PHASE_SEG2(1U) |         // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
                          ARM_CAN_BIT_SJW(1U));                // Resynchronization jump width is same as phase segment 2
                          
	// Mettre ici les filtres ID de réception sur objet 0
	//....................................................
		
	Driver_CAN2.ObjectConfigure(2,ARM_CAN_OBJ_TX);				// Objet 1 du CAN2 pour émission
	
	Driver_CAN2.SetMode(ARM_CAN_MODE_NORMAL);					// fin init
}


// tache envoi toutes les secondes
void CANthreadT(void const *argument)
{
	uint8_t data_buf[8];
	ARM_CAN_MSG_INFO tx_msg_info;
	

	while (1) {
		
		tx_msg_info.id = ARM_CAN_STANDARD_ID (0x002);
		tx_msg_info.rtr = 0; // 0 = trame DATA
		data_buf [0] = 0x22; // data à envoyer à placer dans un tableau de char
		Driver_CAN2.MessageSend(2, &tx_msg_info, data_buf, 1); // 1 data à envoyer	
		
		osSignalWait(0x02, osWaitForever);		// sommeil en attente fin emission
		
		tx_msg_info.id = ARM_CAN_STANDARD_ID (0x003);
		tx_msg_info.rtr = 0; // 0 = trame DATA
		data_buf [0] = 0x33; // data à envoyer à placer dans un tableau de char
		Driver_CAN2.MessageSend(2, &tx_msg_info, data_buf, 1); // 1 data à envoyer	
		
		osSignalWait(0x02, osWaitForever);		// sommeil en attente fin emission
		
		tx_msg_info.id = ARM_CAN_STANDARD_ID (0x004);
		tx_msg_info.rtr = 0; // 0 = trame DATA
		data_buf [0] = 0x44; // data à envoyer à placer dans un tableau de char
		Driver_CAN2.MessageSend(2, &tx_msg_info, data_buf, 1); // 1 data à envoyer	
		
		osSignalWait(0x02, osWaitForever);		// sommeil en attente fin emission
		osDelay(50);
	}		
}

osThreadDef(CANthreadT,osPriorityNormal, 1,0);

int main (void) {
	
	InitCan2();
	osKernelInitialize ();
	
	id_CANthreadT = osThreadCreate (osThread(CANthreadT), NULL);

  osKernelStart ();                         // start thread execution 
	osDelay(osWaitForever);
}
