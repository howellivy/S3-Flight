/*----------------------------------------------------------------------------
// Quest_Que - this is the output Que from microlab to Host controler
// it acts as a first in first our - like a ring buffer
//
// add a file name to the Que  (file name)
// get the next one off the Que (file name)
//
//20230508	hai
*///-------------------------------------------------------------------------
//
//
#ifndef _QUEST_QUE_H_
#define _QUEST_QUE_H_
//
//
#include <stdint.h>
#include "Arduino.h"
#include <SdFat.h>
//
//
// File queue
//
//const uint8_t QUEUE_SIZE=33;
//uint8_t queueHead=0;
//uint8_t queueTail=0;
//char queue[QUEUE_SIZE] [15];
//
//
//
// XXXXXXXX.JPGLL  = 8name 1. 3type 2Length
//
//
void InitQue(void);								//init out Que
//
void addFileToQueue(const char* filename);		//add a file to the Que
//
uint16_t getFilefromQue(char* filename);					//set the next file from the Que
//
uint16_t getQueStatus(void);					//return 0 = file availiable
//												//return 1 = No file availiable
//
//
#endif
