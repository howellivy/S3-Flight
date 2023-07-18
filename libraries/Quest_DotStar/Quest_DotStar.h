#ifndef QUEST_DOTSTAR_H
#define QUEST_DOTSTAR_H

#include <Arduino.h>

//Quest DotStar library 20220621 hai
// Version 0.1
// control the adafruit Dot star RGB
// inclulded on board the Istybitsy M4 board.
//
//Use these pin definitions for the ItsyBitsy M4
#define DotStarDATAPIN    8   //Data pin
#define DotStarCLOCKPIN   6   //Clock
#define testPIN           3   //
//

//
// prototype
//
void DotStarInit();             //Setup DotStar i/o pins
//
void DotStarsend(uint8_t Bright, uint8_t Blue, uint8_t Green, uint8_t Red);
//
void DotStarOff();         //Set DotStar LED to Red
void DotStarRed();         //Set DotStar LED to Red
void DotStarGreen();       //Set DotStar LED to Green
void DotStarYellow();      //Set DotStar LED to Yellow
void DotStarBlue();        //Set DotStar LED to Blue
void DotStarMagenta();     //Set DotStar LED to Yellow
void DotStarCyan();        //Set DotStar LED to Yellow
void DotStarWhite();       //Set DotStar LED to White


#endif