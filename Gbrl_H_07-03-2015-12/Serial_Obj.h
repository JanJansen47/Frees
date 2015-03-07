#include "Arduino.h"
/*
 Grbl Controller for the Grbl on a arduino uno.
 This claas handles the communication with the Grbl Controller. 
 Auth: J. Jansen
 Date: 17-02-2015
 Version: 0.1
 
 */
#ifndef Serial_Obj_h
#define Serial_Obj_h



class Serial_Obj
{
public:
Serial_Obj( int A);  //constructor
int sendgcode(String gcode );
int sendsyscom(String syscom );
int receiveByte();
int flush();
private:

};
#endif
