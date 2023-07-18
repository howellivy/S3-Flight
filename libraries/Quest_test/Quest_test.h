/*----------------------------------------------------------------------------
// Quest_Test- this is a library that will allow the insert of a pulse on
// different IO lines that can be used to test the time and if a function is
// being called
//
// Pulse13();.......Pulse11();
// 
//
//20230516	hai
*///-------------------------------------------------------------------------
//
//
#ifndef _QUEST_TEST_H_
#define _QUEST_TEST_H_
//
//
#include <stdint.h>
#include "Arduino.h"
//
//
void PulseInit(void);
//
void Pulse13(void);		//pulse pin 13 / IO7 one cpu cycle time
void Pulse12(void);		//pulse 12 / IO6
void Pulse11(void);		//pulse 11 / IO5
void Pulse10(void);		//pulse 10 / IO4
void Pulse9(void);		//pulse  9 / IO3
void Pulse4(void);		//pulse  4 / IO2
//
void Pulse13high(void);	//make pin 13 high
void Pulse13low(void);	//make pin 13 low
void Pulse12high(void);	//make pin 12 high
void Pulse12low(void);	//make pin 12 low
void Pulse11high(void);	//make pin 11 high
void Pulse11low(void);	//make pin 11 low
void Pulse10high(void);	//make pin 10 high
void Pulse10low(void);	//make pin 10 low
void Pulse9high(void);	//make pin 9 high
void Pulse9low(void);	//make pin 9 low
void Pulse4high(void);	//make pin 4 high
void Pulse4low(void);	//make pin 4 low
//
//
//
//
#endif
