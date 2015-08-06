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
const int buttonPin = 39;  // for test only
String cartridgeFile = "frame1.g";
char *cartfile = "        " ;
int error = 0 ; // error code
volatile int Stop = HIGH;

int lineCounter;  // the number of lines in a gcode file
// iformation of the selected cartride gcode and index information
char gcode_buffer_first_set[95][30];  //first set of gcodes
char gcode_buffer_second_set[10][30];  // second set of gcodes
float STARTCO[2];  // x and y start coordinates
float PITCH[2];  // deltas in x and y between cartridges
int INDEX[2];     // number of cartridges in x and y
int INFO[2];  // INFO[0] =1 one set of gcode 2 two sets of gcodes
// gcodes = 2 => gcodes for the lid and cartridge inner
#define FS_EXTRA_MENUES 1 // this is the first line in the cartridge menu
// Forward declarations
M2_EXTERN_VLIST(el_list3);
M2_EXTERN_VLIST(opstart);
M2_EXTERN_VLIST(list_sc2);
M2_EXTERN_VLIST(top_el_menu2);
M2_EXTERN_VLIST(top_el_menu4);
M2_EXTERN_VLIST(el_list_home);
M2_EXTERN_VLIST(el_list_mill);
M2_EXTERN_VLIST(el_list_mill_s);
M2_EXTERN_VLIST(el_list_cartridge);
M2_EXTERN_VLIST(el_list_service);
M2_EXTERN_VLIST(el_list_run);
M2_EXTERN_VLIST(el_list_pause);
M2_EXTERN_ALIGN(top_el_start);

M2tk m2(&opstart, m2_es_arduino_rotary_encoder, m2_eh_4bd, m2_gh_lc);

//*********************  menu_1  **********************************
M2_LABEL(opstart_1, "x1y3", "CARTRIDGE MILL");
M2_LABEL(opstart_2, "x1y2", "Version: ");
M2_LABEL(opstart_3, "x10y2" , "15");
M2_ROOT(opstart_ok, "x1y0", "OPSTARTEN", &top_el_menu2);
M2_LIST (list_opstart) = {&opstart_1, &opstart_2, &opstart_3, &opstart_ok};
M2_XYLIST(opstart, NULL, list_opstart);

// ********************  menu_2   *********************************
m2_menu_entry m2_2lmenu2[] =
{
  { "KIES CARTRIDGE", &el_list_cartridge},
  { "HOME", &el_list_home },
  { "SPECIAL", &top_el_menu2 },
  { ". HOME", &el_list_home },
  { ". JOG", &el_list3},
  { ". WISSEL FREES", &el_list_mill },
  { ". SERVICE", &el_list_service },
  { NULL, NULL },
};
// menu2 : The first visible line and the total number of visible lines.
uint8_t m2_2lmenu2_first;
uint8_t m2_2lmenu2_cnt;
// menu2  definition
M2_2LMENU(el_2lmenu2, "l4e1w16", &m2_2lmenu2_first, &m2_2lmenu2_cnt, m2_2lmenu2, '+', '-', '\0');
M2_VSB(el_vsb2, "l4w1r1", &m2_2lmenu2_first, &m2_2lmenu2_cnt);
M2_LIST(list_2lmenu2) = { &el_2lmenu2, &el_vsb2 };
M2_HLIST(top_el_menu2, NULL, list_2lmenu2);

// **********************   menu3: Select a cartridge file  ***********************
uint8_t fs_m2tk_first = 0;
uint8_t fs_m2tk_cnt = 0;
uint8_t index_filelist;

M2_STRLIST(el_fs_strlist, "l4e1w12", &fs_m2tk_first, &fs_m2tk_cnt, fs_strlist_getstr);
M2_VSB(el_fs_strlist_vsb, "l4w1r1", &fs_m2tk_first, &fs_m2tk_cnt);
M2_LIST(list_fs_strlist) = { &el_fs_strlist, &el_fs_strlist_vsb };
M2_HLIST(el_top_fs, NULL, list_fs_strlist);

/* show selected file */
M2_LABEL(el_show_file_label, NULL, "Selected file:");
M2_LABELFN(el_show_filename, NULL, fs_show_file_label_cb);
M2_ROOT(el_show_file_ok, NULL, "ok", &top_el_menu4);
M2_LIST(list_show_file) = { &el_show_file_label, &el_show_filename, &el_show_file_ok };
M2_VLIST(el_show_file_Vlist, NULL, list_show_file);
M2_ALIGN(top_el_show_file, "-1|1W64H64", &el_show_file_Vlist);

// *****************  Select cartridge  *******************************
M2_ROOT(cartridge_root, "x1y0", "Kies", &el_top_fs);
M2_LIST (list_cartridge) = { &cartridge_root};
M2_XYLIST(el_list_cartridge, NULL, list_cartridge);

// ******************  menu_4  ***************************************
m2_menu_entry m2_2lmenu4[] =
{
  { "START FREZEN 1 ", &el_list_run},
  { "KIES CARTRIDGE", &el_list_cartridge},
  { "SPECIAL", &top_el_menu2 },
  { ". HOME", &el_list_home },
  { ". JOG", &el_list3},
  { ". WISSEL FREES", &el_list_mill },
  { ". SERVICE", &el_list_service },
  { NULL, NULL },
};
uint8_t m2_2lmenu4_first;
uint8_t m2_2lmenu4_cnt;

// menu4  definition
M2_2LMENU(el_2lmenu4, "l4e1w16", &m2_2lmenu4_first, &m2_2lmenu4_cnt, m2_2lmenu4, '+', '-', '\0');
M2_VSB(el_vsb4, "l4w1r1", &m2_2lmenu4_first, &m2_2lmenu4_cnt);
M2_LIST(list_2lmenu4) = { &el_2lmenu4, &el_vsb4 };
M2_HLIST(top_el_menu4, NULL, list_2lmenu4);

// *****************  Start (from begin) *********************************
M2_LABELFN(run_label1, "x12y0", fs_show_cartridge_name);
M2_LABEL(run_label2,  "x0y0", "Loopt");
M2_BUTTON(run_ok, "x1y1", "START", fn_run);
M2_LIST (list_run) = {&run_label1, &run_ok};
M2_XYLIST(el_list_run, NULL, list_run);

// Pause
M2_LABEL(pause_label, "x1y3", "Next");
M2_BUTTON(pause_ok, "x1y1", "Restart", fn_run);
M2_BUTTON(stop_ok, "x1y2", "Stop", fn_ok3);
M2_LIST (list_pause) = {&pause_label, &pause_ok, &stop_ok};
M2_XYLIST(el_list_pause, NULL, list_pause);

//Home
M2_BUTTON(home_start, "x1y0", "start", fn_homing_start);
M2_LIST (list_home) = { &home_start};
M2_XYLIST(el_list_home, NULL, list_home);

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

//****************  Exchange mill ************************************
M2_LABEL(mill_label_s, "x1y3", "WISSEL FREES");
M2_BUTTON(mill_s, "x1y0", "WISSELEN", fn_mill_s);
M2_LIST (list_mill_s) = {&mill_label_s, &mill_s};
M2_XYLIST(el_list_mill, NULL, list_mill_s);

//**************  Service  ******************************************
M2_LABEL(service_label_s, "x1y3", "SERVICE");
M2_BUTTON(service_s, "x1y0", "SENDER", fn_service_s);
M2_LIST (list_service_s) = {&service_label_s, &service_s};
M2_XYLIST(el_list_service, NULL, list_service_s);


//=================================================
// Arduino Setup & Loop

void setup() {
  Serial.begin(115200);
  // Serial.println(" start");
  delay(100);
  XX.flush();
  YY.init_SD();

  pinMode(14, OUTPUT); // test only
  pinMode (18, INPUT_PULLUP); // attached to interrupt 5
  attachInterrupt(5, signalint, LOW);
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
  m2.setRoot(&top_el_menu2);
}
/*
void fn_ok4(m2_el_fnarg_p fnarg) {
  m2.setRoot(&el_list2);
}
*/
void fn_ok3(m2_el_fnarg_p fnarg) {
  m2.setRoot(&top_el_menu2);
}

void fn_ok(m2_el_fnarg_p fnarg) {
  /* do something with the number */
}
void ch_x(m2_el_fnarg_p fnarg) {
  /* do something with the number */
}

void fn_opstarten(m2_el_fnarg_p fnarg) {

  m2.setRoot(&top_el_menu2);
}
void fn_jog_xp(m2_el_fnarg_p fnarg) {
  x_pos++;
  XX.sendgcode("G91");
  XX.sendgcode("G1X1.0");

}
void fn_jog_xm(m2_el_fnarg_p fnarg) {

  x_pos--;
  XX.sendgcode("G91");
  XX.sendgcode("G1X-1.0");
}
void fn_jog_yp(m2_el_fnarg_p fnarg) {
  y_pos++;
  XX.sendgcode("G91");
  XX.sendgcode("G1Y1.0");
}
void fn_jog_ym(m2_el_fnarg_p fnarg) {
  y_pos--;
  XX.sendgcode("G91");
  XX.sendgcode("G1Y-1.0");
}
void fn_jog_zp(m2_el_fnarg_p fnarg) {
  z_pos++;
  XX.sendgcode("G91");
  XX.sendgcode("G1Z1.0");
}
void fn_jog_zm(m2_el_fnarg_p fnarg) {
  z_pos--;
  XX.sendgcode("G91");
  XX.sendgcode("G1Z-1.0");
}


void fn_mill_s(m2_el_fnarg_p fnarg) {   // exchange a mill

  XX.sendgcode("F20000");
  error = XX.sendgcode("G54"); // work with G54 origin
  XX.sendgcode("G1Z-0.50");
  XX.sendgcode("G01 X150.0 Y200.0");
  lcd.setCursor(0, 3);
  lcd.print( "NU WISSELEN");
  m2.setRoot(&top_el_menu2);
}
void fn_mill_r(m2_el_fnarg_p fnarg) {


  m2.setRoot(&top_el_menu2);
}

//*****************************GCODESENDER *******************************************
void fn_service_s(m2_el_fnarg_p fnarg) {   // Service mode

  lcd.clear();
  lcd.setCursor(0, 3);
  lcd.print( "GCODESENDER");
  Serial3.write('CTRL-X'); // Reset Grbl
  Serial3.write('CTRL-X'); // Reset Grbl
  // read from port 3, send to port 0:
  while (1 == 1) {
    if (Serial3.available()) {
      int inByte = Serial3.read();
      Serial.write(inByte);
    }

    // read from port 0, send to port 3:
    if (Serial.available()) {
      int inByte = Serial.read();
      Serial3.write(inByte);
    }
  } // eo while
}


void fn_cartridge(m2_el_fnarg_p fnarg) {
  m2.setRoot(&el_top_fs);
}

void fn_homing_start(m2_el_fnarg_p fnarg) {
  error = home();
  m2.setRoot(&top_el_menu2);
}

// **************** Main Milling program  *********************************************
void fn_run(m2_el_fnarg_p fnarg) {
  // load cartidge file in memory.
  error = 0; // reset error
  load_cartridge_file_memory(cartridgeFile);
  errorhandling (error);
  error = home();              // home first
  error = XX.sendgcode("G10L20P1X0Y0Z0"); // Set G54 to homing origin
  error = XX.sendgcode("G54"); // work with G54 origin
  error = XX.sendgcode("M3");   // Spindle enable
  error = XX.sendgcode("S250");   // Spindle at 0 rev/min  // foutje!!
  byte returndir = 1;

  float CARTCO [3]; // temp x,y,z cartridge position
  char temp[9] = "     ";
  m2.setRoot(&el_list_pause);   // trick menu

  lcd.setCursor(0, 3);
  lcd.print( "Loopt ");
  lcd.setCursor(12, 3);
  lcd.print( cartridgeFile);
  lcd.setCursor(1, 2);
  lcd.print( "[PAUZE]");
  errorhandling (error);

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
      dtostrf(CARTCO[0], 1, 2, &temp[0]); // two (2) decimals behind dot
      String  Gbrl = "G1X"; Gbrl += temp; Gbrl += "Y";
      dtostrf(CARTCO[1], 1, 2, &temp[0]); Gbrl += temp; Gbrl += "Z0.0";
     
// print index x and y
      lcd.setCursor(0, 0);
      lcd.print("index:  ");
      lcd.setCursor(7, 0);
      lcd.print( index_x);
      lcd.setCursor(10, 0);
      lcd.print( index_y);
      lcd.setCursor(16, 0);
      lcd.print( Stop);
      if (Stop == LOW) {
        lcd.setCursor(0, 3);
        lcd.print( "Gepauzeerd");
        lcd.setCursor(1, 2);
        lcd.print( "       ");
        // XX.sendsyscom("!");//stop Mill
        delay(5000);
        //  while (Stop == LOW) {
        //      }
        lcd.setCursor(0, 3);
        lcd.print( "Loopt     ");
        lcd.setCursor(1, 2);
        lcd.print( "[PAUZE]");
        //  XX.sendsyscom("~");  //Resume Mill
        
      }
Stop = HIGH;

      error = XX.sendgcode("F20000");    // spec van Fons nodig
      error = XX.sendgcode("G54");
      error = XX.sendgcode("G1 Z-0.50");
      error = XX.sendgcode(Gbrl);   // index position
      error = XX.sendgcode("G10L20P2X0.0Y0.0Z0.0"); // set coordinate system G55
      error = XX.sendgcode ("G55");  // switch to coordinate system G55
      error = XX.sendgcode("G1X0.0Y0.0 Z-0.50");  // spec van FONS nodig
      error = Mill(gcode_buffer_first_set);  // milling a cartridge
      if (error != 0) errorhandling(error);

    } // eo index_x

  } // eo index_y
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
  for (int i = 0; i < 95; i++) {
    int error = 0;
    if (buffers[i][0] != NULL) {
      error = XX.sendgcode(buffers[i]);
      Serial.print(" naar sendc:  "); Serial.println(buffers[i]);
    }
  }

  return (error);
}

//******************** Cartridge selection  ***********************************

const char *fs_show_file_label_cb(m2_rom_void_p element) {
  // retrieve fielname from list
  String FileName = YY.filelist[index_filelist];
  static char arr[15] = "             ";
  for (int i = 0; i < 15; i++) {
    arr[i] = 0;
  }
  int k = 0;
  char instr = YY.filelist[index_filelist].charAt(0);
  while (instr != 0 & instr != 10) {
    arr[k] = instr; // copy char in buffer
    k++;
    instr = YY.filelist[index_filelist].charAt(k);
  }; // eo fill buffer
  return arr;
}

// callback procedure for the file selection dialog
const char *fs_strlist_getstr(uint8_t idx, uint8_t msg)  {
  Serial.print("idx:  "); Serial.println(idx);
  if (msg == M2_STRLIST_MSG_GET_STR)  {
    Serial.println("M2_STRLIST_MSG_GET_STR");
    static char arr[15] = "             ";
    for (int i = 0; i < 15; i++) {
      arr[i] = 0;
    }; // initialize ASCII '0'
    int k = 0;
    char instr = YY.filelist[idx - 1].charAt(0);
    while (instr != 0 & instr != 10) {
      arr[k] = instr; // copy char in buffer
      k++;
      instr = YY.filelist[idx - 1].charAt(k);
    }; // eo fill buffer
    return arr;
  }
  else if ( msg == M2_STRLIST_MSG_GET_EXTENDED_STR ) {
    Serial.println("M2_STRLIST_MSG_GET_EXTENDED_STR");
  } else if ( msg == M2_STRLIST_MSG_SELECT ) {
    Serial.println("M2_STRLIST_MSG_SELECT");
    // File has been selected. Here: Show the file to the user
    index_filelist = (idx - FS_EXTRA_MENUES); Serial.print("Menu:  "); Serial.println(index_filelist);
    cartridgeFile = YY.filelist[index_filelist];
    m2_SetRoot(&top_el_show_file);
  } else if ( msg == M2_STRLIST_MSG_NEW_DIALOG ) {
    Serial.println("M2_STRLIST_MSG_NEW_DIALOG");
    //(re-) calculate number of entries, limit no of entries to 250
    if ( YY.list_Files() < 250 - FS_EXTRA_MENUES ) {
      fs_m2tk_cnt = YY.list_Files() + FS_EXTRA_MENUES + 1;
      Serial.print(" aantal bestanden:  "); Serial.println(fs_m2tk_cnt);
    }
    else
      fs_m2tk_cnt = 250;
  }
  return NULL;
}

byte load_cartridge_file_memory (String file_name) {
  Serial.println("File laden .........");
  for (int i = 0; i < 30; i++) {
    for (int j = 0; j < 30, j++;) {
      gcode_buffer_first_set[i][j] = NULL; gcode_buffer_second_set[i][j] = NULL;
    };
  }

  YY.fillCartridgeData(cartridgeFile, STARTCO, PITCH, INDEX, INFO , gcode_buffer_first_set, gcode_buffer_second_set );
  /*
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
  Serial.print("gcodenum:  ");
  Serial.println(INFO[0]);
  Serial.println("main first set");
  */
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


}

const char *fs_show_cartridge_name(m2_rom_void_p element)

{
  static char arr[9] = "       ";
  for (int i = 0; i < 9; i++) {
    arr[i] = 0;
  }; // initialize ASCII '0'
  int k = 0;
  char instr = cartridgeFile.charAt(0);
  while (instr != 0 & instr != 10) {
    arr[k] = instr; // copy char in buffer
    k++;
    instr = cartridgeFile.charAt(k);
  }; // eo fill buffer
  return arr;
}

int home() {
  lcd.clear();
  lcd.setCursor(0, 3);
  lcd.print( "homing");
  XX.sendsyscom("$X");   // reset Grbl
  XX.sendsyscom("$H");   // Home Grbl}
  //XX.sendgcode("G92X0.0Y0.0Z0.0"); // Work coordinates to 0 20/07/2015
}

void errorhandling (int error) {
  // update display
  lcd.setCursor(7, 3);
  lcd.print( error);
}
void signalint() {
  Stop = LOW;
}

