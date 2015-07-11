#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include "M2tk.h"
#include "utility/m2ghlc.h"
#include "Serial_Obj.h"
#include "SD_Obj.h"
/*
 Grbl Controller for the Grbl on a arduino uno.
 Auth: J. Jansen
 Date: 17-02-2015
 Date: 30-06-2015 Rel3

 Connections:
 Naar "Smartcontroller"
 MEGA    con     name
 50      X2-1    MISO
 51      X2-6    MOSI
 52      X2-2    CLK
 53      X2-4    CS
 40      X2-5    ENCA
 41      X2-3    ENCB
 39      X1-2    ENC-Push_Button
 44      X1-4    LCDRS
 48      X1-3    LCDE
 43      X1-5    LCD4
 45      X1-6    LCD5
 47      X1-7    LCD6
 49      X1-8    LCD7
 GND     X1-9    GND
 VCC     X1-10   VCC

 Naar Grbl UNO
 MEGA    name      UNO    name
 15       RX3      1      TX
 14       TX3      0      RX
 GND      GND      GND    GND
 */

SD_Obj        YY = SD_Obj();         // instantiate object to use SD disk
Serial_Obj    XX = Serial_Obj(22);  // instantiate object to use communication with Grbl Uno
LiquidCrystal lcd(44, 48, 43, 45, 47, 49);


uint8_t x_pos = 0;
uint8_t y_pos = 0;
int8_t z_pos = 0;
uint8_t versie = 13;
uint8_t select_sc2;
const int buttonPin = 24;  // for test only
String cartridgeFile = "frame1.g";
int lineCounter;  // the number of lines in a gcode file
// iformation of the selected cartride gcode and index information
char gcode_buffer_first_set[30][30];  //first set of gcodes
char gcode_buffer_second_set[30][30];  // second set of gcodes
float STARTCO[2];  // x and y start coordinates
float PITCH[2];  // deltas in x and y between cartridges
int INDEX[2];     // number of cartridges in x and y
int GCODENUM;  // gcode = 1 => only one list of gcodes (e.g. for the lid or cartridge inner
// gcodes = 2 => gcodes for the lid and cartridge inner

// Forward declarations
M2_EXTERN_VLIST(el_list3);
M2_EXTERN_VLIST(opstart);
M2_EXTERN_VLIST(list_sc2);
M2_EXTERN_VLIST(top_el_expandable_menu);
M2_EXTERN_VLIST(el_list_home);
M2_EXTERN_VLIST(el_list_mill);
M2_EXTERN_VLIST(el_list_mill_s);
M2_EXTERN_VLIST(el_list_cartridge);
M2_EXTERN_VLIST(el_list_run);
M2_EXTERN_VLIST(el_list_pause);

M2tk m2(&opstart, m2_es_arduino_rotary_encoder, m2_eh_4bd, m2_gh_lc);

//=================================================
// Forward declaration of the toplevel element


// Left entry: Menu name. Submenus must have a '.' at the beginning
// Right entry: Reference to the target dialog box (In this example all menus call the toplevel element again
m2_menu_entry m2_2lmenu_data[] =
{
  { "HOME", &el_list_home },
  { "SELECT CARTRIDGE", &top_el_expandable_menu },
  { ". START FREZEN 1 ", &el_list_run},
  { ". SELECT CARTRIDGE", &el_list_cartridge},
  { ". SPECIAL", &top_el_expandable_menu },
  { "SPECIAL", &top_el_expandable_menu },
  { ". HOME", &el_list_home },
  { ". JOG", &el_list3},
  { ". EXCHANG MILL", &el_list_mill },
  { ". GCODE_SENDER", &top_el_expandable_menu },
  { ". BACK", &top_el_expandable_menu },
  { NULL, NULL },
};

// The first visible line and the total number of visible lines.
// Both values are written by M2_2LMENU and read by M2_VSB
uint8_t m2_2lmenu_first;
uint8_t m2_2lmenu_cnt;

// M2_2LMENU definition
// Option l4 = four visible lines
// Option e1 = first column has a width of 1 char
// Option w12 = second column has a width of 12 chars

M2_2LMENU(el_2lmenu, "l4e1w16", &m2_2lmenu_first, &m2_2lmenu_cnt, m2_2lmenu_data, '+', '-', '\0');
M2_VSB(el_vsb, "l4w1r1", &m2_2lmenu_first, &m2_2lmenu_cnt);
M2_LIST(list_2lmenu) = { &el_2lmenu, &el_vsb };
M2_HLIST(top_el_expandable_menu, NULL, list_2lmenu);


//opstartscherm (scherm1)
M2_LABEL(opstart_1, "x1y3", "CARTRIDGE MILL");
M2_LABEL(opstart_2, "x1y2", "Version: ");
M2_LABEL(opstart_3, "x10y2" , "14");
M2_BUTTON(opstart_ok, "x1y0", "STARTUP", fn_opstarten);
M2_LIST (list_opstart) = {&opstart_1, &opstart_2, &opstart_3, &opstart_ok};
M2_XYLIST(opstart, NULL, list_opstart);

// Start (from begin)
M2_LABEL(run_label, "x1y3", "HP-32");
M2_BUTTON(run_ok, "x1y1", "START", fn_run);
M2_LIST (list_run) = {&run_label, &run_ok};
M2_XYLIST(el_list_run, NULL, list_run);

// Pause 
M2_LABEL(pause_label, "x1y3", "HP-32");
M2_BUTTON(pause_ok, "x1y1", "PAUZE", fn_run);
M2_LIST (list_pause) = {&pause_label, &pause_ok};
M2_XYLIST(el_list_pause, NULL, list_pause);

//Home
M2_BUTTON(home_start, "x1y0", "start", fn_homing_start);
M2_LIST (list_home) = { &home_start};
M2_XYLIST(el_list_home, NULL, list_home);

// Select cartridge
M2_LABEL(cartride_label, "x1y3", "CARTRIDGE");
M2_BUTTON(cartridge_ok, "x1y0", "START SELECT", fn_cartridge);
M2_LIST (list_cartridge) = {&cartride_label, &cartridge_ok};
M2_XYLIST(el_list_cartridge, NULL, list_cartridge);


//SPECIAL : Joggen
M2_LABEL(el_l2, "x0y2", "X: ");
M2_U8NUM(el_u2, "c2x2y2", 0, 99, &x_pos);
M2_LABEL(el_l3, "x6y2", "Y: ");
M2_U8NUM(el_u3, "c3x7y2", 0, 120, &y_pos);
M2_LABEL(el_l4, "x12y2", "Z: ");
M2_S8NUM(el_u4, "c3x14y2", -50, 0, &z_pos);
M2_BUTTON(el_ok3, "x1y0", "Back", fn_ok3);
M2_LIST(list3) = {  &el_l2, &el_u2 , &el_l3, &el_u3, &el_l4, &el_u4 , &el_ok3};
//M2_XYLIST(el_list3, NULL, list3);

//SPECIAL: Joggen 2
M2_LABEL(jogx_label, "x0y3", "X: ");
M2_U8NUM(jogx_num, "c2x2y3", 0, 99, &x_pos);
M2_BUTTON(jog_xp, "x8y3", "+", fn_jog_xp);
M2_BUTTON(jog_xm, "x14y3", "-", fn_jog_xm);
M2_LABEL(jogy_label, "x0y2", "Y: ");
M2_U8NUM(jogy_num, "c3x2y2", 0, 120, &y_pos);
M2_BUTTON(jog_yp, "x8y2", "+", fn_jog_yp);
M2_BUTTON(jog_ym, "x14y2", "-", fn_jog_ym);
M2_LABEL(jogz_label, "x0y1", "Z: ");
M2_S8NUM(jogz_num, "c3x2y1", -50, 0, &z_pos);
M2_BUTTON(jog_zp, "x8y1", "+", fn_jog_zp);
M2_BUTTON(jog_zm, "x14y1", "-", fn_jog_zm);
M2_BUTTON(jog_but, "x1y0", "Back", fn_ok3);
M2_LIST(list_jog) = {  &jogx_label, &jogx_num, &jog_xp, &jog_xm, &jogy_label, &jogy_num, &jog_yp, &jog_ym, &jogz_label, &jogz_num, &jog_zp, &jog_zm, &jog_but};
M2_XYLIST(el_list3, NULL, list_jog);


//SPECIAL: Exchange mill start
M2_LABEL(mill_label_s, "x1y3", "EXCHANGE MILL");
M2_BUTTON(mill_s, "x1y0", "EXCHANGE", fn_mill_s);
M2_LIST (list_mill_s) = {&mill_label_s, &mill_s};
M2_XYLIST(el_list_mill, NULL, list_mill_s);

//=================================================
// Arduino Setup & Loop

void setup() {
  Serial.begin(115200);
  Serial.println(" start controller -------");
  delay(100);
  XX.flush();
  YY.init_SD();
  /*
  lineCounter = YY.open_File(cartridgeFile);
  Serial.print(" no lines:  ");
  Serial.println( lineCounter);
  for (int i=0; i <lineCounter;) {
    Serial.print( i);
    Serial.print("  :");
    Serial.println(YY.read_line_from_open_File());
    i++;
  }
  YY.close_File();
  */
  for (int i = 0; i < 30; i++) {
    for (int j = 0; j < 30, j++;) {
      gcode_buffer_first_set[i][j] = NULL; gcode_buffer_second_set[i][j] = NULL;
    };
  }
  //*********************************test***************************
  YY.fillCartridgeData(cartridgeFile, STARTCO, PITCH, INDEX, &GCODENUM , gcode_buffer_first_set, gcode_buffer_second_set );
  Serial.print("startcoordinaten0:  ");
  Serial.println(STARTCO[0]);
  Serial.print("startcoordinaten1:  ");
  Serial.println(STARTCO[1]);
  Serial.print("Pitch x:  ");
  Serial.println(PITCH[0]);
  Serial.print("Pitch y:  ");
  Serial.println(PITCH[1]);
  Serial.print("INDEX x:  ");
  Serial.println(INDEX[0]);
  Serial.print("INDEX y:  ");
  Serial.println(INDEX[1]);
  Serial.print("gcodenum:  "); Serial.println(GCODENUM);
  Serial.println("main first set");
  for (int i = 0; i < 30; i++) {
    if (gcode_buffer_first_set[i][0] != NULL) {
      Serial.println(gcode_buffer_first_set[i]);
    }
  }
  Serial.println("main second set");
  for (int i = 0; i < 30; i++) {
    if (gcode_buffer_second_set[i][0] != NULL) {
      Serial.println(gcode_buffer_second_set[i]);
    }
  }
  //*****************************eo test *****************************

  pinMode(14, OUTPUT); // test only
  pinMode(buttonPin, INPUT_PULLUP); //
  m2_SetLiquidCrystal(&lcd, 20, 4);

  // define button for the select message
  m2.setPin(M2_KEY_SELECT, 39);

  // The incremental rotary encoder is conected to these two pins
  m2.setPin(M2_KEY_ROT_ENC_A, 40);
  m2.setPin(M2_KEY_ROT_ENC_B, 41);
}

void loop() {
  m2.checkKey();
  m2.checkKey();
  if ( m2.handleKey() )
    m2.draw();
  m2.checkKey();

}


void fn_ok1(m2_el_fnarg_p fnarg) {
  //m2.setRoot(&el_list3);
  m2.setRoot(&top_el_expandable_menu);
}
/*
void fn_ok4(m2_el_fnarg_p fnarg) {
  m2.setRoot(&el_list2);
}
*/
void fn_ok3(m2_el_fnarg_p fnarg) {
  m2.setRoot(&top_el_expandable_menu);
}

void fn_ok(m2_el_fnarg_p fnarg) {
  /* do something with the number */
}
void ch_x(m2_el_fnarg_p fnarg) {
  /* do something with the number */
}
/*
void scr1_home (m2_el_fnarg_p fnarg) {
   XX.sendsyscom("$X");   // reset Grbl
   XX.sendsyscom("$H");   // Home Grbl}
  Serial.println( " homing");
}
*/
void fn_opstarten(m2_el_fnarg_p fnarg) {

  m2.setRoot(&top_el_expandable_menu);
}
void fn_jog_xp(m2_el_fnarg_p fnarg) {
  x_pos++;

}
void fn_jog_xm(m2_el_fnarg_p fnarg) {

  x_pos--;
}
void fn_jog_yp(m2_el_fnarg_p fnarg) {
  y_pos++;

}
void fn_jog_ym(m2_el_fnarg_p fnarg) {

  y_pos--;
}
void fn_jog_zp(m2_el_fnarg_p fnarg) {
  z_pos++;

}
void fn_jog_zm(m2_el_fnarg_p fnarg) {

  z_pos--;
}


void fn_mill_s(m2_el_fnarg_p fnarg) {
  lcd.clear();
  lcd.setCursor(0, 3);
  lcd.print( "exchanging......");
  delay(5000);
  m2.setRoot(&top_el_expandable_menu);
}
void fn_mill_r(m2_el_fnarg_p fnarg) {


  m2.setRoot(&top_el_expandable_menu);
}
void fn_cartridge(m2_el_fnarg_p fnarg) {
  m2.setRoot(&top_el_expandable_menu);
}

void fn_homing_start(m2_el_fnarg_p fnarg) {

  lcd.clear();
  lcd.setCursor(0, 3);
  lcd.print( "homing......");
  XX.sendsyscom("$X");   // reset Grbl
  XX.sendsyscom("$H");   // Home Grbl}
  //while (digitalRead(buttonPin) ==HIGH) {delay (100);Serial.println(digitalRead(buttonPin));};
  m2.setRoot(&top_el_expandable_menu);
}
void fn_run(m2_el_fnarg_p fnarg) {
  byte returndir = 1;
  boolean Stop = false;
  float CARTCO [3]; // track x,y,z cartridge position
  char temp[9] = "     ";
  m2.setRoot(&el_list_pause);

    for (byte index_y = 0; index_y < INDEX[1];) {
    CARTCO[1] = STARTCO[1] + index_y * PITCH[1]; index_y++;
    // meander
    for (byte index_x = 0; index_x < INDEX[0] + 1;) {
      switch (returndir) {
        case 1: {
            CARTCO[0] = STARTCO[0] + index_x * PITCH[0];
            if (index_x == INDEX[0]) {
              returndir = 2;
            };
            break;
          }
        case 2: {
            CARTCO[0] = STARTCO[0] + (INDEX[0] - index_x) * PITCH[0];
            if (index_x == INDEX[0]) {
              returndir = 1;
            };
            break;
          }
      }
      index_x++;
      // calculate next Cartridge coordinates
      XX.sendgcode("F20000");
      XX.sendgcode(" G90 "); // absolute distance mode
      XX.sendgcode("G1Z-0.50");  // spec van FONS nodig
      dtostrf(CARTCO[0], 1, 2, &temp[0]); // two (2) decimals behind dot
      String  Gbrl = "G1 X"; Gbrl += temp; Gbrl += "Y ";
      dtostrf(CARTCO[1], 1, 2, &temp[0]); Gbrl += temp; Gbrl += " ";
      if (digitalRead(buttonPin) == LOW) {
        XX.sendsyscom("!");//stop Frees
        Stop = true;
        Serial.println("stopped");
      }
      while (Stop == true) {
        if (digitalRead(buttonPin) == HIGH) {
          Stop = false; XX.sendsyscom("~");
        }
      }
      XX.sendgcode(Gbrl);
      XX.sendgcode ("G91");  // incremental distance mode
      Mill(gcode_buffer_first_set);
    }
    
  }

  int i = 0;
  lcd.setCursor(0, 3); lcd.print("              ");
  while (digitalRead(buttonPin) &  digitalRead(10) ) {
    delay (300);
    lcd.setCursor(0, 3);
    lcd.print(i);
    i++;
  };

  
}

int Mill( char buffers[][30]) {
  for (int i = 0; i < 30; i++) {
    if (buffers[i][0] != NULL) {
      XX.sendgcode(buffers[i]);
    }
  }
  return (1);
}

