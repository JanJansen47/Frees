#include "Arduino.h"
#include "SD.h"
/*
 Grbl Controller for the Grbl on a arduino uno.gcl
 This claas handles the SD disk I/O. 
 Auth: J. Jansen
 Date: 17-02-2015
 Version: 0.1
 
 */
#ifndef SD_Obj_h
#define SD_Obj_h

class SD_Obj
{
public:
SD_Obj();  //constructor
int init_SD();  // initialize the SD disk
int read_File(String Filename); // reads one gcode
int open_File(String Filename);
String read_line_from_open_File();
int close_File();
int list_Files();
String nextgcode(int next); 
String gclb [10];  // gcode buffer (sores the gcl's.
String gcl;         //gcl = line with gcode 
int lptr;  	   //lptr -1 holds the number of gcodes in the file
String filelist [8];  // tabel with the files on the SD card
File gcodeFile;
private:
int printDirectory(File dir, int numTabs);
};
#endif
