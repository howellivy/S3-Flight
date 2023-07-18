//*************************************************************************************
//  Quest operations with the installed FRAM 32K memory 
//  I2C address 0x50 and is 32K in size
//  Dedicated FRAM Table is located start 0x100 and goes to FRAMend
//  hai 20230503 - version 1.0
//*************************************************************************************
//
//
#ifndef _QUEST_FRAM_H_
#define _QUEST_FRAM_H_
//
//
#include "Arduino.h"
#include <Wire.h>
//
#define FRAM_ADDRESS 0x50  // FRAM memory address
#define FRAM_Size 0x1FF   //Fram is 32k in size
//
//uint16_t pulse = 13; // Define the pin number for the test
//
//------------------------  FRAM Memory Setup   -------------------
//
//
const uint16_t StartFram = (0x100);                          //Start location in FRAM memory for stored data
const uint16_t Valid = StartFram;							 //Test location for valid Fram present
const uint16_t Validsize = (2);								 //Size of valid Fram location
const uint16_t ID = (Valid+Validsize);                       //ID code of all files with this program
const uint16_t IDSize = (2);                                   //Size of ID code
const uint16_t MCcounter = (ID+IDSize);				 		 //binary mission clock counter
const uint16_t MCcountersize = (4);							 //size of MC counter
const uint16_t CumUnix = (MCcounter+MCcountersize);			 //Cumlitive Unix sec for Mission clock
const uint16_t CumUnixsize = (4);								 //Cumlitive Unix size
const uint16_t PreviousUnix = (CumUnix+CumUnixsize);		 //Previous Unix sec for Mission clock
const uint16_t PreviousUnixsize =(4);						 //Size of Previous unix
const uint16_t Resetaddress = (PreviousUnix + PreviousUnixsize); //Number of resets from program load address
const uint16_t ResetSize  = (2);                               //Size of Reset counter
const uint16_t PCSaddress = (Resetaddress + ResetSize);      //Default photo number address serial photos
const uint16_t PCSsize = (2);                                  //sixe of PC0address
const uint16_t PCPaddress = (PCSaddress + PCSsize);          //PCDaddress number address
const uint16_t PCPsize = (2);                                  //Size of PCDaddress
const uint16_t PCDaddress = (PCPaddress + PCPsize);          //PCMaddress number address Spi photos
const uint16_t PCDsize = (2);                                  //Size of PCMaddress
//
//
//const statement[] = "this is an array in fram"
//-----------------------------------------------------------------
//
//  Functional prototypes
//
//
uint16_t chkfram();								//return 1, Fram not present or not working
//
void writeIDfram(uint8_t id);						//set the unit ID code byte
uint8_t readIDfram();								//return with int of 2 alpha ID code 
void writebytefram(uint8_t value, uint16_t addr);	//write a byte to fram
void writeintfram(uint16_t value, uint16_t addr); 	//write an int to fram
void writelongfram(uint32_t value, uint16_t addr); 	//write 32 bits to fram
//
uint8_t readbyteFromfram(uint16_t addr); 			//return with byte 
uint16_t readintFromfram(uint16_t addr) ;			//return with 16 bits
uint32_t readlongFromfram(uint16_t addr);			//return with 32 bits
//
int framdump();										//dump all fram to terminal
int framclear();									//set all of fram to "0"
int initfram();										//init fram to default start values
//
void incMCcounter();			//inc Binary Mission Clock
void incphotoS();				//increment default photo number
void incphotoP();				//increment data file sequence
void incphotoD();				//increment movie photo sequence
void incresetnumber();			//increment reset number
//
void printMissionclock();		//print Mission Clock to terminal
//
//uint8_t SystemSetup();				//Setup Fram values for this module
//uint8_t Getchar();
//
#endif
