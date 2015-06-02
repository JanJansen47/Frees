
#include <Serial_Obj.h>
#include <Menu3_Obj.h>
#include <SD_Obj.h>



#include "Arduino.h"
#include "SPI.h"
#include "SD.h"
#include "EEPROM.h"


/*
 Grbl Controller for the Grbl on a arduino uno. 
 Auth: J. Jansen
 Date: 14-03-2015
 */
int version; 
 



SD_Obj        YY = SD_Obj();         // instantiate object to use SD disk
Menu3_Obj     ZZ = Menu3_Obj(1);     // instantiate object to present menu and other information
Serial_Obj    XX = Serial_Obj(22);  // instantiate object to use communication with Grbl Uno
int i=0;
unsigned long currentMillis; // to measure the remaining work
unsigned long startMillis; 
unsigned long totalWorkMillis; 
int choice= 0;  // menu selection output
int language = 1; //English is default
int homing =0;    // Indicates whether homning was done 0 = not done
int lineCounter;  // the number of lines in a gcode file 
String gcode_buffer ="";
String cartridgeFile ="Jan_easy.gcode";
int num_of_files = 0; // number of files on SD disk
boolean cartridge_files_loaded = false;  // files are loaded once ( rarity arduino) 
int repeat = 1;  // Normal : rune once  Demo run 10 times

void setup()
{
  version = 13; 
  // temp input touch 
  pinMode(22, INPUT);           // set pin to input
  digitalWrite(22, HIGH);       // turn on pullup resistors
  pinMode(23, INPUT);           // set pin to input
  digitalWrite(23, HIGH);       // turn on pullup resistors
  pinMode(24, INPUT);           // set pin to input
  digitalWrite(24, HIGH);       // turn on pullup resistors
  pinMode(25, INPUT);           // set pin to input
  digitalWrite(25, HIGH);       // turn on pullup resistors

  Serial.begin(115200);
  Serial.println(" start controller -------"); 
  delay(100);
  XX.flush();
  YY.init_SD();
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
  
  ZZ.init_LCD("Deetman");
  ZZ.Welcome(1, version);
  delay(3000);


}

void loop()
{
  Serial.println(" Interface test start"); 

  // define menu 1 on display
  int button[] = { 
    1,2,3,4             } 
  ;  // define number of buttons with reference to text
  int size_of_button = sizeof(button)/2;  // onverklaarbaar telt kennelijk in bytes!!!
  ZZ.MakeMenu(1,language, button, size_of_button );
  ZZ.product(cartridgeFile);  

  while (chose() == 0) {
  }
  /* ************************************
   Menulevel 1  (Main Menu)
   **************************************
   */
  switch (choice) {
  case 1:  
    {  
      /* ************************************
       Menulevel 1-1 (Homing) 
       **************************************
       */
      XX.sendsyscom("$X");   // reset Grbl
      XX.sendsyscom("$H");   // Home Grbl
      homing =1; 
      break; 
    }

  case 2:  
    {   
      /* ************************************
       Menulevel 1-2 (Start Frees)
       **************************************
       */
      // define number of buttons with reference to text
      int button[] = {        
        5 ,6                            }       
      ;  // stop & pause
      int size_of_button = sizeof(button)/2;  
      ZZ.MakeMenu(1,language, button, size_of_button );
      ZZ.product(cartridgeFile); 
      if (homing==0) {  // Home if homing was not done!!!
        ZZ.position( "HOMING");
        XX.sendsyscom("$X");   // reset Grbl
        XX.sendsyscom("$H");   // Home Grbl}
        homing = 1; 
      }
      if (repeat==0) repeat =1;
           
      while (repeat >0 ) {
         ZZ.product(cartridgeFile);  
         repeat--; 
        lineCounter = YY.open_File(cartridgeFile);  
        
        for (int k = 0; k <lineCounter;) {
          if (k==0) {                       // begin measuring the cyclus time of a catridgefile
            startMillis = millis();
          }   
          gcode_buffer = YY.read_line_from_open_File(); 
          XX.sendgcode(gcode_buffer);
          XX.send_real_time_status_request("?");        // ask the position from grbl
        
          /* ************************************
           Menulevel 2-1 (Stop / resume  Frees)
           **************************************2
           */
          if (chose()== 1) {  // read input 
            // define number of buttons with reference to text
            int button[] = {       
              5,7                       };   // stop and resume
            int size_of_button = sizeof(button)/2;  // onverklaarbaar telt kennelijk in bytes!!!
            ZZ.MakeMenu(1,language, button, size_of_button );

            switch (choice) {  //ml 2-1
            case 1:  
              k=lineCounter;
              repeat = 0; //stop endurence test
              // Stop and return to menu "Main" 
              break; 
            case 2:  
                            {
                while (chose()== 0) {
                };
                
                delay(500);
                /* ************************************
                 Menulevel 3-3 (Stop resume /2
                 Frees)
                 **************************************
                 */
                int button[] = {
                  5,7                  };  // Stop menu
                // define number of buttons with reference to text
                int size_of_button = sizeof(button)/2;  // onverklaarbaar telt kennelijk in bytes!!!
                ZZ.MakeMenu(1,language, button, size_of_button ); 
                switch (choice) {  //ml 2-2
                case 1:  // stop
                  Serial.println(" in Stop");
                  k=lineCounter;
                  repeat = 0; // stop repeat (endurance)
                  // Stop and return to menu "Main" 
                  break; 
                case 2:    //resume
                  Serial.println(" in Resume");
                  break;  
                }//eo  switch ml 2-2
                break;
              }
              break;
            }  //eo  switch ml 2-1

          }  // eo read input

          // display the number of the gcode step
          String test = String(k, 10);
          ZZ.position( test);
          k++; // next gcode line
          // display remaining work (in seconds) 
          test = String ((totalWorkMillis- (millis()-startMillis))/1000 , 10); // display
          ZZ.information(test);
          if (k == lineCounter) { // measure worktime of a cartridgefile
              totalWorkMillis = millis() - startMillis;  
          } 
        }  // eo running gcode lines
        YY.close_File(); 
      }//eo while
      break;
    }  //eo case 2 (Start) 
  case 3:  
    {
      //All the cartridge files are loaded the first time and stored in the filelist. 
      // It appeared impossible to load multiple times. Reason is unknown (arduino rarity)
      if (!cartridge_files_loaded){    // cartridge files not loaded
      num_of_files = YY.list_Files();
      ZZ.show_select_File( YY.filelist,  num_of_files );
      cartridge_files_loaded =true;
      }
     if (cartridge_files_loaded){ZZ.show_select_File( YY.filelist,  num_of_files );} // cartridge files already loaded
      
      while (chose() == 0) {  // wait for input
      } 

      switch(choice) {
        case 1:
        cartridgeFile = YY.filelist[0];
        break;
      case 2: 
        cartridgeFile = YY.filelist[1];
        break;
      case 3:  
        cartridgeFile = YY.filelist[2];
        break; 
      case 4: 
        cartridgeFile = YY.filelist[3];
        break;
      case 5: 
        cartridgeFile = YY.filelist[4];
        break;
      case 6: 
        cartridgeFile = YY.filelist[5];
        break;
      case 7: 
        cartridgeFile = YY.filelist[6];
        break;
      }

      break;
    }  
  case 4:  
    {  //case 4

      /* ************************************
       Menulevel 1-4  (Select Language, USB passtrough and repeat)
       **************************************
       */
      // define number of buttons with reference to text
      int button[] = {       
        11,12,13                };   // language and USB Passtrough
      int size_of_button = sizeof(button)/2;  // onverklaarbaar telt kennelijk in bytes!!!
      ZZ.MakeMenu(1,language, button, size_of_button );
      {  
        while (chose() == 0) {  // wait for input
        } 


        switch (choice) {  
        case 1:  
          {  //AA

            /* ************************************
             Menulevel 4-1  (Select Language)
             **************************************
             */
            // define number of buttons with reference to text
            int button[] = { 
              8,9,10                               };
            int size_of_button = sizeof(button)/2;  // onverklaarbaar telt kennelijk in bytes!!!
            ZZ.MakeMenu(1,language, button, size_of_button );
            while (chose() == 0) {  
            };

            switch (choice) { //A0
            case 1:          //A1
              language = 3; 
              break;         //A1
            case 2:          //A2 
              language = 2; 
              break;         //A2
            case 3:          //A3
              language = 1; 
              break;         //A3
            }                //A0
            break; 
          }   //AA
        case 2:       //BB
          ZZ.information("gsender active");
          Serial3.write('CTRL-X'); // Reset Grbl
          // read from port 3, send to port 0:
          while (1==1) {
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
          break;     //BB
          case 3:
          repeat =1000;
          break;
        }
      }
    }  
  }  //eo case 4
}

// Temp instead of Touch
int  chose () {
  choice = 0; 
 // Serial.println( "in chose"); 
 // Serial.println(digitalRead(22));
 // Serial.println(digitalRead(23));
 // Serial.println(digitalRead(24));
  if (digitalRead(22) ==1 & digitalRead(23)==1 & digitalRead(24) ==1 & digitalRead(25) ==1) {
    return (0);
  }
  if (digitalRead(22) ==0) { 
    choice = 1;
  }
  if (digitalRead(23) ==0) { 
    choice = 2;
  }
  if (digitalRead(24) ==0) { 
    choice = 3;
  }
  if (digitalRead(25) ==0) { 
    choice = 4;
  }
 // Serial.print(" choice  :  ");
 // Serial.println(choice);
  //if (Serial.available() > 0) {
  // choice = Serial.read() -'0'; 
  delay(100);
  return (1);

}








