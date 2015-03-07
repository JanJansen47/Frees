#include "Arduino.h"

/*
 Grbl Controller for the Grbl on a arduino uno.
 This class handles the communication with the Grbl Controller. 
 Auth: J. Jansen
 Date: 17-02-2015
 Version: 0.1
 
 */
#ifndef Menu3_Obj_h
#define Menu3_Obj_h



class Menu3_Obj
{
public:
Menu3_Obj(int A);  //constructor
int init_LCD(String txt );
void Welcome(int language, int version );
void MakeMenu(int num,int language, int button[], int size_of_button);
int show_select_File(String files [], int num_of_files );   // Show the avalaible files (for cartridges and select one
void position(String position); // display position information 
void information(String info);  // display error and other messages
void product(String product); // display name of the product
String files [8];

private:
void paintmenu(int mpx, int mpy, int mw, int mh );
void paintbutton(String buttonlabel, int x, int y );
void painttxt(String txt,int x, int y );
String  textNL[18] ;
String  textGE[18] ;
String  textUK[18] ;
String buttonlabel;
int menu_pos_x;  
int menu_w;
int menu_pos_y;  
int menu_h;
int button_w; 
int button_h;
int button_shift_x; 
int button_shift_y;
//int button[];
};
#endif
