#include "Arduino.h"
#include "Serial_Obj.h" 
/*
 Grbl Controller for the Grbl on a arduino uno.
 This class handles the communication with the Grbl Controller. 
 Auth: J. Jansen
 Date: 14-03-2015
 Version: 0.2
 
 */


TextFinder  finder(Serial3, 2);
const int NUMBER_OF_FIELDS = 3; // 3 comma seperated fields  expected                    
float values[NUMBER_OF_FIELDS];   // array holding values for all the fields

Serial_Obj::Serial_Obj(int A)
{
int _A = A;
Serial3.begin(115200);
Serial.println ("Serial3 started");
 
}

/*
Send system commands to Grbl 
Results in System command output window

*/

int Serial_Obj::sendsyscom(String syscom )

{
  String Charstring = "SCommand_response:  ";
  Serial3.println(syscom);
   while (Serial3.available() ==0) {
  };
  while (Serial3.available() >0) {
    // read the incoming byte:
    delay(3);
    char incomingByte = Serial3.read();
    Charstring +=   incomingByte;
  }
  Serial.println(Charstring);

  return(10);
}
/********************************************
Send gcodes to Grbl  Results in Ok window
*********************************************
*/
int Serial_Obj::sendgcode(String gcode )
{
Serial3.println(gcode);
String Charstring = "Sgcode_response_current  :  ";
String CharstringS = "Sgcode_response_previous:  "; // previous status


  
  //Serial3.println(gcode);
  delay(3);
//Serial.println("na delay response Grbl");
while (Serial3.available() ==0) {
  };
//Serial.println("voor response Grbl");
// Check Grbl's response.. if no or an error response printline after 1 second

long Zeit = millis();
 finder.find("ok");   

// check errors
if((millis()-Zeit)>1000) {Serial.println("timeout response Grbl");
Serial.print("na zeit Grbl  : ");Serial.println(millis()-Zeit);

Serial3.println("?");// request status 
while (Serial3.available() ==0) {
  };
  while (Serial3.available() >0) {
    // read the incoming byte:
    delay(5);
     char incomingByte = Serial3.read();
    Charstring +=   incomingByte;
  }
  //Serial.println(CharstringS);
  Serial.println(Charstring);

}
/*
// store previous reading
 Serial3.println("?");// request status 
while (Serial3.available() ==0) {
  };
  while (Serial3.available() >0) {
    // read the incoming byte:
    delay(5);
     char incomingByte = Serial3.read();
    CharstringS +=   incomingByte;
  }
*/
delay(1);  // give Grbl some time before starting a new command
 

/*
  while (Serial3.available() ==0) {
  };
  while (Serial3.available() >0) {
    // read the incoming byte:
    delay(5);
     char incomingByte = Serial3.read();
    Charstring +=   incomingByte;
  }

  Serial.println(Charstring);
*/
  return(10);
}
/********************************************
receivebyte Grbl  Results in Ok window
*********************************************
*/
int Serial_Obj::receiveByte(){
  //Serial.begin(115200);
  while (Serial3.available() > 0) {
    // read the incoming byte:
    char incomingByte = Serial3.read();
    Serial.println(incomingByte);
  }
}
/********************************************
real time status  Grbl  Results in Ok window
*********************************************
*/
int Serial_Obj::send_real_time_status_request(String syscom )
{  Serial3.println(syscom);
while (Serial3.available() ==0) {
  };
  
  const int NUMBER_OF_FIELDS = 3; // 3 comma seperated fields expected                                    
  float values[NUMBER_OF_FIELDS];   // array holding values for all the fields
  //Serial.println("net voor de textfinder");
  int fieldIndex = 0;            // the current field being received
  while (Serial3.available() ==0) {
  };
  finder.find("MPos:");  
 //Serial.println("net na de textfinder");
 while(fieldIndex < NUMBER_OF_FIELDS){
   values[fieldIndex++] = finder.getFloat();  }    
 Serial.print("X  Y  Z :");
  for(fieldIndex=0; fieldIndex < NUMBER_OF_FIELDS; fieldIndex++)
   Serial.print(values[fieldIndex]);         
   Serial.println();


  return(10);
}

/*
flush the input buffer 
Results in information window

*/
int Serial_Obj::flush()
{
  Serial3.flush();
  Serial.println("flush done"); 
  return(10);
}

/* pass serial interface to Serial3 interface
This is necessary to have one USB connection for both the Grbl controller and Mega.
*/
void Serial_Obj::connectSerial_Serial3(){
   // read from port 3, send to port 0:
  if (Serial3.available()) {
    int inByte = Serial3.read();
    Serial.write(inByte); 
  }
  
  // read from port 0, send to port 3:
  if (Serial.available()) {
    int inByte = Serial.read();
    Serial3.write(inByte); 
  }
  }

