#include "Adafruit_GFX.h"
#include "pin_magic_UNO.h"
#include "pin_magic_MEGA.h"
#include "registers.h"
#include "Adafruit_TFTLCD.h"
#include "Arduino.h"
#include "Menu3_Obj.h" 
/*
 Grbl Controller for the Grbl on a arduino uno.
 This class handles the menu
 Auth: J. Jansen
 Date: 17-02-2015
 Version: 0.2
 This version is for the graphic display 
 */

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
// Adafruit_TFTLCD tft;
//**************MENU Configuration*************************
int menu_pos_x;  
int menu_w;
int menu_pos_y;  
int menu_h;
int button_w;   //button width
int button_h;   //button height 
int button_shift_x ; // x distance between buttons
int button_shift_y ; // y distance between buttons
int max_buttons_y_direction ;
int max_buttons_x_direction ;
//**********************************************************

String files [8];

Menu3_Obj::Menu3_Obj(int A)
{
  int _A = A;

}

/*
Send system commands to Grbl 
 Results in System command output window
 
 */

int Menu3_Obj::init_LCD(String txt )
{   
  Serial.begin(115200); 
  //-----LCD initializing begins-----//
  tft.reset(); 
  int identifier = tft.readID();
  tft.begin(identifier);
  Serial.println(" LCD initialized");  
  tft.fillScreen(RED);
//***********************************************************
// coordinates menu
  menu_pos_x=50; 
  menu_pos_y=10;
// menu size
  menu_w = 150;
  menu_h = 300;
// button properties  
  button_w = 130;
  button_h = 30;
// distances between buttons
  button_shift_x = 150; // x distance between buttons
  button_shift_y = 50; // y distance between buttons
// max buttons in vertical direction
  max_buttons_y_direction = 2;
  max_buttons_x_direction = 3;
//**************************************************************
String files [8] = String(8);
  return(10);
}
// Make a menu
void Menu3_Obj::MakeMenu(int num, int language,  int button[], int size_of_button) {
 
 //button text for 3 languages
  String  textNL[15] = { "",  "Nullen", "Start", "Kies", "Instelling" ,"Stop", "Pauze", "Vervolg", "Engels", "Duits", "Brabants", "Taal",  "gcodesender" }; 
  String  textGE[15] = { "",  "eins", "zwei", "drie", "vier" ,"funf", "sechs", "sieben" , "acht" , "Neun", "Zehn" , "Elb" , "Zwolf", "Sprache", "gcodesender" };
  String  textUK[15] =  { "",  "Home", "Start", "Select", "Settings" ,"Stop", "Pause", "Resume", "English","German" , "Dutch", "Language", "gcodesender" };
  
        
  
 //paintmenu(menu_pos_x,menu_pos_y,menu_w, menu_h); 
  tft.fillScreen(RED);
  int i= 0; 
  int X; 
  int Y;
   //first select the menu
    
  // location of the first button
  X = menu_pos_x -30;   //intermediate variables Rara
  Y = menu_pos_y +35;
  String buttonlabel;   //intermediate variables
    while (i < (size_of_button)) {  // position button in menu
   
    if (language==1) {              // select language
      buttonlabel = textNL[button[i]];
    } 
    if (language==2){ 
      buttonlabel = textGE[button[i]];
    } 
    if (language==3){  
      buttonlabel = textUK[button[i]];
    }
    
    paintbutton( buttonlabel,X,Y);    // create the button on the display
    if ( i <  (max_buttons_x_direction-1)) { Y +=button_shift_y;}
    if ( i == (max_buttons_x_direction-1)) { X +=button_shift_x; Y = menu_pos_y +35; }
    if ( i >  (max_buttons_x_direction-1)) { Y +=button_shift_y;}
    i++;
  }

  switch (num) {
  case 0:  
    {  // menu 1

      break;
    }
  case 2:    // menu 2
    break;
 
}  // eo Menu
// information takes care of the display of status and error messages
}

// display the Cartridge files on disk and select one
int Menu3_Obj::show_select_File(String files [], int num_of_files ) {

 tft.fillScreen(RED);
  int i= 0; 
  int X; 
  int Y;
   //first select the menu
    
  // location of the first button
  X = menu_pos_x -30;   //intermediate variables Rara
  Y = menu_pos_y +35;
  String buttonlabel;   //intermediate variables
    while (i < (num_of_files)) {  // position button in menu
Serial.print("In menu3   > "); 
 Serial.println(files[i]); 


    paintbutton( files[i] ,X,Y);    // create the button on the display
    tft.setCursor(X, Y);
  tft.setTextColor(RED);
  tft.setTextSize(2);
  tft.setRotation(1); //Landscape
 // tft.println(files[i]);
    if ( i <  (max_buttons_x_direction-1)) { Y +=button_shift_y;}
    if ( i == (max_buttons_x_direction-1)) { X +=button_shift_x; Y = menu_pos_y +35; }
    if ( i >  (max_buttons_x_direction-1)) { Y +=button_shift_y;}
    i++;
  }


return(1); 
} 

 void Menu3_Obj::information(String info){
 int information_pos_x =20;
 int information_pos_y = 5;
 int information_w = 150;
 int information_h = 20; 
  paintmenu(information_pos_x,information_pos_y,information_w, information_h); 
  painttxt( info, information_pos_x, information_pos_y);
 }

// display the x, y position
void Menu3_Obj::position(String position){
 int information_pos_x =20;
 int information_pos_y = 200;
 int information_w = 130;
 int information_h = 20; 
  paintmenu(information_pos_x,information_pos_y,information_w, information_h); 
  painttxt( position, information_pos_x, information_pos_y);
 }
//displays the name of the product (cartridge
void Menu3_Obj::product(String product){
 int information_pos_x =170;
 int information_pos_y = 200;
 int information_w = 120;
 int information_h = 20; 
  paintmenu(information_pos_x,information_pos_y,information_w, information_h); 
  painttxt( product, information_pos_x, information_pos_y);
 }
//this function paints a menu starting at location mpx,mpy with height mg and width mw
void Menu3_Obj::paintmenu(int mpx, int mpy, int mw, int mh     ) {
  tft.fillRect(mpx, mpy, mw, mh, WHITE);
}

// this function creates a button starting at x,y with text button label
void Menu3_Obj::paintbutton(String buttonlabel,int x, int y ) {
   
// button size
  

  tft.setRotation(1); //Landscape
  tft.fillRect(x, y, button_w, button_h, GREEN);
  tft.setCursor(x+10, y+10);  // was 10 10
  tft.setTextColor(BLUE);
  tft.setTextSize(2);
  tft.println(buttonlabel);
  }

// this function prints text starting at y, y
void Menu3_Obj::painttxt(String txt,int x, int y ) {
  tft.setCursor(x, y);
  tft.setTextColor(RED);
  tft.setTextSize(2);
  char tekst[txt.length()+1];
  txt.toCharArray(tekst, sizeof(tekst));
  tft.setRotation(1); //Landscape
  tft.println(txt);
  }





