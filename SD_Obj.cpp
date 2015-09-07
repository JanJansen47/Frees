#include "SD.h"
#include "Arduino.h"
#include "SD_Obj.h" 
//#include "/home/jan/Arduino/testen/basic_test_gcode/config_frees.h"
#include "/home/jan/Arduino/Vrees/MegaFinal/config_frees.h"  // let op afhankelijk van de plaats waar de config_frees.h staat
/*
 Grbl Controller for the Grbl on a arduino uno.
 This class handles the SD disk I/O. 
 Auth: J. Jansen
 Date: 11-07-2015
 Version: 0.3
 
 */
File gcodeFile;
File root;  // list directories 
String gclb [2];  // gcode buffer (sores the gcl's.
String gcl = String(20);     //gcl = line with gcode (STring constructor)
String filelist [12]; //  = String(8);  // tabel with the files on the SD card
int lptr =0;        // counts gcode lines
SD_Obj::SD_Obj()
{ ; };


/* *************************************************
Initialize SD
****************************************************
*/
int SD_Obj::init_SD()
{
 Serial.println("SD initialization start");
#ifdef MEGAmarc
  pinMode(53, OUTPUT);  // mega specific
 const int chipSelect = 53; // mega specific
#endif
#ifdef MEGAjan
  pinMode(53, OUTPUT);  // mega specific
 const int chipSelect = 53; // mega specific
#endif
#ifdef Teensy
 pinMode(2, OUTPUT);  // Teensy specific
 const int chipSelect = 2; // Teensy specific
#endif
  if (!SD.begin(chipSelect)) {
   Serial.println("SD initialization failed xx!");
    
    return(10);
  }
  Serial.println("initialization SD done.");
  //filelist [8]  = String(8);  // 22-8-2015 removed geen effect merkbaar
   
return(11);
} 



/* *************************************************
Open file from SD
****************************************************
*/

int SD_Obj::open_File(String Filename) {
Serial.println("open file from SD");
char gcodechar;
int i =0;
// conversion from String to Char[]
char filename[Filename.length()+1];
Filename.toCharArray(filename, sizeof(filename));
//Open the file
gcodeFile = SD.open(filename);
Serial.println(" Gcode file open*");
if (gcodeFile) { 
//put char in gcode buffer 
while (gcodeFile.available()) {
gcodechar = gcodeFile.read();
if (gcodechar == '\n') {  // check eol
i++;}  // if eol >>  add one line

}//eo while
//gcodeFile.close();
close_File();
gcodeFile = SD.open(filename);  // reset file to start
Serial.println(" Gcode file open**");
return(i); // return the number of lines in the file
 }
// gcode file empty or does not exist?
//gcodeFile.close();
close_File();
return (-101); // file does not exist
}
/* *************************************************
Read line from open file
****************************************************
*/
String SD_Obj::read_line_from_open_File() {
gcl = "";  // empty String
char gcodechar;
Serial.println("..Readline");
if (gcodeFile) { 
//put char in gcode buffer 
//Serial.println("in if");
while (gcodeFile.available() ) {
//Serial.println("in while");
gcodechar = gcodeFile.read();
gcl += gcodechar;
if (gcodechar == '\n') {  // check eol
//Serial.println(" in read ONE line from SD");
return (gcl);
	   } // if eol check \n
  	 }  //eo while available
	} // eo if gcodefile 
      }  // eo read_line_from_open_File

/************************************************************** 
fillcartridge data

 **************************************************************
  */
int SD_Obj::fillCartridgeData(String cartridgeFile, float STARTCO[2], float PITCH[2], int INDEX[2], int info[2] , char gcode_buffer_first_set[][30], char gcode_buffer_second_set [][30])
{
String pcl;
int no_of_lines = open_File(cartridgeFile);    // opening file for the first time to read STARTCO, PITCH INDEX etc
int index1, index2,index3, test;
char instr;
char buf[30];
// read and store gcode file parameters and gcode line index's
// check lines in cartridge file for O,H,B,M,#,* and &
for( int i=0; i< no_of_lines;i++) {
pcl = read_line_from_open_File();
float INDEX_S[2], GCODENUM_S[2];
getdat (STARTCO,    'O', pcl);
getdat (PITCH,      'H', pcl);
getdat (INDEX_S,    'B', pcl);
getdat (GCODENUM_S, 'M', pcl);
if (getgcode ('#', pcl) > 0) {index1 = i+1;}  // gcode starts at the next line (+1)
if (getgcode ('*', pcl) > 0) {index2 = i+1;}  // gcode starts at the next line (+1)
if (getgcode ('&', pcl) > 0) {index3 = i;}
INDEX[0] = (int)INDEX_S[0];INDEX[1] = (int)INDEX_S[1];  // index of the two gcode sets
info[0] = (int)GCODENUM_S[0];
//Serial.print(" gcodenumf:  "); Serial.println(GCODENUM_S[0]);Serial.print(" gcodenumi:  ");Serial.println(info[0]);

}
close_File();  //  close file opened for the first time
// read and store gcode files
open_File(cartridgeFile);   // open file for the second time to read the gcodes
//skip lines before first gcode line
for (int i =0; i< index1;i++) { read_line_from_open_File();} // skip lines to start at first set of gcode lines

//first gcode set
//Serial.println("first set");
for (int j =index1; j < index2-1; j++) { 
pcl =read_line_from_open_File();  
int k=0;
instr = pcl.charAt(0);
while (instr != NULL & instr != 10) { 
gcode_buffer_first_set[j-index1][k] = instr; // copy char in gcode buffer
k++;
instr =pcl.charAt(k);//Serial.print(instr)
 }

};

//second gcode set
if (info[0]=2){   // is there a second gcode set?
read_line_from_open_File(); //skip line between gcode set 1 and 2
//Serial.println("sec set");
for (int j =index2; j < index3; j++) {
pcl = read_line_from_open_File();
int k=0;
instr = pcl.charAt(0);
while (instr != NULL & instr != 10) {
gcode_buffer_second_set[j-index2][k] = instr; // copy char in gcode buffer
k++;
instr =pcl.charAt(k);//Serial.print(instr); 
}; // eo fill buffer
} // gcode lines
}// eo is there a second gcode set
close_File();  //close file for the reading of the gcodes
 }




/************************************************************** 
getdata 
 Reads String pcl and tries to find char 'l'. If l of the ',' 
 separating data is not found the procedure return -1. 
**************************************************************
  */
int SD_Obj::getdat(float pdata[], char l,  String pcl)
{ 
  int ind1;
  int ind2;
  ind1 = pcl.indexOf(l);
  if (ind1<0) {
    return (ind1);
  };  // char not present => error
  ind2 = pcl.indexOf(',');
  if (ind2<0) {
    return (ind2);
  };  // char not present => error

// Rasperry does not support the arduino IDE 1.5 or higher and thus not support .toFloat() 

char tempbuf[10];  // temporary solution (Raspberry pi)
pcl.substring(ind1+1 , ind2).toCharArray(tempbuf,10); // temporary solution  (Raspberry pi)
pdata[0]= atof(tempbuf); // temporary solution (Raspberry pi)
//pdata[0] =  pcl.substring(ind1+1 , ind2).toFloat(); 
ind1 = pcl.indexOf(',', ind2+1);
pcl.substring(ind2+1, ind1).toCharArray(tempbuf,10); // temporary solution  (Raspberry pi)
pdata[1]= atof(tempbuf); // temporary solution (Raspberry pi)
  //pdata[1] = pcl.substring(ind2+1, ind1).toFloat();
}

/************************************************************** 
getgcode
 Reads String pcl and tries to find char 'x'. If return is > 0 the gcode is in the next line
**************************************************************
  */
int SD_Obj::getgcode(char l, String pcl)
{
  int ind1;
  ind1 = pcl.indexOf(l);
  return (ind1);
}

/* *************************************************
Close file from SD
****************************************************
*/
int SD_Obj::close_File() {  // no file name required (only one file open at a time)
//close the file
gcodeFile.close();
//Serial.println("gcodefile closed");  // debugging
}

/* *************************************************
Read file from SD
****************************************************
*/

int SD_Obj::read_File(String Filename) {
char gcodechar;
int i;
lptr=0;
// conversion from String to Char[]
char filename[Filename.length()+1];
Filename.toCharArray(filename, sizeof(filename));
//Open the file
gcodeFile = SD.open(filename);
if (gcodeFile) { 
//put char in gcode buffer 
while (gcodeFile.available()) {
gcodechar = gcodeFile.read();
gcl += gcodechar;
if (gcodechar == '\n') {  // check eol
gclb[lptr] = gcl;
gcl = "";
lptr++;}  // if eol insert '/0' (c convention)
 }
//close the file
gcodeFile.close();
// print gcodes in buffer
//for ( i=0; i <(lptr-1);) { Serial.print("--" );Serial.print(i);Serial.print(">> " );Serial.println(gclb[
//i]);i++;};
return(10);
}
return(-102);
 }

//read gcode line

String SD_Obj::nextgcode(int next)  {
if ( next < lptr-1) {Serial.println(gclb[next]); return gclb[next];} 
if ( next== lptr-1) return ("Ready");
}

/* *************************************************
List files on SD
****************************************************
*/
int SD_Obj::list_Files() {  // no file name required 
Serial.println("-------List_Files-- -----");
root = SD.open("/");
int result = printDirectory(root, 0); // return the number of files in the filelist buffer (pointer)
root.close();
return(result);
}

/* *************************************************
Print Directories on SD
****************************************************
*/
int SD_Obj::printDirectory(File dir, int numTabs) {
bool morefiles = true;  
int i = 0; // count the files on the SD
dir.rewindDirectory();  // 22-08-2015 to be sure we have the full directory in scope and not only the next file
Serial.println("------------File_List---Print Directory -----"); 
while(morefiles) {
     File entry =  dir.openNextFile();
     if (entry) {
       if (entry.isDirectory()) {
      Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
      Serial.println(entry.name());
       filelist[i] = entry.name();  // Store file entries in tabel
       i++;
     } // eo else
   } //eo if entry
      else {morefiles=false ;
 //     Serial.println("----------------------------------------")
;}
}// eo while morefiles
return (i-1); // number of files on the SD
}
