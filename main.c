/* Includes ------------------------------------------------------------------*/
#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"  
#include "main.h"
#include "Board_LED.h"                  // ::Board Support:LED
#include "Driver_SPI.h"
#include "cmsis_os.h"                   // CMSIS RTOS header file
#include "Driver_USART.h"
#include <string.h>
#include <stdio.h>

#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif
#ifdef RTE_CMSIS_RTOS2                  // when RTE component CMSIS RTOS2 is used
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif

ADC_HandleTypeDef myADC2Handle;


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

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
	
extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_SPI Driver_SPI1;

void mySPI_Thread (void const *argument);                             // thread function
osThreadId tid_mySPI_Thread;                                          // thread id
osThreadDef (mySPI_Thread, osPriorityNormal, 1, 0);                   // thread object

	
ADC_HandleTypeDef myADC2Handle;

//Fonction d'initalisation de l'ADC2, sur PA0, pour la lecture du Ambiente Luminescence Sensor (ALS)
void init_PIN_PA0_ALS()
{
	GPIO_InitTypeDef ADCpin; //Cr�ation de structure de config PIN_ANALOG
	ADC_ChannelConfTypeDef Channel_AN0; // Cr�ation de structure de config ADC
	
	//Intialisation PA0
__HAL_RCC_GPIOA_CLK_ENABLE(); // activation Horloge GPIOA
	ADCpin.Pin = GPIO_PIN_0; // Selection pin PA0
	ADCpin.Mode = GPIO_MODE_ANALOG; // Selection mode analogique
	ADCpin.Pull = GPIO_NOPULL; // D�sactivation des r�sistance de pull-up et pull-down
	
	//Param�trage ADC2 
__HAL_RCC_ADC2_CLK_ENABLE(); // activation Horloge ADC2
	myADC2Handle.Instance = ADC2; // Selection de ADC2
	myADC2Handle.Init.Resolution = ADC_RESOLUTION_8B; // Selection r�solution 8 bits 
	myADC2Handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV; //Selection du mode single pour l'EOC(end of conversion)
	myADC2Handle.Init.DataAlign = ADC_DATAALIGN_RIGHT; // Alignement des data � gauche des octets
	myADC2Handle.Init.ClockPrescaler =ADC_CLOCK_SYNC_PCLK_DIV8; //Syncronisation des horloges
	
	//Param�trage CHANEL0
	Channel_AN0.Channel = ADC_CHANNEL_0; // Selection analog channel 0
	Channel_AN0.Rank = 1; // Selection du Rang : 1
	Channel_AN0.SamplingTime = ADC_SAMPLETIME_15CYCLES; // Selection du temps d'�chentillonage � 15
	
	HAL_GPIO_Init(GPIOA, &ADCpin); // Initialisation PA0 avec les param�tre ci-dessus
	HAL_ADC_Init(&myADC2Handle); // Initialisation ADC2 avec les param�tre ci-dessus
	HAL_ADC_ConfigChannel(&myADC2Handle, &Channel_AN0); // Initialisation Chanel_0 avec les param�tre ci-dessus. 
}

void Init_UART(void){
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
}

void Init_SPI(void){
	Driver_SPI1.Initialize(NULL);
	Driver_SPI1.PowerControl(ARM_POWER_FULL);
	Driver_SPI1.Control(ARM_SPI_MODE_MASTER | 
											ARM_SPI_CPOL1_CPHA1 | 
//											ARM_SPI_MSB_LSB | 
											ARM_SPI_SS_MASTER_UNUSED |
											ARM_SPI_DATA_BITS(8), 1000000);
	Driver_SPI1.Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
}



void mySPI_Thread (void const *argument) {
	char tab[500];
	int i, nb_led, eclairage, ADC_Value;
	int seuil = 200; //Seuil de luminosit� pour le capteur
	while(1){
		
		//Lecture capteur
		HAL_ADC_Start(&myADC2Handle); // start A/D conversion
		
		if(HAL_ADC_PollForConversion(&myADC2Handle, 5) == HAL_OK) //check if conversion is completed
		{
			ADC_Value=HAL_ADC_GetValue(&myADC2Handle); // read digital value and save it inside uint32_t variable
		}
		HAL_ADC_Stop(&myADC2Handle); // stop conversion
		
		//eclairage = 224 - ADC_Value/9;
	
		for (i=0;i<4;i++)
		{
			tab[i] = 0;
		}
	
		// end
		tab[68] = 0; 
		tab[69] = 0; // (7+nb_led*4) -2

		
		// Voir si on peut tout allumer seulement quand on a approch� le RFID
		
		//if(ADC_Value < seuil) //NUIT
		//{
			//2 x 4 leds phares  blancs avant 
			for (nb_led = 0; nb_led <8;nb_led++)
			{
				tab[4+nb_led*4]=0xe8;
				tab[5+nb_led*4]=0xff; //Bleu
				tab[6+nb_led*4]=0xff; //Vert
				tab[7+nb_led*4]=0xff; //Rouge
			}	
		
			// 2 x 4 leds rouges arri�re
			for (nb_led = 8; nb_led <16;nb_led++)
			{
				tab[4+nb_led*4]=0xe8;
				tab[5+nb_led*4]=0x00; //Bleu
				tab[6+nb_led*4]=0x00; //Vert
				tab[7+nb_led*4]=0xff; //Rouge
			}	
			
			// 2 x 4 leds rouges arri�re
			for (nb_led = 8; nb_led <16;nb_led++)
			{
				tab[4+nb_led*4]=0xe8;
				tab[5+nb_led*4]=0x00; //Bleu
				tab[6+nb_led*4]=0x00; //Vert
				tab[7+nb_led*4]=0xff; //Rouge
			}	
		//}
			
		//else if(ADC_Value > seuil) //JOUR
		//{
			//2 x 4 leds phares  blancs avant 
//			for (nb_led = 0; nb_led <8;nb_led++)
//			{
//				tab[4+nb_led*4]=0xe0;
//				tab[5+nb_led*4]=0x00; //Bleu
//				tab[6+nb_led*4]=0x00; //Vert
//				tab[7+nb_led*4]=0x00; //Rouge
//			}	
//		
//		
//			// 2 x 4 leds rouges arri�re
//			for (nb_led = 8; nb_led <16;nb_led++)
//			{
//				tab[4+nb_led*4]=0xe0;
//				tab[5+nb_led*4]=0x00; //Bleu
//				tab[6+nb_led*4]=0x00; //Vert
//				tab[7+nb_led*4]=0x0; //Rouge
//			}	
		//}
		Driver_SPI1.Send(tab,70);
  }
}


int main(void)
{
	osKernelInitialize ();
  HAL_Init();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();
	
	
	LED_Initialize();
	Init_SPI();
	Init_UART();
	init_PIN_PA0_ALS();
	
	tid_mySPI_Thread = osThreadCreate (osThread(mySPI_Thread), NULL);
	
	
	/*
	//Start frame
	tab[0] = 0x00;
	tab[1] = 0x00;
	tab[2] = 0x00;
	tab[3] = 0x00;
	
	tab[4] = 0xFA;
	tab[5] = 0x00;
	tab[6] = 0x00;
	tab[7] = 0xFF;
	
	tab[8] = 0xFF;
	tab[9] = 0xFF;
	tab[10] = 0xFF;
	tab[11] = 0xFF;
	
	Driver_SPI1.Send(tab,12);
*/

  

  osKernelStart();
	osDelay(osWaitForever);
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

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


