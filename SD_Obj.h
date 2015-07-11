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
int getdat(float pdata[],char l,String pcl);  // rel1 used to implement "subroutines" reads the data from SD
int getgcode(char l, String pcl);  // rel 1 used to find a line with gcode
int close_File();
int list_Files();
int fillCartridgeData(String cartridgeFile, float STARTCO[2], float PITCH[2], int INDEX[2], int *GCODENUM , char gcode_buffer_first_set[][30], char gcode_buffer_second_set [][30]);
String nextgcode(int next); 
String gclb [1];  // gcode buffer (sores the gcl's.
String gcl;         //gcl = line with gcode 
int lptr;  	   //lptr -1 holds the number of gcodes in the file
String filelist [12];  // tabel with the files on the SD card
File gcodeFile;
private:
int printDirectory(File dir, int numTabs);
};
#endif
