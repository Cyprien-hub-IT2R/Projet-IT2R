/*------------------------------------------------------------------------------
 * Serveur Web embarqué 
 * GE2 parcours IT2R - IUT de Cachan
 *------------------------------------------------------------------------------
 * Name:    main.c
 * Purpose: Serveur web HTTP
 * Modif: le 31/01/2016 pour la Team IT2R
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include "cmsis_os.h"                   /* CMSIS RTOS definitions             */
#include "rl_net.h"                     /* Network definitions                */

#include "Board_GLCD.h"
#include "GLCD_Config.h"
#include "Board_LED.h"
#include "Board_Buttons.h"
#include "Board_ADC.h"

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

/*----------------------------------------------------------------------------
  Thread principale : gestion réseau
 *---------------------------------------------------------------------------*/
int main (void) {
	LED_Initialize     ();
  ADC_Initialize     ();
  GLCD_Initialize         ();
  GLCD_SetBackgroundColor (GLCD_COLOR_WHITE);
  GLCD_SetForegroundColor (GLCD_COLOR_BLUE);
  GLCD_ClearScreen        ();
  GLCD_SetFont            (&GLCD_Font_16x24);
  GLCD_DrawString         (0, 1*24, "Cloud Donnee voiture");
	GLCD_DrawString         (0, 2*24, "IT2R Power !");
	
	netInitialize     ();
	
  while(1) {
  }
	
	return 0;
}
