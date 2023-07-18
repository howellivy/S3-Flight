/**
 * @file flying.ino
 * @author Your-Name-Here (you@domain.com)
 * @brief Logic for experiment My-Cool-Experiment
 * @version 0.1
 * @date 2023-07-15
 * 
 */

#include "Quest_Flight.h"
#include "Quest_CLI.h"


/**
 * @brief Flying function is used to capture all logic of an experiment during flight.
 */
void Flying(){
  Serial.println("Here to Run flight program, not done yet 20220717");
  Serial.println(" 20230518 working on it");

#define one_sec   1000                         //one second = 1000 millis
#define one_min   60*one_sec                   // one minute of time
#define one_hour  60*one_min                   // one hour of time
//
#define eventTime0  one_min*2                 //event time 1 time
#define eventTime1  one_min*1                 //make it hard

//
  uint32_t event0timer = millis();                   //event zero timer
  uint32_t event1timer = millis();
  uint32_t event2timer = millis();
//
  uint32_t one_secTimer = millis();
  uint32_t sec60Timer = millis();



//------------ flying -----------------------
  
  Serial.println("Flying NOW  -  x=abort");
  Serial.println("Terminal must be reset after abort");

  missionMillis = millis();     //1st time set
  //
  /////////////////////////////////////////////////////
  //----- Here to start a flight from a reset ---------
  /////////////////////////////////////////////////////
  //
  DateTime now = rtc.now();                   //get time now
  currentunix = (now.unixtime());             //get current unix time, don't count time not flying
  writelongfram(currentunix, PreviousUnix);   //set fram Mission time start now
  //
  //Serial.print("CumUnix is at fram address = ");Serial.println(CumUnix);
  //Serial.print("PreviousUnix is at fram address = ");Serial.println(PreviousUnix);
  //
  //
  //
  while(1){
      //
      //----------- Test for terminal abort command from flying ----------------------
      //
      while(Serial.available()){        //Abort flight program progress
       byte x = Serial.read();          //get the input byte
       if (x == 'x'){                   //check the byte for an abort x 
       return  ;                        //return back to poeration sellection
       }                                //end check
      }                                 //end abort check
//-------------------------------------------------------------------
//  long eventTime0 = one_min;                    //event time 1 time
//
    if((millis() - event0timer)>eventTime0){
         event0timer = millis();                    //reset event0timer
         Serial.println();
         Serial.println(millis());
         //
         //  Build the User Text buffer here
         //
         strcat(user_text_buf0,("User Buffer space here, millis = "));
         itoa(millis(),text_buf,10);
         strcat(user_text_buf0,(text_buf));
         //
         //  Take a serial photo and place file name in Queue
         //
        cmd_takeSphoto();                          //Take photo and send it
        delay(100);
        
        // Call the freeMemory function and print the result
        cmd_stackandheap();

  //
 //       logit_string();                            //log the event
    }
//----------------------------------------------------------------------
//  Event 1 timer operation
//
     if((millis() - event1timer)>eventTime1){
         event1timer = millis();                    //reset event0timer
         Serial.println();
         Serial.println(millis());
         //
         //  Build the User Text buffer here
         //
//         strcat(user_text_buf0,("User Buffer space here, millis = "));
//         itoa(millis(),text_buf,10);
//         strcat(user_text_buf0,(text_buf));
         //
         //  Take a serial photo and set flag data is ready for host to take
         //
 //        cmd_takeSpiphoto();                              //Take photo and send it
//         logit_string();                            //log the event
 //        delay(500);
//         cmd_takeSpiphoto();
//         logit_string();
    }
//------------------------------------------------------------------    
//------------  Here one sec timer  ------ -every second ------
//
    if((millis() - one_secTimer) > one_sec){
      one_secTimer = millis();
      DotStarYellow();                              //Blink Yellow when flying
      Serial.write('+');
      logit('+');
      //
//-------------------------------
//    Mission clock timer 
//    FRAM keep track of cunlitive power on time
//    and RTC with unix seconds    
//-------------------------------
//
      DateTime now = rtc.now();                           //get the time time,don't know how long away 
      currentunix = (now.unixtime());                     //get current unix time
      Serial.print(currentunix);Serial.print("  ");       //testing print unix clock
      uint32_t framdeltaunix =(currentunix - readlongFromfram(PreviousUnix)); //get delta sec of unix time
      uint32_t cumunix = readlongFromfram(CumUnix);       //Get cumulative unix mission clock
      writelongfram((cumunix+framdeltaunix),CumUnix);     //add and Save cumulative unix time Mission
      writelongfram(currentunix, PreviousUnix);           //reset PreviousUnix to current for next time
          Serial.print("--- Fram Mission Clock = ");      //testing print mission clock
          Serial.print(readlongFromfram(CumUnix));        //mission clock
          Serial.print("  =>   ");                        //spacer
        //
        //------Output the terminal  days hours min sec
        // 
            getmissionclk();
            Serial.print(xd);Serial.print(" Days   ");
            Serial.print(xh);Serial.print(" Hours  ");
            Serial.print(xm);Serial.print(" Min  ");
            Serial.print(xs);Serial.println(" Sec");

            //

            //
//
//------------ print time once a min -------------------
//
      if((millis()-sec60Timer) > (60*one_sec)){
        sec60Timer=millis();
        Serial.println();
        cmd_time();   
      }
      DotStarOff();
    }
  }  
}