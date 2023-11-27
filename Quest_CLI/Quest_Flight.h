/*
 *                     *              20231110 Version 1.0
  20231120 CLI_V1.0 Required to make a complete program - this file, CLI_V1.0,Quest_CLI.h, Quest_Flight.h,Quest_flight.cpp
                  cmd_takeSphot , cmd_takeSpiphoto, nophotophoto, nophoto30K --,clean up code for understanding.

*/
//
#ifndef __FLYING_H__
#define __FLYING_H__

void Flying();
void add2text(int value1,int value2,int value3);      //add the values to the text buffer
int value1 = 0;                                       //valus used in add2text       
int value2 = 0;
int value3 = 0;
//
void dataappend(int sensor2count,int ampli,int SiPM,int Deadtime);
void appendToBuffer(const char* data);
//
#endif
