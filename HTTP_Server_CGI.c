/*------------------------------------------------------------------------------
 * Serveur Web embarqu� - Script CGI
 * GE2 parcours IT2R - IUT de Cachan
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server_CGI.c
 * Purpose: Serveur web HTTP
 * Modif: le 31/01/2016 pour la Team IT2R
 *----------------------------------------------------------------------------*/
 
#include <stdio.h>
#include <string.h>
#include "rl_net.h"
#include "Board_GLCD.h"

uint8_t ip_addr[NET_ADDR_IP4_LEN];
uint8_t mask[NET_ADDR_IP4_LEN];
uint8_t gateway[NET_ADDR_IP4_LEN];
uint8_t pri_dns[NET_ADDR_IP4_LEN];
uint8_t sec_dns[NET_ADDR_IP4_LEN];

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

char lcd_text[20+1];
char P2=0;

// Fonction de gestions des requetes de type POST.
// \param[in]     code          type de donn�es � g�rer :
//                              - 0 = www-url-encoded form data,
//                              - sinon = autre (hors programme)
// \param[in]     data          pointeur sur donnee POST
// \param[in]     len           longueur donnee POST.
void netCGI_ProcessData (uint8_t code, const char *data, uint32_t len) {
  char var[40];
	int rep,rep1;
	
	if (code != 0) {
    // Les autres codes sont ignor�s
    return;
  }
	
  if (len == 0) {
    // Si pas de donn�e � traiter
    return;
  }
		
//  do {
//    // Appel de la fonction d'analyse de la trame "POST"
//    data = netCGI_GetEnvVar (data, var, sizeof (var));	// var contient la donn�e � g�rer
//    if (var[0] != 0) {
//      // si il y a une donn�e � g�rer
//			//METTRE ICI LE TRAITEMENT DE LA CHAINE ENVOYE PAR LE NAVIGATEUR
//			rep=strncmp("lcd1=",&var[0],5);
//			rep1=strncmp("led0=1",&var[0],6);
//			if(rep==0){
//				strcpy(lcd_text,&var[5]);
//				GLCD_DrawString(0,90,lcd_text);
//			}	
//			if(rep1==0){
//				P2=1;
//			}
//      if(strncmp("led0=0",&var[0],6)==0){
//				P2=0;
//			}	
//			//.............
//			//FIN TRAITEMENT TRAME
//    }
//  } while (data);			// Tant qu'il a a des donn�es � traiter
	LED_SetOut(P2);
}
 
// Fonction de g�n�ration des lignes CGI � mettre � jour
// \param[in]     env           environment string.
// \param[out]    buf           output data buffer.
// \return        number of bytes written to output buffer.
uint32_t netCGI_Script (const char *env, char *buf, uint32_t buf_len, uint32_t *pcgi) {
  uint32_t len = 0;
	uint16_t ultrason=17,ultrasonfacedroit=15,ultrasonfacegauche=23,ultrasonarrieredroit=15,ultrasonarrieregauche=44,ultrasondroit=90,vitesse=25,detect=1,leds=1,journuit=0;         // d�clarations des variables diff�rents ultasons,detect pour le badge et la vitesse
	float latitude=10.5,longitude=5.5;                                            // d�clarations des variables latitude et longitude
	char led[15],alternance[10],etat[10],badge[15],verif[30];                      // d�clarations des variables char led,alternance,etat,badge et verif pour �crire sur le adc cgi
   
  switch (env[0]) {
    
    case 'a':
      // Mise a jour du champ du script CGI
       {len = sprintf (buf, &env[2], lcd_text);
       break;}
		case 'b':                                      // Pour afficher les valeurs des ultrasons
      // Mise a jour du champ du script CGI
		   {
			 if(ultrason<16 || ultrasonarrieredroit<12 || ultrasondroit<10){                    // si obstacle autour de la voiture alors dans l'etat = PB sinon etat = OK
			 sprintf(etat,"PB");
			 }
			 else{
			 sprintf(etat,"OK");
			 }
       len = sprintf (buf, &env[2],ultrason,ultrasonarrieredroit,ultrasondroit,etat); // valeurs des ultrasons
       break;}
		 case 'r':                                       // pour afficher la 2e ligne c'est � dire d'afficher s'il y a un obstacle ou pas autour de la voiture
      // Mise a jour du champ du script CGI
		   {
			 if(ultrason<16){
			 sprintf(verif,"Obstacle en face !");               // si obstacle en face 
			 }
			 if(ultrasonarrieredroit<12){
			 sprintf(verif,"Obstacle derri�re la voiture !");          // si obstacle derriere
			 }
			 if(ultrasondroit<10){
			 sprintf(verif,"Obstacle � droite !");                   // si obstacle � droite
			 }
			 if(ultrason>16 && ultrasonarrieredroit>12 && ultrasondroit>10){
			 sprintf(verif,"Aucun obstacle dangereux !");                    // si 0 obstacle
			 }
       len = sprintf (buf, &env[2],verif); // affiche le char verif
       break;}
		 case 'f':                                    // pour afficher si le badge est activ� ou pas
      // Mise a jour du champ du script CGI
		   {
			 if(detect == 1){
					sprintf(badge,"activ�");              // si badge d�tect� alors "activ�"
			 }
			 if(detect == 0){
					sprintf(badge,"non d�tect�");         // si badge non d�tect� alors "non d�tect�"
			 }
       len = sprintf (buf, &env[7],badge);          // badge oui ou non
       break;}
		 case 'j':                                        // pour afficher si les leds sont allum�es et si jour ou nuit
      // Mise a jour du champ du script CGI
		   {
			 if ( leds==1){
					sprintf(led,"allum�es");                        // si valeur leds renvoy�e par une autre carte == 1 alors "allum�es"
			 }
			 if ( leds==0){
					sprintf(led,"�teintes");                        // si valeur leds renvoy�e par une autre carte == 0 alors "�teintes"
			 }
			 if(journuit==1){
					sprintf(alternance,"jour");                      // si valeur journuit renvoy�e par une autre carte == 1 alors "jour"
			 }
			 if(journuit==0){
					sprintf(alternance,"nuit");                        // si valeur journuit renvoy�e par une autre carte == 0 alors "nuit"
			 }
       len = sprintf (buf, &env[6],led,alternance);          // Led allum�e/�teinte et jour ou nuit
       break;}
		 case 'v':
      // Mise a jour du champ du script CGI
		   {
			 
       len = sprintf (buf, &env[7],latitude,longitude,vitesse);     //affichage de latitude, longitude et la vitesse
       break;}
		 case 'd':
      // Mise a jour du champ du script CGI
		   {
       if(P2==1){
				 len = sprintf (buf, &env[4],"checked");
			 }
			 if(P2==0){
				 len = sprintf (buf, &env[4],"");
			 }
       break;}
			 
    }
  return (len);
}
 