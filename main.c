/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @brief   STM32F4xx HAL API Template project 
  *
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C
#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
//#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "stdio.h"
#include "main.h"
#include "Board_LED.h"                  // ::Board Support:LED
#include "Driver_USART.h"               // ::CMSIS Driver:USART
//#include "LPC17xx.h"


#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif
#ifdef RTE_CMSIS_RTOS2                  // when RTE component CMSIS RTOS2 is used
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif
#define SLAVE_I2C_ADDR       0x73			// Adresse esclave sur 7 bits
#ifdef RTE_CMSIS_RTOS2_RTX5
/**
  * Override default HAL_GetTick function
  */

uint32_t HAL_GetTick (void) {
  static uint32_t ticks = 0U;
         uint32_t i;

  if (osKernelGetState () == osKernelRunning) {
    return ((uint32_t)osKernelGetTickCount ());
  }

  /* If Kernel is not running wait approximately 1 ms then increment 
     and return auxiliary tick counter value */
  for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  }
  return ++ticks;
}

#endif

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur);
unsigned char read1byte( unsigned char composant, unsigned char registre);
unsigned char read2byte( unsigned char composant, unsigned char registre);
void Timer_Init(unsigned int prescaler, unsigned int valeur);
void TIMER0_IRQHandler(void);
void ultra(void const* argument);
void terminal(void const* argument);

//typedef struct{
//	char valeurultrason[3];
//	int entier;
//}Lettre;

extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_I2C Driver_I2C1;
unsigned char data;

uint8_t DeviceAddr;
osThreadId ID_tacheultra,ID_tacheterminal;
osThreadDef(ultra,osPriorityNormal,1,0);
osThreadDef(terminal,osPriorityNormal,1,0);
osMailQId ID_BAL ;
osMailQDef (BAL, 10, int) ;   //nom du bal, nombre de message, type de message si char, int ou autre 

void Init_I2C(void){
	Driver_I2C1.Initialize(NULL);
	Driver_I2C1.PowerControl(ARM_POWER_FULL);
	Driver_I2C1.Control(	ARM_I2C_BUS_SPEED,				// 2nd argument = débit
							ARM_I2C_BUS_SPEED_STANDARD  );	// 100 kHz
	//Driver_I2C1.Control(	ARM_I2C_BUS_CLEAR,
							//0 );
}
unsigned char val;
  char valeurultrason[4];
	char valeur[16];
void ultra(void const*argument){
	//Lettre *ptr;
	//osEvent result;
	write1byte( SLAVE_I2C_ADDR,0x00 ,0x50 );
	while (1)
  {
		//ptr = osMailAlloc(ID_BAL, 1000);  //initialisation du pointeur on cherche un case vide
		//ptr ->valeurultrason[0]=read1byte(SLAVE_I2C_ADDR,0x02);
    //ptr ->valeurultrason[1]=read1byte(SLAVE_I2C_ADDR,0x03);
		//ptr ->valeurultrason[2]=(ptr ->valeurultrason[0]<<8) | (ptr ->valeurultrason[1]);
		write1byte( SLAVE_I2C_ADDR,0x01 ,0x06 );
		write1byte( SLAVE_I2C_ADDR,0x00 ,0x51 );
		valeurultrason[0]=read1byte(SLAVE_I2C_ADDR,0x02);
    valeurultrason[1]=read1byte(SLAVE_I2C_ADDR,0x03);
		val=(valeurultrason[0]<<8) | (valeurultrason[1]);
    
		sprintf(valeur,"\n\r %d cm",val);
		while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
		Driver_USART1.Send(valeur,15);
		
		LED_On (1);																																														//while
		LED_Off (2);
		LED_Off (3);
		if (val<13){
			LED_On (2);
			LED_On(3);
		}
		//osDelay(100);
  }
}	
void terminal(void const*argument){
	//char *ptr2,tab[20],val;
	//osEvent result;
	while (1)
  {
//		result=osMailGet (ID_BAL , osWaitForever); //on attend toujours  
//		if( result.status==osEventMail)
//			{    //sortie si on recoit un caractère
//		ptr2=result.value.p;         //on recupere la valeur recu dans le pointeur
//		val= *ptr2; 						//et on pointe la veleur vers un autre pointeur;
//		sprintf(tab,"\n\r %d cm",val);
//		while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
//		Driver_USART1.Send(tab,15);
//		osMailFree(ID_BAL, ptr2);   // on libere le pointeur 
//    
//		osDelay(1000);
//  }
		sprintf(valeur,"\n\r %d cm",valeurultrason[2]);
		while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
		Driver_USART1.Send(valeur,15);
		osDelay(1000);
   }
}
void Init_UART(void){
	Driver_USART1.Initialize(NULL);
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	Driver_USART1.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							115200);
	Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART1.Control(ARM_USART_CONTROL_RX,1);
}


/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user 
             can eventually implement his proper time base source (a general purpose 
             timer for example or other time source), keeping in mind that Time base 
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
             handled in milliseconds basis.
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();

  /* Add your application code here
     */
	LED_Initialize();
  Init_I2C();
	Init_UART();
	
	
//#ifdef RTE_CMSIS_RTOS2	// A commenter si utilisation RTOS
  /* Initialize CMSIS-RTOS2 */
  osKernelInitialize ();
  //write1byte( SLAVE_I2C_ADDR,0x00 ,0x51 );
  /* Create thread functions that start executing, 
  Example: osThreadNew(app_main, NULL, NULL); */
  ID_tacheultra = osThreadCreate(osThread(ultra),NULL);
	//ID_tacheterminal = osThreadCreate(osThread(terminal),NULL);
	//ID_BAL = osMailCreate(osMailQ(BAL), NULL);
  /* Start thread execution */
  osKernelStart();
	osDelay(osWaitForever);
//#endif
  

  /* Infinite loop */
  
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif
void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur)
{
	uint8_t tab[2];
	tab[0]=registre; 
	tab[1]= valeur; 
	
		Driver_I2C1.MasterTransmit (composant, tab, 2, false);		// false = avec stop
		while (Driver_I2C1.GetStatus().busy == 1);	// attente fin transmission
	
	
}

unsigned char read1byte( unsigned char composant, unsigned char registre)
{
	uint8_t tabR[1];
	tabR[0]=registre;
	
	
	Driver_I2C1.MasterTransmit (composant, tabR, 1, true);		// false = avec stop
		while (Driver_I2C1.GetStatus().busy == 1);	// attente fin transmission
	
		Driver_I2C1.MasterReceive (composant, &data, 1, false);		// false = avec stop
		while (Driver_I2C1.GetStatus().busy == 1);	// attente fin transmission
	
	return data;
}	
//void Timer_Init(unsigned int prescaler, unsigned int valeur)
//{
//LPC_SC->PCONP |= (1<<1); //allume le timer 0 (facultatif, déjà allumé après un reset)

//LPC_TIM0->PR =  prescaler;
//LPC_TIM0->MR0 = valeur; 
//LPC_TIM0->MCR = 3;	//reset counter si MR0=COUNTER + interrupt*/

//LPC_TIM0->TCR = 1; //démarre le comptage
//}
//void TIMER0_IRQHandler(void)
//{  
//	  LPC_TIM0->IR |= 1<<0;   /* clear interrupt bit */  
//	  valeurultrason[0]=read1byte(SLAVE_I2C_ADDR,0x02);
//    valeurultrason[1]=read1byte(SLAVE_I2C_ADDR,0x03);
//		valeurultrason[2]=(valeurultrason[0]<<8) | (valeurultrason[1]);
//    
//		sprintf(valeur,"\n\r %d cm",valeurultrason[2]);
//		while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
//		Driver_USART1.Send(valeur,15);
//		
//		LED_On (1);																																														//while
//		LED_Off (2);
//		LED_Off (3);
//		if (valeurultrason[2]<13){
//			LED_On (2);
//			LED_On(3);
//		}
//	
//	
//}
/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
