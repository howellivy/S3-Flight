/*
// Quest_Que - this is the output Que from microlab to Host controler
// it acts as a first in first our - like a ring buffer
//
// add a file name to the Que  (file name)
// get the next one off the Que (file name)
//
//20230508	hai
//
*/
//
//
#include "Quest_Que.h"
//
//
const uint8_t QUEUE_SIZE=33;
uint8_t queueHead=0;
uint8_t queueTail=0;
char queue[QUEUE_SIZE] [15];
//
//
//
//------------------------------------------------
//    Init Queue
//-------------------------------------------------
//
void InitQue(){
  queueHead = 0;
  queueTail = 0;
}
//
//
//-------------------------------------------------
//    Add a full file name to the Que
//-------------------------------------------------
//
void addFileToQueue(const char* filename) {
  if ((queueTail + 1) % QUEUE_SIZE != queueHead) {
    // Add filename to queue
    strncpy(queue[queueTail], filename, 15);
    queueTail = (queueTail + 1) % QUEUE_SIZE;
  }
  else{
    Serial.println("Output Que full");
  }
}
//
//-------------------------------------------------
//     Get a full file name from the Que
//-------------------------------------------------
//
uint16_t getFilefromQue(char *filename){				//set the next file from the Que
    if (queueHead != queueTail) {
        strncpy(filename, queue[queueHead], 80);
      	//filename = queue[queueHead];

        //
        //----
        Serial.print("\n\rGot file from Que = ");
        Serial.println(filename);
      	//-----
        //
      	queueHead = (queueHead + 1) % QUEUE_SIZE;
       	return 0;
    } else {
 		    Serial.println("Queue is empty");
        return 1;
    }
  }
//
uint16_t getQueStatus(){
    if(queueHead != queueTail){
      return 0;                     //Queue has file
    } else {
      return 1;
    }

}
//------------------------------------------------
//   list the contents of the Queue
//------------------------------------------------
//
 uint16_t listQue(){
  uint8_t i = queueHead;
  while (i != queueTail){
    Serial.print(queue[i]);
    Serial.print("\n");
    i = (i+1) % QUEUE_SIZE;
  }
  return 0;
}

