//*************************************************************************************
//  Quest Testing interface, will execute different operations 
//
//  hai 20230503 - Moved from mail code to library
//*************************************************************************************
//
#include "Quest_command.h"
//
#include "Arduino.h"
#include <Wire.h>
//
 //
 #ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
//
/*
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
//----------------------------------------------------------------
*/
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
/*
int cmd_text(){
 //   fill_text();
  //                    
   Serial.println(text_header);
   Serial.println (text_DataDown); 
   Serial.println (text_time);
   Serial.println (text_Unix);
   Serial.println (text_Mission);
   Serial.println (text_Atemp);
   Serial.println (text_humidity);
   Serial.println (text_gas);
   Serial.println (text_pressure);
   Serial.println (text_altitude);
   Serial.println (text_Ain0);
   Serial.println (text_Ain1);
   Serial.println (text_Ain2);
   Serial.println (text_Ain3);
   //   
   Serial.print (text_buf95);
   Serial.println (source_file);
   Serial.print (text_buf96);
   Serial.println (compile_date);
   Serial.print (text_buf97);
   Serial.println (Qversion);
   //
   return 0;
}
*/
