#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "GPIO.h"          
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "stdio.h"
#include "GPIO_LPC17xx.h"
#include <LPC17xx.h>
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include <stdlib.h>
extern ARM_DRIVER_USART Driver_USART1;
extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

void tache1 (void const *arg);
void Init_UART(void);
void Init_PWM_MCC(void);
void Timer_Init(void);
void Avance_Recule(int a,int x);
void Tourne(int z);


float a;
char etat=0;

osThreadId ID_tache1;
osThreadDef (tache1,osPriorityNormal,1,0);


void tache1 (void const *arg)  //PWM
{
	char GLCD[20],direction[10];
	int INA,INB;
	char recuN[6],vitesse, recuY,recuX,X;
	signed char Y;
	int PWM;
  
  char etat = 0;
	LPC_TIM0->MR0 = 500000;    // Ceci ajuste la p?riode de la PWM ? 20 ms
  
	while(1)
	{
    /*R�ception Bluetooth par USART, de la position X et Y du Nunchuk*/
    /*****************************************/
		Driver_USART1.Receive(&recuY,1);
		while (Driver_USART1.GetRxCount() < 1);
		Driver_USART1.Receive(&recuX,1);
		while (Driver_USART1.GetRxCount() < 1);
    /*****************************************/
		
		Y = recuY - 124;                     //r�-ajustement de Y pour pouvoir traiter l'information sur l'axe Y
		
    recuX=234-recuX+32;                  //afin de tourner le Nunchuk dans le m�me sens que les roues
		a=((recuX-31.0)/4000.0)+0.05;        //traitement du coefficient directeur pour les servomoteurs
		
		if (a>0.090)
		{
			a=0.090;
		}
		else if (a<0.060)
		{
			a=0.060;
		}
    /*Traitement de la MCC*/
    /*****************************************/
		if(Y<0)
		{
		GPIO_PinWrite(0,16,false);          //INA     /*Permet de reculer*/
		GPIO_PinWrite(0,17,true);           //INB	
		direction[0]='r';
		}
		else if (Y>0)
		{
		GPIO_PinWrite(0,16,true);           //INA     /*Permet d'avancer*/
		GPIO_PinWrite(0,17,false);          //INB	
		direction[0]='a';
		}
		else
		{
			Y=0;
		}
		
     /*Saturation*/
    /*****************************************/
		if (Y > 95)
		{
			Y = 95;
		}
		else if (Y < -95)
		{
			Y = -95;
		}
    /*****************************************/
    
		vitesse=abs(Y);
		PWM = (int) (vitesse*2499)/95;     //Calcul de la valeur du rapport cyclique de la MCC
		if (PWM > 2499)
		{
			PWM = 2499;
		}
		
		LPC_PWM1->MR2 = PWM/2;              // ceci ajuste la duree de l'etat haut ; mr2 pour P3.25
    /*****************************************/
	}
}

/*
 * main: initialize and start the system
 */
int main (void) {
  
  /*Initialisation (Timer,UART,GPIO,GLCD)*/
	/*****************************************/
  osKernelInitialize ();                    
	Init_UART();
  Initialise_GPIO();
	GLCD_Initialize();
	GLCD_ClearScreen();
  Timer_Init();
	GLCD_SetFont(&GLCD_Font_16x24);
	/*****************************************/
  
  
  /*Avant et Arri�re de la MCC, Configuration des broches INA et INB*/
	GPIO_SetDir(0,16,GPIO_DIR_OUTPUT);           //INA (0.16) en sortie
	GPIO_SetDir(0,17,GPIO_DIR_OUTPUT);           //INB (0.17) en sortie
  
  /*Configuration de la PWM de la MCC*/
  /*****************************************/
  Init_PWM_MCC();
  /*****************************************/
  
  /*Active l'interruption et choisi son importance*/
  NVIC_SetPriority(TIMER0_IRQn,0);
	NVIC_EnableIRQ(TIMER0_IRQn);
  
  ID_tache1 = osThreadCreate(osThread(tache1),NULL);


  osKernelStart ();                           // start thread execution 
	osDelay(osWaitForever);
}

void Init_UART(void)                          /*Initialisation de la liaison USART*/
{                       
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

void Init_PWM_MCC(void)
{
  LPC_SC->PCONP = LPC_SC->PCONP | 0x00000040;   // enable PWM1
	LPC_PWM1->PR = 0;  // prescaler
  LPC_PWM1->MR0 = 2499;    // Ceci ajuste la p?riode de la PWM ? 48 us
	LPC_PWM1->MR2 = 0;    // ceci ajuste la duree de l'etat haut ; mr2 pour P3.25
	LPC_PINCON->PINSEL7 |= (3<<18); //  P3.25 est la sortie PWM Channel 3 de Timer 1
	LPC_PWM1->MCR = LPC_PWM1->MCR | 0x00000002; // Timer relance quand MR0 repasse � 0
	LPC_PWM1->LER = LPC_PWM1->LER | 0x00000005;  // ceci donne le droit de modifier dynamiquement la valeur du rapport cyclique
	                                             // bit 0 = MR0    bit2 = MR2
	LPC_PWM1->PCR |= (1<<10);  // autorise la sortie PWM                           
  LPC_PWM1->TCR = 1;  /*validation de timer 1 et reset counter */
}

void Timer_Init(void)                        /*Initialisation du Timer pour cr�er une PWM (Servomoteurs)*/
	{
	LPC_SC->PCONP |= (1<<1);                   /*enable MAT0*/
	LPC_TIM0->PR = 0;                          /*prescaler*/
	LPC_PINCON->PINSEL7 |= (1<<21);            /* P3.26 est la sortie MAT0.1*/
	LPC_TIM0->MCR |= 3;                        /*RAZ du compteur MR si correspondence avec MR0 et MR1, et interruption*/
	LPC_TIM0->EMR |= (3<<6) ;                  /*inverse la sortie MAT0.1, mettre 11 dans les bits 6 et 7*/
  LPC_TIM0->TCR = 1;                         /*validation de timer 1 et reset counter */
} 

void TIMER0_IRQHandler(void)                 /*Interruption d�s que le Timer atteint le seuil de MR0*/
{
	static int mr0;
	Allumer_1LED(1);                           /*Principe de fonctionnement*/
	LPC_TIM0->IR=1;                            /*Impose un Timer avec un MR0 selon le rapport cyclique d�cid�*/
	if (etat==0){                              /*Selon l'�tat, change de niveau afin d'obtenir une PWM*/
			mr0=500000*a;                          /*Etat 0 : Cr�er le niveau heut de la PWM (mr0 =500000*a), pourcentage de 20 ms entre 5% (1 ms) et 10% (2 ms)*/
			etat=1;                                /*Etat 1 : Cr�er le niveau bas de la PWM avec la diff�rence selon la p�riode initiale (20 ms - (500000*a))*/
	}                                          
	else if (etat==1){
		mr0=500000-(500000*a);
		etat=0;
	}
	LPC_TIM0->MR0 = mr0;                       /*Impose le timer selon le mr0 voulu*/
}
 
void Avance_Recule(int a,int x)              /*param�tre a : 1 (avancer) 0 (reculer) et param�tre x : 0-100 (pourcentage de vitesse)*/
{
if (a==1)
{
GPIO_PinWrite(0,16,false);                   //INA     /*Permet de reculer*/
GPIO_PinWrite(0,17,true);                    //INB
}
else if(a==0)
{
GPIO_PinWrite(0,16,true);                    //INA     /*Permet d'avancer*/
GPIO_PinWrite(0,17,false);                   //INB	
}
LPC_PWM1->MR2 = x*25;                        //Choisi le rapport cyclique entre 0 et 2500
}

