/***********************************************************************

Deze configuratiefile is er om te kiezen tussen de MEGA  en Teensy configuratie voor het 4 regelig display
MEGAmarc:
De bedrading van het printje van Marc route de verbindingen zoals via de ramps 1.4 wiring.
MEGAjan
De bedrading van Jan is gebaseerd op het zoveel mogelijk een op een gebruik maken van de verbindingen en enkele omleidingen
Teensy:
De verbindignen voor de Teensy 3.1 zijn een van de vele configuraties.
*/

// Kies een van de bedradingsschema's:
//#define MEGAmarc
#define MEGAjan
//#define Teensy

// Dit is een maat voor de snelheid van de spindel. 
// S250 betekent 0 RPM S0 betekent max RPM. (het Spindel PMW signaal moet geinverteerd worden
#define Start_spindel_speed "S250"
