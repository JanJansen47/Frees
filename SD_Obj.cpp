#include "SD.h"
#include "Arduino.h"
#include "SD_Obj.h" 

/*
 Grbl Controller for the Grbl on a arduino uno.
 This class handles the SD disk I/O. 
 Auth: J. Jansen
 Date: 17-02-2015
 Version: 0.1
 
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
   pinMode(53, OUTPUT);  // mega specific
  const int chipSelect = 53; // mega specific
  if (!SD.begin(chipSelect)) {
   Serial.println("initialization failed xx!");
    
    return(10);
  }
  Serial.println("initialization SD done.");
  filelist [8]  = String(8);
   
return(11);
} 



/* *************************************************
Open file from SD
****************************************************
*/

int SD_Obj::open_File(String Filename) {
char gcodechar;
int i =0;
// conversion from String to Char[]
char filename[Filename.length()+1];
Filename.toCharArray(filename, sizeof(filename));
//Open the file
gcodeFile = SD.open(filename);
if (gcodeFile) { 
//put char in gcode buffer 
while (gcodeFile.available()) {
gcodechar = gcodeFile.read();
if (gcodechar == '\n') {  // check eol
i++;}  // if eol >>  add one line

}//eo while
gcodeFile.close();
gcodeFile = SD.open(filename);  // reset file to start
return(i); // return the number of lines in the file
 }
return (-101); // file does not exist
}
/* *************************************************
Read line from open file
****************************************************
*/
String SD_Obj::read_line_from_open_File() {
gcl = "";  // empty String
char gcodechar;
//Serial.println("voor if");
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
/* *************************************************
Close file from SD
****************************************************
*/
int SD_Obj::close_File() {  // no file name required (only one file open at a time)
//close the file
gcodeFile.close();
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
Serial.println("------------File_List-------------------"); 
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
      Serial.println("----------------------------------------");}
}// eo while morefiles
return (i-1); // number of files on the SD
}
