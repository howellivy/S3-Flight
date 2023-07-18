//
//
#include "Quest_fram.h"
//
//***********************************************************************************
//***********************************************************************************
void setup() {
  Wire.begin();           // Initialize I2C communication
  Wire.setClock(400000);  // set I2C clock frequency to 400kHz 
  Serial.begin(9600);     // Initialize serial communication
  while (!Serial);        // Wait for serial monitor to open
  Serial.println("\n\r<<<<< setup here >>>>>>");
//--------------  
  if(chkFram()==1){
      Serial.println("\n\rERROR=>Fram Memory not present of not working");
    }
    else {
      Serial.println("\n\rFram Memory OK present");
    }
//
//----------------
//
  FramClear()       ;Serial.println("Fram Clear done"); 
  Initfram();
}
//
//************************************************************************************
//************************************************************************************
//****************************** Testing loop **********************************
//
void loop() {
  //
  //FramClear();Serial.print("done");
  //
  
    delay(1);     //sync analizer 
    incMC();      //inc Mission Clock
    delay(1);     //delay only to sync logic analizer

    Serial.print("Mission Clock = ");
    printMissionclock();
    //
   //
    uint32_t x = readlongFromfram(MCcounter);    //readlongFromfram(MCcounter);   //get the binary counter
    x++;
    writelongfram(x, MCcounter);
    //
    Serial.println();
    Serial.print("Mission Clock Binary = ");
    Serial.println(x, HEX);
    Serial.println();

    
    Serial.print(" resetnumber = ");
    incresetnumber();
    Serial.print(readintFromfram(Resetaddress));
    Serial.print("   PC0 = ");
    incphoto0();
    Serial.print(readintFromfram(PC0address));
    Serial.print("   PCD = ");
    incphotoD();
    Serial.print(readintFromfram(PCDaddress));   
     Serial.print("  PCM = ");
    incphotoM();
    Serial.print(readintFromfram(PCMaddress)); 
    Serial.println();
//
    delay(100);       //delay to read
}
