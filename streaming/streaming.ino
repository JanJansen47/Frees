#include <SPI.h>
#include <SD.h>
#include <Serial_Obj.h>
#define RXBUF 100
#include <TextFinder.h>
#define MAX 30
String gcode [MAX];
Serial_Obj    XX = Serial_Obj(22);  // instantiate object to use communication with Grbl Uno
int sizetotal, size, size_i; 
byte frontptr, endptr; 
TextFinder  vinder(Serial3, 10);
byte length_gcodes [20];
void setup() {
  Serial.begin(115200);
  Serial3.begin(115200);
  gcode[0] = "F1000";
  gcode[1] = "G91";
  gcode[2] = "G1X1.00Y0.1Z0.00";
  gcode[3] = "G1X-1.00Y-0.1Z0.00";
  gcode[4] = "G1X1.00Y0.1Z1.00";
  gcode[5] = "G1X-1.00Y-0.1Z-1.00";
  gcode[7] = "G90";
  gcode[8] = "G1X50.0Y50.0Z-2.0";
  XX.sendsyscom("\r\n\r\n");
  delay(200);
XX.sendsyscom("$X");
//XX.sendsyscom("$H");

delay(200);
XX.flush();
frontptr = endptr =0;
sizetotal = 0;
}

void loop() {
  
  for (int i = 0; i < 7; ) {
    if(frontptr==MAX){frontptr=0;} // reset front ptr to begin of lenght array.
    length_gcodes[frontptr] = gcode[i].length();
    sizetotal += length_gcodes[frontptr];
    Serial.print(gcode[i]); 
    Serial.print(" total:     ");Serial.println(sizetotal); 
    Serial.print(" frontptr:  ");Serial.println(frontptr); 
    Serial.print(" endptr:    ");Serial.println(endptr); 
    Serial.print(" front size:");Serial.println(length_gcodes[frontptr]); 
    Serial.print(" end size:  ");Serial.println(length_gcodes[endptr]); 
    frontptr ++;
  while ( sizetotal > RXBUF) {
    while (Serial3.available() == 0) {
    };
    vinder.find("ok");  // check "ok" for each counted gcode
//    sizetotal -= length_gcodes[endptr];
    endptr ++; 
    if(endptr == MAX) endptr=0;
    
  } //eo while
  Serial3.println(gcode[i]);;
  //delay(500);
  i++;
  } // eo for lines
}
