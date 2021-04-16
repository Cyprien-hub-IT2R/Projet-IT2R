/* -------------------------------------------
Ex Interface CAN 
// Reception regime (0x0f6) et vitesse (0x128)
// puis affichage sur l'interface LCD
// XM, 03/02/2021
------------------------------------------- */
// A ajouter dans windowDLG
/*

char ultr=0, phar=0, gps=0, esse=0;
#include "stdio.h"

char text_ultrason[20];
char text_phares[20];
char text_gps[20];

#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif

case WM_USER: // mise a jour si informations arrivee au niveau thread UART
			
		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);		// texte reg
		TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
		sprintf(text_ultrason,"Ultrason = 0x%X",ultr);
		TEXT_SetText(hItem, text_ultrason);
	
		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);		// texte reg
		TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
		sprintf(text_phares,"Phares = 0x%X",phar);
		TEXT_SetText(hItem, text_phares);
	
		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);		// texte reg
		TEXT_SetFont(hItem, GUI_FONT_20B_ASCII);
		sprintf(text_gps,"GPS = 0x%X",gps);
		TEXT_SetText(hItem, text_gps);
		break;
		
*/

#include "stm32f7xx_hal.h"
#include "stm32746g_discovery_sdram.h"
#include "RTE_Components.h"
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "Board_Touch.h"                // ::Board Support:Touchscreen
#include "GUI.h"
#include "stdio.h"
#include "DIALOG.h"
#include "Board_LED.h"                  // ::Board Support:LED
#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN


#define  CAN_BITRATE_NOMINAL    125000  // Nominal bitrate (125 kbit/s)
uint32_t                        rx_obj_idx;
uint32_t                        tx_obj_idx;

// A mettre en global pour partage au thread UART
WM_HWIN hDlg;

extern ARM_DRIVER_CAN Driver_CAN1;
//extern char reg, vit;	// réception regime moteur  et vitesse pour affichage
extern char ultr, phar, gps, esse;

ARM_CAN_MSG_INFO   rx_msg_info;
uint8_t data_buf[8];

#ifdef RTE_CMSIS_RTOS_RTX
extern uint32_t os_time;

uint32_t HAL_GetTick(void) { 
  return os_time; 
}
#endif


/*********************************************************************
* *
Externals
* **********************************************************************
*/
WM_HWIN CreateWindow(void);


/*----------------------------------------------------------------------------
 *      GUIThread: GUI Thread for Single-Task Execution Model
 *---------------------------------------------------------------------------*/
 
void GUIThread (void const *argument);              // thread function
osThreadId tid_GUIThread;                           // thread id
osThreadDef (GUIThread, osPriorityIdle, 1, 2048);   // thread object

osThreadId id_CANthreadR;
osThreadId id_CANthreadT;

void CANthreadT (void const *argument);                             // thread function Transmit
void CANthreadR (void const *argument);

osThreadDef(CANthreadR,osPriorityNormal, 1,0);
osThreadDef(CANthreadT,osPriorityNormal, 1,0);                   // thread object

// CAN1 callback
void myCAN1_callback(uint32_t obj_idx, uint32_t event)
{
	
    switch (event)
    {
    case ARM_CAN_EVENT_RECEIVE:
        /*  Message was received successfully by the obj_idx object. */
				Driver_CAN1.MessageRead(0, &rx_msg_info, data_buf, 8);	// 8 data max
				osSignalSet(id_CANthreadR, 0x01);
				break;
		case ARM_CAN_EVENT_SEND_COMPLETE:
        /* 	Message was sent successfully by the obj_idx object.  */
        osSignalSet(id_CANthreadT, 0x02);
        break;
    }
}

/**
  * System Clock Configuration
  *   System Clock source            = PLL (HSE)
  *   SYSCLK(Hz)                     = 216000000
  *   HCLK(Hz)                       = 216000000
  *   AHB Prescaler                  = 1
  *   APB1 Prescaler                 = 4
  *   APB2 Prescaler                 = 2
  *   HSE Frequency(Hz)              = 25000000
  *   PLL_M                          = 25
  *   PLL_N                          = 432
  *   PLL_P                          = 2
  *   PLL_Q                          = 9
  *   VDD(V)                         = 3.3
  *   Main regulator output voltage  = Scale1 mode
  *   Flash Latency(WS)              = 7
  */
static void SystemClock_Config (void) {
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;  
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Activate the OverDrive to reach the 216 MHz Frequency */
  HAL_PWREx_EnableOverDrive();
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
}

/**
  * Configure the MPU attributes
  */
static void MPU_Config (void) {
  MPU_Region_InitTypeDef MPU_InitStruct;
  
  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes for SDRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0xC0200000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_2MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * CPU L1-Cache enable
  */
static void CPU_CACHE_Enable (void) {

  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

void GUIThread (void const *argument) {


	// Call creation function for the dialog
	hDlg = CreateWindow();
	
	  /* Add GUI setup code here */

  while (1) {
    
    /* All GUI related activities might only be called from here */
		// Les images à afficher doivent être placées dans la boucle infinie
	
		// mises à jour affichage
		//GUI_TOUCH_Exec();
		//GUI_Exec();
		GUI_Delay(10);
		//GUI_X_ExecIdle();             /* Nothing left to do for the moment ... Idle processing */
  }
}

void CAN_SignalUnitEvent (uint32_t event) {}

//------------------------------------------------------------------------------
//  CAN Interface Initialization
//------------------------------------------------------------------------------

void InitCan1 (void) {
  
	Driver_CAN1.Initialize(CAN_SignalUnitEvent,myCAN1_callback);
	Driver_CAN1.PowerControl(ARM_POWER_FULL);
	
	Driver_CAN1.SetMode(ARM_CAN_MODE_INITIALIZATION);
	Driver_CAN1.SetBitrate( ARM_CAN_BITRATE_NOMINAL,
													125000,
													ARM_CAN_BIT_PROP_SEG(5U)   |         // Set propagation segment to 5 time quanta
                          ARM_CAN_BIT_PHASE_SEG1(1U) |         // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
                          ARM_CAN_BIT_PHASE_SEG2(1U) |         // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
                          ARM_CAN_BIT_SJW(1U));                // Resynchronization jump width is same as phase segment 2
	
  // Mettre ici les filtres ID de réception sur objet 0
	Driver_CAN1.ObjectSetFilter(0,ARM_CAN_FILTER_ID_EXACT_ADD, ARM_CAN_STANDARD_ID(0x020),0);
	Driver_CAN1.ObjectSetFilter(0,ARM_CAN_FILTER_ID_EXACT_ADD, ARM_CAN_STANDARD_ID(0x030),0);
	Driver_CAN1.ObjectSetFilter(0,ARM_CAN_FILTER_ID_EXACT_ADD, ARM_CAN_STANDARD_ID(0x040),0);
	Driver_CAN1.ObjectSetFilter(0,ARM_CAN_FILTER_ID_EXACT_ADD, ARM_CAN_STANDARD_ID(0x050),0);
	
	Driver_CAN1.ObjectConfigure(0,ARM_CAN_OBJ_RX);				// Objet 0 du CAN1 pour réception
	Driver_CAN1.ObjectConfigure(2,ARM_CAN_OBJ_TX);				// Objet 2 du CAN1 pour émission
	
	Driver_CAN1.SetMode(ARM_CAN_MODE_NORMAL);					// fin init
}


// tache envoi (inactive)
void CANthreadT(void const *argument)
{
	//ARM_CAN_MSG_INFO                tx_msg_info;
	//int i;	
	
	while (1) {
		
		// A compléter si besoin envoi
		osDelay(osWaitForever);
	}		
}

// tache reception
void CANthreadR(void const *argument)
{
	char data_reception;
	int identifiant=0; 
	
		while(1)
	{	
		// Reception trames CAN 	
		osSignalWait(0x01, osWaitForever);		// sommeil en attente réception CAN
		LED_On(0);
		
		identifiant = rx_msg_info.id;	// recup id
		data_reception = data_buf [0] ;			// 1ère donnée de la trame récupérée
		
		// Allumage/Extinction LED
		switch (identifiant)
			{
			case 0x020 :	ultr=data_reception;
										WM_SendMessageNoPara(hDlg, WM_USER);  // pour maj affichage
										break;
											
			case 0x030 :	phar=data_reception;
										WM_SendMessageNoPara(hDlg, WM_USER);  // pour maj affichage
										break;
				
			case 0x040 :	gps=data_reception;
										WM_SendMessageNoPara(hDlg, WM_USER);  // pour maj affichage
										break;
			
			case 0x050 :	esse=data_reception;
										WM_SendMessageNoPara(hDlg, WM_USER);  // pour maj affichage
										break;	
				
			default : 		ultr = 0;
										phar = 0;
										gps = 0;
										esse = 0;
										WM_SendMessageNoPara(hDlg, WM_USER);  // pour maj affichage
										break;	
			}
	}
}

/*********************************************************************
*
*       Main
*/
int main (void) {
	
  MPU_Config ();
	CPU_CACHE_Enable();                       /* Enable the CPU Cache           */
  
  osKernelInitialize ();                    // initialize CMSIS-RTOS

  HAL_Init();                               /* Initialize the HAL Library     */
  BSP_SDRAM_Init();                         /* Initialize BSP SDRAM           */
  SystemClock_Config();                     /* Configure the System Clock     */

  GUI_Init();
  
	Touch_Initialize();
  
  InitCan1();
	
  // initialize peripherals here
	LED_Initialize();		// pour modifier etat LED
	
	
	id_CANthreadT = osThreadCreate (osThread(CANthreadT), NULL);
	id_CANthreadR = osThreadCreate (osThread(CANthreadR), NULL);

  // create 'thread' functions that start executing,
  tid_GUIThread = osThreadCreate (osThread(GUIThread), NULL);
  if (!tid_GUIThread) return(-1);
	
  osKernelStart ();                         // start thread execution 
	
  osDelay(osWaitForever);
}

/*************************** End of file ****************************/

