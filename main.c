/* Includes ------------------------------------------------------------------*/
#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include <stdio.h>
#include "Board_LED.h"                  // ::Board Support:LED
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_hal_conf.h"         // Keil::Device:STM32Cube Framework:Classic
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "main.h"
#include <string.h>

#include "Driver_SPI.h"


#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif
#ifdef RTE_CMSIS_RTOS2                  // when RTE component CMSIS RTOS2 is used
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif
#ifdef RTE_CMSIS_RTOS2_RTX5


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

osThreadId ID_rfid;

extern ARM_DRIVER_SPI Driver_SPI1;

//Fonction d'initialisation SPI
void Init_SPI(void){
	Driver_SPI1.Initialize(NULL);
	Driver_SPI1.PowerControl(ARM_POWER_FULL);
	Driver_SPI1.Control(ARM_SPI_MODE_MASTER | 
	ARM_SPI_CPOL1_CPHA1 | 
	ARM_SPI_MSB_LSB | 
	ARM_SPI_SS_MASTER_UNUSED |
	ARM_SPI_DATA_BITS(8), 1000000);
	Driver_SPI1.Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
}

//Création de 3 tableaux pour Allumer_LED(), Eteindre_LED() et Allumer_LEDNON()
char tab[250], tab1[250], tab2[250];


void AllumerLED()
{
	int i, nb_led;
	
	for (i=0;i<4;i++)
	{
		tab[i] = 0;
	}
	
	// end
	tab[53] = 0; 
	tab[54] = 0; // (7+nb_led*4) -2
	
	//Tous les phares oranges
	for (nb_led = 0; nb_led <12;nb_led++)
	{
		tab[4+nb_led*4]=0xef;
		tab[5+nb_led*4]=0x00; //Bleu
		tab[6+nb_led*4]=0x25; //Vert
		tab[7+nb_led*4]=0xff; //Rouge //Orange 0025ff
	}	
	Driver_SPI1.Send(tab,54); //Envoi de la trame
}

void AllumerLEDNON()
{
	int i, nb_led;
	
	for (i=0;i<4;i++)
	{
		tab2[i] = 0;
	}
	
	// end
	tab2[53] = 0; 
	tab2[54] = 0; // (7+nb_led*4) -2
	
	//Tous les phares rouges
	for (nb_led = 0; nb_led <12;nb_led++)
	{
		tab2[4+nb_led*4]=0xef;
		tab2[5+nb_led*4]=0x00; //Bleu
		tab2[6+nb_led*4]=0x00; //Vert
		tab2[7+nb_led*4]=0xff; //Rouge
	}	
	Driver_SPI1.Send(tab2,54); //Envoi de la trame
}

void EteindreLED()
{
	int i, nb_led;
	
	for (i=0;i<4;i++)
	{
		tab1[i] = 0;
	}
	
	// end
	tab1[52] = 0; 
	tab1[53] = 0; // (7+nb_led*4) -2
	
	//Tous les phares éteints
	for (nb_led = 0; nb_led <12;nb_led++)
	{
		tab1[4+nb_led*4]=0xe0;
		tab1[5+nb_led*4]=0x00; //Bleu
		tab1[6+nb_led*4]=0x00; //Vert
		tab1[7+nb_led*4]=0x00; //Rouge
	}	

	Driver_SPI1.Send(tab1,54); //Envoi de la trame
}


void rfidUART(void const*argument){
	char rfid[14], chaine_rfid[8]
	char idValide[8] = {'0','0','C','6','F','8','5','F'};	//ID du badge correct
	int i;
	
	while (1)
  	{
		Driver_USART1.Receive(rfid,14); 	//Réception de 14 caractères à mettre dans le tableau RFID[]
		while(Driver_USART1.GetRxCount()<1);	
		osDelay(300);
		
		//Le numéro du badge est décalé, on doit supprimer les bits de start et autres qui ne nous concernent pas
		for(i=0;i<8;i++)
		{
			chaine_rfid[i]=rfid[i+3];	
		}		
			if(strncmp(chaine_rfid,idValide,8)==0) //Les deux ID sont les mêmes
			{
				//On fait clignoter à intervalles irréguliers
				AllumerLED();
				osDelay(300);
				EteindreLED();
				osDelay(350);
				AllumerLED();
				osDelay(600); 
				EteindreLED();				
			}
			
			else if(strncmp(chaine_rfid,idValide,8)!=0) //Les 2 ID sont différents
			{
				//On fait clignoter à intervalles réguliers et plus rapides en rouge
				AllumerLEDNON();
				osDelay(200);
				EteindreLED();
				osDelay(200);
				AllumerLEDNON();
				osDelay(200);
				EteindreLED();
				osDelay(200);
				AllumerLEDNON();
				osDelay(200);
				EteindreLED();			
			}
		}
}


/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

osThreadDef(rfidUART,osPriorityNormal,1,0);  

int main(void)
{
	osKernelInitialize ();
  	HAL_Init();

  	/* Configure the system clock to 168 MHz */
  	SystemClock_Config();
  	SystemCoreClockUpdate();

	LED_Initialize();
	Init_UART();
	Init_SPI();

  	ID_rfid = osThreadCreate(osThread(rfidUART),NULL);

  	/* Start thread execution */
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

