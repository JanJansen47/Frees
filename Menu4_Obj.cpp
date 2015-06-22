#include "registers.h"
#include "Arduino.h"
#include "Menu4_Obj.h" 
#include <LiquidCrystal.h>
/*
 Grbl Controller for the Grbl on a arduino uno.
 This class handles the menu
 Auth: J. Jansen
 Date: 19-06-2015
 Version: 0.1
 This version is for the the 4 lines 20 char display
 */


  //LCD wired on my AtMega2560
  #define RS 44
  #define RW 46
  #define EN 48
  LiquidCrystal LCD(RS, RW, EN, 43, 45, 47, 49);

//**************MENU Configuration*************************
int menu_pos_x;  
int menu_pos_y;  
int button_shift_x ; // x distance between buttons
int button_shift_y ; // y distance between buttons
int max_buttons_y_direction ;
int max_buttons_x_direction ;
//**********************************************************

String files [8];

Menu4_Obj::Menu4_Obj(int A)
{
  int _A = A;
}

int Menu4_Obj::init_LCD(String txt )
{   
  Serial.begin(115200); 
  LCD.begin(20,4);// display met 4  regels
  //-----LCD initializing begins-----//
   Serial.println(" LCD initialized");  
  //***********************************************************
// coordinates menu
  menu_pos_x=0; 
  menu_pos_y=0;
//**************************************************************
String files [8] = String(8);
  return(10);
}


// Welcome menu
void Menu4_Obj::Welcome(int language, int version ) {
LCD.setCursor(0, 0);
LCD.print("Cartridge Frees");
LCD.setCursor(0, 1);
LCD.print("Version: ");
LCD.print(version);

}
// Make a menu
void Menu4_Obj::MakeMenu(int num, int language,  int button[], int size_of_button) {
 
 //button text for 3 languages
  String  textNL[18] = { "",  "Nullen", "Start", "Kies", "Instel" ,"Stop", "Pauze", "Vervolg", "Engels", "Duits", "Brabo", "Taal",  "gsender", "duurp" }; 
  String  textGE[18] = { "",  "eins", "zwei", "drie", "vier" ,"funf", "sechs", "sieben" , "acht" , "Neun", "Zehn" , "Elb" , "Zwolf", "Sprache", "gsender", "dauer"  };
  String  textUK[18] =  { "",  "Home", "Start", "Select", "Setting" ,"Stop", "Pause", "Resume", "English","German" , "Dutch", "Lang", "gsender","endure"  };
for (int j=0;j<3;j++) {LCD.setCursor(0,j-1); LCD.print("                    ");}  // blank first three rows
    
 int i=0;
    //String buttonlabel;   //intermediate variables
    while (i < (size_of_button)) {  // number of selections in one menu
    if (i<3) {LCD.setCursor(0,i);}
    if (i> 3 | i==3) {LCD.setCursor(10,(i-3));}
    LCD.print(" "); // space for >
    if (language==1) {   
      // select language
      LCD.print(textNL[button[i]]);
Serial.println(textNL[button[i]]);
    } 
    if (language==2){ 
      LCD.print (textGE[button[i]]);
    } 
    if (language==3){  
      LCD.print (textUK[button[i]]);
    }
    i++; 
  };
}  // eo Menu

// information takes care of the display of status and error messages


// display the Cartridge files on disk and select one
int Menu4_Obj::show_select_File(String files [], int num_of_files ) {
  int i= 0; 
for (int j=0;j<4;j++) {LCD.setCursor(0,j-1); LCD.print("                    ");}  // blank first three rows
   while (i < (num_of_files+1)) {  // position button in menu
Serial.print("In Menu4   > "); 
 while (i < (num_of_files+1)) {  // position button in menu
    if (i<4) {LCD.setCursor(0,i);}
    if (i>4 ) {LCD.setCursor(10,(i-3));}
    LCD.print(" "); // space for >
    LCD.print( files[i]);    //     
    Serial.println(files[i]);
    i++;
  }
return(1); 
} 
} // eo show_sel
 void Menu4_Obj::information(String info){  // info max 6 chars
  LCD.setCursor(0,4);
  LCD.print("      ");
  LCD.setCursor(0,4);
  LCD.print(info);
  }

// display the x, y position
void Menu4_Obj::position(String position){
  LCD.setCursor(4,4);
  LCD.print("               ");
  LCD.setCursor(4,4);
  LCD.print(position);
   }
//displays the name of the product (cartridge
void Menu4_Obj::product(String product){
  LCD.setCursor(10,4);
  LCD.print("      ");
  LCD.setCursor(10,4);
  LCD.print(product);
 }










