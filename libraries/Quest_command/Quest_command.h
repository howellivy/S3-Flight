//*************************************************************************************
//  Quest Testing interface, will execute different operations 
//
//  hai 20230503 - Moved from main code to library
//*************************************************************************************
//
#ifndef _QUEST_COMMAND_H_
#define _QUEST_COMMAND_H_
//
#include "Arduino.h"
#include <Wire.h>
//
//
//
// 
//Function declarations
void cli_init();
int cmd_help();
int cmd_led();
int cmd_exit();
int cmd_scani2c();
int cmd_bme680();
int cmd_time();
int cmd_settime();
int cmd_dir();
int cmd_mkdir();
int cmd_rmdir();
int cmd_open();
int cmd_erase();
int cmd_rtcreg();
int cmd_DotStar();        //13
int cmd_dump();           //14
int cmd_sphoto();         //15
int cmd_upload();         //16
int cmd_io();             //17
int cmd_ana();            //18
int cmd_takeSphoto();     //19
int cmd_stackandheap();   //20
int cmd_initQueue();      //21
int cmd_text();           //22
int cmd_view();           //23
int cmd_tdump();          //24
int cmd_info();           //25  program info
int cmd_trtc();           //26  test real time clock
int cmd_format();         //27  format SD
int cmd_free();           //28  free space on SD
int cmd_takeSpiphoto();   //29  for testing SPI Camera
int framdump();           //30  dump total contents of fram
int cmd_initfram();           //31  Default setup for Fram memory
int framclear();          //32  Set all of fram to "0"
int SystemSetup();         //33  Seup Fram Memory to Modual values
int ReadSetup();           //34 display the fram setup
int cmd_listQue();             //35list the contents of oputput Queue
int cmd_enterTeamID();      // require team ID input for all file names

void setPCF85263();
void readPCF85263();
void help_led();
void help_exit();
void help_help();
void help_scani2c();
void help_settime();
//
//
//
/* 
//List of functions pointers corresponding to each command
int (*commands_func[])(){
    &cmd_help,
    &cmd_led,
    &cmd_exit,
    &cmd_scani2c,
    &cmd_bme680,
    &cmd_time,
    &cmd_settime,
    &cmd_dir,
    &cmd_mkdir,
    &cmd_rmdir,
    &cmd_open,
    &cmd_erase,
    &cmd_rtcreg,
    &cmd_DotStar,
    &cmd_dump,
    &cmd_sphoto,
    &cmd_upload,
    &cmd_io,
    &cmd_ana,
    &cmd_B0stat,           //19
    &cmd_B1stat,           //20
    &cmd_B2stat,           //21
    &cmd_text,             //22
    &cmd_view,             //23
    &cmd_tdump,            //24
    &cmd_tc,               //25
    &cmd_trtc,             //26
    &cmd_format,           //27
    &cmd_free,             //28
    &cmd_spicam,           //29
    &Framdump,             //30
    &InitFRAM,             //31
    &FramClear             //32
};
 
//List of command names
const char *commands_str[] = {
    "?",
    "led",
    "exit",
    "scani2c",
    "bme680",
    "time",
    "settime",
    "dir",
    "mkdir",
    "rmdir",
    "open",
    "erase",
    "rtcreg",
    "DotStar",
    "dump",
    "sphoto",
    "upload",
    "io",
    "ana",
    "B0stat",         //19
    "B1stat",         //20
    "B2stat",         //21
    "text",           //22
    "view",           //23
    "tdump",          //24
    "tc",             //25
    "trtc",           //26
    "format",         //27
    "free",           //28
    "spicam",         //29
    "Framdump",       //30
    "InitFRAM",       //31
    "FramClear"       //32
};
 
//List of LED sub command names
const char *led_args[] = {
    "on",
    "off",
    "blink"
};
//List of DotStar sub command names
const char *DotStar_args[] = {
    "Off",
    "Red",
    "Green",
    "Yellow",
    "Blue",
    "Magenta",
    "Cyan",
    "White"
};
//
//-------------------------------------------- for command line processor
int num_commands = sizeof(commands_str) / sizeof(char *);
//-------------------------------------------- end command line processor
//
//
*/
//
//
 //-----------------------------------------------------------------
 //tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt
 //-----------------------------------------------------------------
 //
/*
 #ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
//
*/
//-----prototypes -----
int freeMemory();
int cmd_text();




#endif