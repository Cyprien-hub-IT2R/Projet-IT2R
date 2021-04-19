t <html>
t <head>
t  <title>Cloud donnée Voiture</title>
t </head>
t
t <body bgcolor="#FFFFFF" leftmargin="0" topmargin="10" marginwidth=
t "0" marginheight="0">
t  <div align="center" style="width: 833; height: 470">
t    	<table style="border: 1px solid #000080" height="384"
t    	cellspacing="0" cellpadding="0" width="815">
t      		<tbody>
t        		<tr bgcolor="#EEEEEE">
t          			<td style="border-bottom: 1px solid #000080" valign=
t          			"bottom" nowrap height="70" margin="50" width="567">
t            		<h2 align="center"><font face="verdana" color=
t            		"#FE2E2E">IT2R : Véhicule Webserveur</font></h2>
t          			</td>
t
t        			<td style="border-bottom: 1px solid #000080" valign=
t          			"center" nowrap height="73" width="240"><a href=
t          			"http://www.iut-cachan.u-psud.fr/fr/index.html"><img border="0" src="IT2R.png"
t          			align="left" width="151" height="47"></a></td>
t
t 					<td style="border-bottom: 1px solid #000080" align=center vAlign=center noWrap width="70"> 
t   				<a href="index.htm"><img border="0" src="home.png" width="31" height="29"></a>
t  					</td>
t        		</tr>
t
t        		<tr>
t          			<td colspan="5" height="400" width="805" bgcolor="#73C2FB">
t            			<div align="center">
t              				<center>
t                			<table width="90%" border="0">
t                  			<tbody>
t                    			<tr>
t                     				<td width="95%">
t 										<h2 align="center"><br>
t           							<b><font face="verdana" color=
t           							"#003366">Récupération des données de la voiture</font></b></h2>
t 										<p align="center"><img border="0" height="140" width="150"
t                         				src="IUT-CACHAN.png"></a></p>
t 										<p><font size="5">DATA UDP transmis par le véhicule</font></p>
t 										<form action=adc.cgi method=post name=cgi>
t 										<input type=hidden value="adc" name=pg>
t 											<table border=0 width=100%><font size="3">
t 												 <tr bgcolor=#aaccff>
t  													<th width=40%>Données</th>
t  													<th width=80%>Valeurs</th><th width=10%>Etats</th></tr>
# Cette partie contient les informations CGI qu'il va falloir traiter
t 												<tr>
t 													<td>  Distance de l'obstacle : (ultrasons) </td>
c b <td> face: %d cm     arrière: %d cm     droit: %d cm </td><td> %s</td></tr>
c r <tr> <td> verification d'obstacle :</td><td> %s </td><td> okk</td></tr>
c f <tr> <td> présence du badge :</td><td> fonction (activé ou non): %s </td><td> okk</td></tr>
c j <tr> <td> état des leds :</td><td> Leds: %s et alternance (Jour/Nuit): %s </td><td> okk</td></tr>
c v <tr> <td> GPS :</td><td> Latitude: %f longitude: %f vitesse: %d km/h</td><td> okk</td></tr>
t 												</font></table>
# Définition du bouton Rafraichir
t 										<p align=center>
t 											<input type=submit name=set value="Rafraîchir" id="sbm">
t 										</p>
t 										</form>
t       							</td>
t      							</tr>
t 							</tbody>
t 							</table>
t     						</center>
t    					</div>
t   				</td>
t  				</tr>
t 				<tr>
t  					<td colSpan=5 height=20 width="805">
t   				<p align=center><font face="Verdana" size="1"><a href="http://www.keil.com/company/copyright.asp">
t    				Copyright</a> © 2004-2014 <a href="http://www.keil.com/company/">KEIL - An ARM Company</a> 
t    				All rights reserved.</font></p>
t  					</td>
t 				</tr></tbody></table>
t 	</div>
t </body>
t </html>
. End of script.