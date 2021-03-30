/*---------------------------------------------------
* CAN 2 uniquement en TX 
* + réception CAN1 
* avec RTOS et utilisation des fonction CB
* pour test sur 1 carte -> relier CAN1 et CAN2
* 2017-04-02 - XM
---------------------------------------------------*/

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
		char  donnee1;
		char  donnee2;
		char  donnee3;
		char  donnee4;
	}MaStruct;

extern   ARM_DRIVER_CAN         Driver_CAN1;
extern   ARM_DRIVER_CAN         Driver_CAN2;

osMailQId ID_BAL_ultrason ;
osMailQDef (NOM_BAL, 50,MaStruct) ;
	
osMailQId ID_BAL_phares ;
osMailQDef (NOM_BAL_phares, 50,MaStruct) ;

osMailQId ID_BAL_gps ;
osMailQDef (NOM_BAL_gps, 50,MaStruct) ;	
	
	

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
	//....................................................
		
	Driver_CAN1.ObjectConfigure(0,ARM_CAN_OBJ_RX);				// Objet 0 du CAN1 pour réception
	
	Driver_CAN1.ObjectSetFilter(0, ARM_CAN_FILTER_ID_EXACT_ADD,ARM_CAN_STANDARD_ID(0x001),0) ; // Objet 0 : réception Status capteur Ultrason
	Driver_CAN1.ObjectSetFilter(0, ARM_CAN_FILTER_ID_EXACT_ADD,ARM_CAN_STANDARD_ID(0x002),0) ; // Objet 0 : réception capteur luminosité + Leds
	Driver_CAN1.ObjectSetFilter(0, ARM_CAN_FILTER_ID_EXACT_ADD,ARM_CAN_STANDARD_ID(0x004),0) ; // Objet 0 : réception GPS
	
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


// tache envoi toutes les secondes
void CANthreadT(void const *argument)
{
	char identifiant,retour;
	char texte1[10];
	char texte2[10];
	uint8_t data_buf[8];
	ARM_CAN_MSG_INFO tx_msg_info;
	

	while (1) {
		
		osSignalWait(0x02, osWaitForever);		// sommeil en attente fin emission
		
		tx_msg_info.id = ARM_CAN_STANDARD_ID (0x001);
		tx_msg_info.rtr = 0; // 0 = trame DATA
		identifiant = tx_msg_info.id; // (int)
		data_buf [0] = 0x55; // data à envoyer à placer dans un tableau de char
		retour = data_buf [0] ; // 1ère donnée de la trame récupérée (char)
		Driver_CAN2.MessageSend(1, &tx_msg_info, data_buf, 1); // 1 data à envoyer	

//		sprintf(texte1,"ID= 0x%X",identifiant);
//		GLCD_DrawString(1,1,(unsigned char*)texte1);
//		sprintf(texte2,"DATA= 0x%X",retour);
//		GLCD_DrawString(1,30,(unsigned char*)texte2);
		
		
		osDelay(50);
	}		
}


// tache reception
void CANthreadR(void const *argument)
{
	ARM_CAN_MSG_INFO  rx_msg_info;
	uint8_t data_buf[8];
	char taille,ID;
	char chaine[20];
	
	while(1)
	{	
		
		MaStruct*ptr;
		
		osSignalWait(0x01, osWaitForever);		// sommeil en attente réception
	
		Driver_CAN1.MessageRead(0, &rx_msg_info, data_buf, 8); // 8 data max
		ID = rx_msg_info.id; // (int)
		taille = rx_msg_info.dlc; // nb data (char)
		
		sprintf(chaine,"ID recu : %d",ID);
		GLCD_DrawString(1,1,(unsigned char*)chaine);
		
//		switch(ID){
//	
//			case 0x002 :
//				
//			//stockage infrmation data capteur ultrason
//			ptr = osMailAlloc(ID_BAL_ultrason, osWaitForever);
//			ptr -> donnee1 = data_buf[0]; // valeur à envoyer
//			osMailPut(ID_BAL_ultrason, ptr);
//			
//			// reveille tache à effectuer
//			
//				
//			break;
//			
//			case 0x003 :
//				
//			// Reception  data status capteur luminosité + LEDS (phares)
//			ptr = osMailAlloc(ID_BAL_phares, osWaitForever);
//			ptr -> donnee2 = data_buf[0]; // valeur à envoyer
//			osMailPut(ID_BAL_phares, ptr);
//			
//			// reveille tache à effectuer
//			
//				
//			break;
//			
//			case 0x004:
//				
//			// Reception data GPS
//			ptr = osMailAlloc(ID_BAL_gps, osWaitForever);
//			ptr -> donnee3 = data_buf[0]; // valeur à envoyer
//			osMailPut(ID_BAL_gps, ptr);
//			
//			// reveille tache à effectuer
//			
//			break;
//		
//		
//	}
	}
}

void ultrason(void const *argument)
{
		osEvent result;
		MaStruct *recep;
		MaStruct valeur_recue;
		char chaine[20];

		while(1)
		{
		result = osMailGet(ID_BAL_ultrason, osWaitForever); // attente mail
		recep = result.value.p; // on cible le pointeur...
		valeur_recue = *recep ; // ...et la valeur pointée
		osMailFree(ID_BAL_ultrason, recep); // libération mémoire allouée
		
		sprintf(chaine,"valeur reçu : %d",valeur_recue.donnee1);
		GLCD_DrawString(1,1,(unsigned char*)chaine);
			
		osDelay(50);
	}		
}

void phares(void const *argument)
{
	osEvent result;
	MaStruct *recep;
	MaStruct valeur_recue;
	
	while(1)
	{
		result = osMailGet(ID_BAL_phares, osWaitForever); // attente mail
		recep = result.value.p; // on cible le pointeur...
		valeur_recue = *recep ; // ...et la valeur pointée
		osMailFree(ID_BAL_phares, recep); // libération mémoire allouée
		
		osDelay(50);
	}		
}

void gps(void const *argument)
{
	osEvent result;
	MaStruct *recep;
	MaStruct valeur_recue;
	
	while(1)
	{
		result = osMailGet(ID_BAL_gps, osWaitForever); // attente mail
		recep = result.value.p; // on cible le pointeur...
		valeur_recue = *recep ; // ...et la valeur pointée
		osMailFree(ID_BAL_gps, recep); // libération mémoire allouée
		
		osDelay(50);
	}		
}

osThreadDef(CANthreadR,osPriorityNormal, 1,0);
osThreadDef(CANthreadT,osPriorityNormal, 1,0);
osThreadDef(ultrason,osPriorityNormal,1,0);
osThreadDef(phares,osPriorityNormal,1,0);
osThreadDef(gps,osPriorityNormal,1,0);

/*
 * main: initialize and start the system
 */
int main (void) {
	
	
  
	
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);	// initialize CMSIS-RTOS
	//GLCD_SetBackgroundColor (0x11);
	
	// Initialisation des 2 périphériques CAN
	InitCan1();
	InitCan2();
	osKernelInitialize ();
	
  // create 'thread' functions that start executing,
 // example: tid_name = osThreadCreate (osThread(name), NULL);
	id_CANthreadR = osThreadCreate (osThread(CANthreadR), NULL);
	id_CANthreadT = osThreadCreate (osThread(CANthreadT), NULL);
//	id_ultrason = osThreadCreate (osThread(ultrason), NULL);
//	id_phares = osThreadCreate (osThread(phares), NULL);
//	id_gps = osThreadCreate (osThread(gps), NULL);

  osKernelStart ();                         // start thread execution 
	osDelay(osWaitForever);
}
