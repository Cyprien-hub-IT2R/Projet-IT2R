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
osThreadId id_CANthreadT;
osThreadId id_ultrason;
osThreadId id_phares;
osThreadId id_gps;

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

typedef struct{
		char  data_ultrason;
		char  data_phares;
		char  data_gps;
	}MaStruct;

extern   ARM_DRIVER_CAN         Driver_CAN1;
extern   ARM_DRIVER_CAN         Driver_CAN2;

osMutexId ID_mut_GLCD; // Mutex pour accès LCD
osMutexDef (mut_GLCD);
	
osMailQId ID_BAL_ultrason ;
osMailQDef (NOM_BAL_ultrason, 20,MaStruct) ;
	
osMailQId ID_BAL_phares ;
osMailQDef (NOM_BAL_phares, 20,MaStruct) ;

osMailQId ID_BAL_gps ;
osMailQDef (NOM_BAL_gps, 20,MaStruct) ;	

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
		
	Driver_CAN1.ObjectConfigure(0,ARM_CAN_OBJ_RX);				// Objet 0 du CAN1 pour réception
	
	Driver_CAN1.ObjectSetFilter(0, ARM_CAN_FILTER_ID_EXACT_ADD,ARM_CAN_STANDARD_ID(0x002),0) ; // Objet 0 : réception et ID=0x002 ultra son
	Driver_CAN1.ObjectSetFilter(0, ARM_CAN_FILTER_ID_EXACT_ADD,ARM_CAN_STANDARD_ID(0x003),0) ; // Objet 0 : réception et ID=0x003 phares
	Driver_CAN1.ObjectSetFilter(0, ARM_CAN_FILTER_ID_EXACT_ADD,ARM_CAN_STANDARD_ID(0x004),0) ; // Objet 0 : réception et ID=0x004 gps

	Driver_CAN1.SetMode(ARM_CAN_MODE_NORMAL);					// fin init
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
		
	Driver_CAN2.ObjectConfigure(1,ARM_CAN_OBJ_TX);				// Objet 1 du CAN2 pour émission
	
	Driver_CAN2.SetMode(ARM_CAN_MODE_NORMAL);					// fin init
}


// Thread envoi
void CANthreadT(void const *argument)
{
	uint8_t data_buf[8];
	ARM_CAN_MSG_INFO tx_msg_info;

	while (1)
	{
		tx_msg_info.id = ARM_CAN_STANDARD_ID (0x020);
		tx_msg_info.rtr = 0; // 0 = trame DATA
		data_buf [0] = 0x20; // data à envoyer à placer dans un tableau de char
		Driver_CAN2.MessageSend(1, &tx_msg_info, data_buf, 1); // 1 data à envoyer	
		
		osSignalWait(0x02, osWaitForever);		// sommeil en attente fin emission
		
		tx_msg_info.id = ARM_CAN_STANDARD_ID (0x030);
		tx_msg_info.rtr = 0; // 0 = trame DATA
		data_buf [0] = 0x30; // data à envoyer à placer dans un tableau de char
		Driver_CAN2.MessageSend(1, &tx_msg_info, data_buf, 1); // 1 data à envoyer	
		
		osSignalWait(0x02, osWaitForever);		// sommeil en attente fin emission
		
		tx_msg_info.id = ARM_CAN_STANDARD_ID (0x040);
		tx_msg_info.rtr = 0; // 0 = trame DATA
		data_buf [0] = 0x40; // data à envoyer à placer dans un tableau de char
		Driver_CAN2.MessageSend(1, &tx_msg_info, data_buf, 1); // 1 data à envoyer	
		
		osSignalWait(0x02, osWaitForever);		// sommeil en attente fin emission
		
		osDelay(50);
	}		
}

// Thread reception
void CANthreadR(void const *argument)
{
	ARM_CAN_MSG_INFO  rx_msg_info;
	uint8_t data_buf[8];
	MaStruct *ptr;
//	char taille,retour;
	short identifiant;
	
	while(1)
	{
		osSignalWait(0x01, osWaitForever);		// sommeil en attente réception
		
		Driver_CAN1.MessageRead(0, &rx_msg_info, data_buf, 32); // 8 data max
		
		identifiant = rx_msg_info.id;
//		taille = rx_msg_info.dlc; // nb data (char)
//		retour = data_buf[0];
		
		switch(identifiant){
	
			case 0x002 :

				//stockage information data capteur ultrason
				ptr = osMailAlloc(ID_BAL_ultrason, osWaitForever);
				ptr -> data_ultrason = data_buf[0]; // valeur à envoyer
				osMailPut(ID_BAL_ultrason, ptr); // reveille tache à effectuer

				break;
			
			case 0x003 :

				// Reception  data status capteur luminosité + LEDS (phares)
				ptr = osMailAlloc(ID_BAL_phares, osWaitForever);
				ptr -> data_phares = data_buf[0]; // valeur à envoyer
				osMailPut(ID_BAL_phares, ptr); // reveille tache à effectuer

				break;
			
			case 0x004:
			
				// Reception data GPS
				ptr = osMailAlloc(ID_BAL_gps, osWaitForever);
				ptr -> data_gps = data_buf[0]; // valeur à envoyer
				osMailPut(ID_BAL_gps, ptr); // reveille tache à effectuer

				break;
		}
	}
}

void ultrason(void const *argument)
{
		char chaine_ultrason[20];
		MaStruct *recep;
		MaStruct valeur_recue;
		osEvent result;
		uint8_t data_buf[8];
		ARM_CAN_MSG_INFO tx_msg_info;

		while(1)
		{
			result = osMailGet(ID_BAL_ultrason, osWaitForever); // attente mail
			recep = result.value.p; // on cible le pointeur...
			valeur_recue = *recep ; // ...et la valeur pointée
			osMailFree(ID_BAL_ultrason, recep); // libération mémoire allouée
			
			sprintf(chaine_ultrason,"ULTR 2 = 0x%X",valeur_recue.data_ultrason);
			osMutexWait(ID_mut_GLCD, osWaitForever);
			GLCD_DrawString(1,1,(unsigned char*)chaine_ultrason);
			osMutexRelease(ID_mut_GLCD); // libération SC	
			
			tx_msg_info.id = ARM_CAN_STANDARD_ID (0x020);
			tx_msg_info.rtr = 0; // 0 = trame DATA
			data_buf [0] = valeur_recue.data_ultrason; // data à envoyer à placer dans un tableau de char
			Driver_CAN2.MessageSend(1, &tx_msg_info, data_buf, 1); // 1 data à envoyer	

			osSignalWait(0x02, osWaitForever);		// sommeil en attente fin emission
			osDelay(50);
		}
}

void phares(void const *argument)
{
	char chaine_phares[20];
	osEvent result;
	MaStruct *recep;
	MaStruct valeur_recue;
	uint8_t data_buf[8];
	ARM_CAN_MSG_INFO tx_msg_info;
	
	while(1)
	{
		result = osMailGet(ID_BAL_phares, osWaitForever); // attente mail
		recep = result.value.p; // on cible le pointeur...
		valeur_recue = *recep ; // ...et la valeur pointée
		osMailFree(ID_BAL_phares, recep); // libération mémoire allouée
		
		sprintf(chaine_phares,"PHAR 3 = 0x%X",valeur_recue.data_phares);
		osMutexWait(ID_mut_GLCD, osWaitForever);
		GLCD_DrawString(1,51,(unsigned char*)chaine_phares);			
		osMutexRelease(ID_mut_GLCD); // libération SC	
		
		tx_msg_info.id = ARM_CAN_STANDARD_ID (0x030);
		tx_msg_info.rtr = 0; // 0 = trame DATA
		data_buf [0] = valeur_recue.data_phares; // data à envoyer à placer dans un tableau de char
		Driver_CAN2.MessageSend(1, &tx_msg_info, data_buf, 1); // 1 data à envoyer	
		
		osSignalWait(0x02, osWaitForever);		// sommeil en attente fin emission
		osDelay(50);
	}		
}

void gps(void const *argument)
{
	char chaine_gps[20];
	osEvent result;
	MaStruct *recep;
	MaStruct valeur_recue;
	uint8_t data_buf[8];
	ARM_CAN_MSG_INFO tx_msg_info;
	
	while(1)
	{	
		result = osMailGet(ID_BAL_gps, osWaitForever); // attente mail
		recep = result.value.p; // on cible le pointeur...
		valeur_recue = *recep ; // ...et la valeur pointée
		osMailFree(ID_BAL_gps, recep); // libération mémoire allouée
		
		sprintf(chaine_gps,"GPS 4 = 0x%X",valeur_recue.data_gps);
		osMutexWait(ID_mut_GLCD, osWaitForever);
		GLCD_DrawString(1,101,(unsigned char*)chaine_gps);
		osMutexRelease(ID_mut_GLCD); // libération SC	
		
		tx_msg_info.id = ARM_CAN_STANDARD_ID (0x040);
		tx_msg_info.rtr = 0; // 0 = trame DATA
		data_buf [0] = valeur_recue.data_gps; // data à envoyer à placer dans un tableau de char
		Driver_CAN2.MessageSend(1, &tx_msg_info, data_buf, 1); // 1 data à envoyer	
		
		osSignalWait(0x02, osWaitForever);		// sommeil en attente fin emission
		osDelay(50);
	}		
}

osThreadDef(CANthreadR,osPriorityNormal, 1,0);
osThreadDef(CANthreadT,osPriorityNormal, 1,0);
osThreadDef(ultrason,osPriorityNormal,1,0);
osThreadDef(phares,osPriorityNormal,1,0);
osThreadDef(gps,osPriorityNormal,1,0);

int main (void)
{
	// Init GLCD
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
	// Initialisation des 2 périphériques CAN
	InitCan1(); // reception
	InitCan2(); // emission
	
	osKernelInitialize ();
	
	ID_mut_GLCD = osMutexCreate(osMutex(mut_GLCD)) ;
	
	ID_BAL_ultrason = osMailCreate(osMailQ(NOM_BAL_ultrason), NULL);
	ID_BAL_phares = osMailCreate(osMailQ(NOM_BAL_phares), NULL);
	ID_BAL_gps = osMailCreate(osMailQ(NOM_BAL_gps), NULL);
	
	id_CANthreadR = osThreadCreate (osThread(CANthreadR), NULL);
//	id_CANthreadT = osThreadCreate (osThread(CANthreadT), NULL);
	id_ultrason = osThreadCreate (osThread(ultrason), NULL);
	id_phares = osThreadCreate (osThread(phares), NULL);
	id_gps = osThreadCreate (osThread(gps), NULL);

  osKernelStart (); // start thread execution 
	osDelay(osWaitForever);
}
