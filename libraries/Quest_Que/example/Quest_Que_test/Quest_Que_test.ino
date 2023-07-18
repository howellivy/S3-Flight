#include <SdFat.h>
#include "Quest_fram.h"
// SD card settings
const uint8_t SD_CHIP_SELECT = A5;  // SD card chip select pin

// Serial buffer
const uint8_t BUFFER_SIZE = 64;
char buffer[BUFFER_SIZE];
uint8_t bufferIndex = 0;

// File queue
const uint8_t QUEUE_SIZE = 33;
char queue[QUEUE_SIZE][13];
uint8_t queueHead = 0;
uint8_t queueTail = 0;

// SD card object
SdFat sd;

void addFileToQueue(const char* filename);


void setup() {
  Serial.begin(9600);
  while(!Serial);
  InitFRAM;
  // Initialize SD card
  if (!sd.begin(SD_CHIP_SELECT)) {
    Serial.println("Failed to initialize SD card");
    while (true);
  }
Serial.println("Test of Que, files added to Que");
  // Add files to queue
    addFileToQueue("HI00000.txt"); 
    addFileToQueue("HI00001.txt"); 
    addFileToQueue("HI00002.txt"); 
    addFileToQueue("HI00003.txt");
    addFileToQueue("HI00004.txt");
    addFileToQueue("HI00005.txt"); 
    addFileToQueue("HI00006.txt"); 
    addFileToQueue("HI00007.txt"); 
    addFileToQueue("HI00008.txt");
    addFileToQueue("HI00009.txt");
    //
    addFileToQueue("HI00000.txt"); 
    addFileToQueue("HI00001.txt"); 
    addFileToQueue("HI00002.txt"); 
    addFileToQueue("HI00003.txt");
    addFileToQueue("HI00004.txt");
    addFileToQueue("HI00005.txt"); 
    addFileToQueue("HI00006.txt"); 
    addFileToQueue("HI00007.txt"); 
    addFileToQueue("HI00008.txt");
    addFileToQueue("HI00009.txt");
    //
    addFileToQueue("HI00000.txt"); 
    addFileToQueue("HI00001.txt"); 
    addFileToQueue("HI00002.txt"); 
    addFileToQueue("HI00003.txt");
    addFileToQueue("HI00004.txt");
    addFileToQueue("HI00005.txt"); 
    addFileToQueue("HI00006.txt"); 
    addFileToQueue("HI00007.txt"); 
    addFileToQueue("HI00008.txt");
    addFileToQueue("HI00009.txt");
    //
    addFileToQueue("HI00000.txt"); 
    addFileToQueue("HI00001.txt"); 
    addFileToQueue("HI00002.txt"); 
    addFileToQueue("HI00003.txt");
    addFileToQueue("HI00004.txt");
    addFileToQueue("HI00005.txt"); 
    addFileToQueue("HI00006.txt"); 
    addFileToQueue("HI00007.txt"); 
    addFileToQueue("HI00008.txt");
    addFileToQueue("HI00009.txt");       
    /*
    addFileToQueue("HI00009.txt"); 
    addFileToQueue("HI00010.txt"); 
    addFileToQueue("HI00011.txt"); 
    addFileToQueue("HI00012.txt");
    //
  addFileToQueue("file1.txt");
  addFileToQueue("file2.txt");
  addFileToQueue("file3.txt");
  addFileToQueue("file4.txt");
  addFileToQueue("file5.txt");
//  addFileToQueue("file6.txt"); 
//  addFileToQueue("file7.txt");
//  addFileToQueue("file8.txt");

    addFileToQueue("HI00000.txt"); 
    addFileToQueue("HI00001.txt"); 
    addFileToQueue("HI00002.txt"); 
    addFileToQueue("HI00003.txt");
    addFileToQueue("HI00004.txt"); 
    */
}

void loop() {
  // Check for incoming serial data
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      // End of line, process command
      
      buffer[bufferIndex] = '\0';
      processCommand(buffer);
      bufferIndex = 0;
      
    } else if (bufferIndex < BUFFER_SIZE - 1) {
      // Add character to buffer
      buffer[bufferIndex] = c;
      bufferIndex++;
    }
  }
}
//
//  To add to Que use full file name in quotes 
//
void addFileToQueue(const char* filename) {
  if ((queueTail + 1) % QUEUE_SIZE != queueHead) {
    // Add filename to queue
    strncpy(queue[queueTail], filename, 13);
    queueTail = (queueTail + 1) % QUEUE_SIZE;
  }
  else{
    Serial.println("Output Que full");
  }
}

void processCommand(const char* command) {
  if (strcmp(command, "list") == 0) {
    // List files in queue
    uint8_t i = queueHead;
    while (i != queueTail) {
      Serial.print(queue[i]);
      Serial.print("\n");
      i = (i + 1) % QUEUE_SIZE;
    }
    //
    //
    //
  } else if (strcmp(command, "send") == 0) {
    // Send next file in queue
    uint16_t x = sendfile();
    if(x==2){
      Serial.println("Queue Empty");
    }
    if(x==1){
      Serial.println("Can not openfile");
    }
  }
  else {
    // Unknown command
    Serial.println("Unknown command");
   }
}
  
    //
    //
uint16_t  sendfile(){
    if (queueHead != queueTail) {
      const char* filename = queue[queueHead];
      SdFile file;
      if (file.open(filename, O_READ)) {
        while (file.available() > 0) {
          Serial.write(file.read());
        }
        file.close();
        Serial.print("\n");
        queueHead = (queueHead + 1) % QUEUE_SIZE;
        return 0;
      } else{
//        Serial.println("Failed to open file");
        return 1;
      }
    } else {
      // Queue is empty
 //     Serial.println("Queue is empty");
      return 2;
    }
  }
 //       addFileToQueue("HI00005.txt");        ////??????????????????
