//*************************************************************************************
//*************************************************************************************
//  Quest operations with the installed FRAM 32K memory 
//  I2C address 0x50 and is 32K in size
//  Dedicated FRAM Table is located start 0x100 and goes to FRAMend
//  hai 20230503 - version 1.0
//*************************************************************************************
//************************************************************************************
//
#include "Quest_fram.h"
//
//
uint16_t chkFram(){                               //Fram locatgion to check if present
  writeintfram( 0x55AA,  Valid);  //write a value to be read back
  if(readintFromfram( Valid)==0x55AA){    //compair the value read back
    return 1;     //fram returned ok data         //correct value read back return 0
    }
    else {                    //if not good must be bad
      return 0;               //Error no response from Fram return 1
    }
}
//
//----------------------------------------
//----- Function to read and write ID into Fram
//----------------------------------------
void writeIDfram(uint8_t id){
    writebytefram(id, ID);
}
uint8_t readIDfram(){
    uint8_t x = readintFromfram(ID);
    return x;   
}
//
//--------------------------------------------------------------------------
//----- Function for writing a byte value to a specific FRAM memory location
//Enter with =>  writebytefram(address to write, byte to write);
//--------------------------------------------------------------------------
void writebytefram( uint8_t value, uint16_t addr) {
  Wire.beginTransmission(FRAM_ADDRESS);  // Start I2C communication with FRAM
  Wire.write((addr >> 8) & 0xFF);  // Send high byte of address
  Wire.write(addr & 0xFF);  // Send low byte of address
  Wire.write(value);  // Send data byte
  Wire.endTransmission();  // End I2C communication with FRAM
}
//
//---------------------------------------------------------------
//----- Function to write an int to fram memory
//Enter with => writeintfram(valueToWrite, addressToWrite);
//
void writeintfram(uint16_t value, uint16_t addr) {
  uint8_t* ptr = (uint8_t*)&value;
  uint16_t size = sizeof(value);
  Wire.beginTransmission(FRAM_ADDRESS);
  Wire.write((addr >> 8) & 0xFF); // Write the upper byte of the address
  Wire.write(addr & 0xFF); // Write the lower byte of the address  
  for(uint16_t i = 0; i < size; i++) {
    Wire.write(ptr[i]); // Write each byte of the int value
  }  
  Wire.endTransmission();
}
//-------------------------------------------------------------
//----- Function to write a long to fram memory
//Enter with: => writelongfram(Long in value, int address)
//
void writelongfram(uint32_t value, uint16_t addr) {
  byte* ptr = (byte*)&value;
  uint16_t size = sizeof(value);  
  Wire.beginTransmission(FRAM_ADDRESS);
  Wire.write((addr >> 8) & 0xFF); // Write the upper byte of the address
  Wire.write(addr & 0xFF); // Write the lower byte of the address  
  for(int i = 0; i < size; i++) {
    Wire.write(ptr[i]); // Write each byte of the long int value
  }  
  Wire.endTransmission();
}
//
//-------------------------------------------------------------------------
//----- Function for reading a byte from a specific FRAM memory location
//Enter with =>  uint8_t data = readByteFromFRAM(address to read);
//
uint8_t readbyteFromfram(uint16_t address) {
  Wire.beginTransmission(FRAM_ADDRESS);   // Start I2C communication with FRAM
  Wire.write((uint8_t)(address >> 8));    // Send high byte of address
  Wire.write((uint8_t)(address & 0xFF));  // Send low byte of address
  Wire.endTransmission();                 // End I2C communication with FRAM  
  Wire.requestFrom(FRAM_ADDRESS, 1);  // Request 1 byte of data from FRAM
  uint8_t data = Wire.read();  // Read the byte of data from FRAM  
  return data;  // Return the byte of data
}
//
//------------------------------------------------------------------
//----  Function to read an int from fram memory
//Enter with +>  int valueRead = readIntFromFRAM(addressToRead);
//
uint16_t readintFromfram(uint16_t addr) {
  uint16_t value = 0;
  uint8_t* ptr = (byte*)&value;
  uint16_t size = sizeof(value);
  Wire.beginTransmission(FRAM_ADDRESS);
  Wire.write((addr >> 8) & 0xFF); // Write the upper byte of the address
  Wire.write(addr & 0xFF); // Write the lower byte of the address
  Wire.endTransmission(false); // Send a restart message to keep the connection open
  Wire.requestFrom(FRAM_ADDRESS, size);
  for(int i = 0; i < size; i++) {
    ptr[i] = Wire.read(); // Read each byte of the int value
  }
  return value;
}
//-----------------------------------------------------------
// ----- Function to read a long from fram memory 
//enter with: => readlongfram(fram address);
//
uint32_t readlongFromfram(uint16_t addr) {
  uint32_t value = 0;
  byte* ptr = (byte*)&value;
  uint16_t size = sizeof(value);
  Wire.beginTransmission(FRAM_ADDRESS);
  Wire.write((addr >> 8) & 0xFF); // Write the upper byte of the address
  Wire.write(addr & 0xFF); // Write the lower byte of the address
  Wire.endTransmission();
  Wire.requestFrom(FRAM_ADDRESS, size); // Request the size of the long int value
  for(int i = 0; i < size; i++) {
    ptr[i] = Wire.read(); // Read each byte of the long int value
  }
  return value;
}
//
//----------------------------------------------------------------------
//----- Function to dump all of Fram
//
int framdump(){
  Serial.println();                         //do a linefeed and carrage return so it is clean
  int i=0;                                  //set i address pointer to 0 to print address
  Serial.printf("%04X ", i);                //print the first address
  Serial.print(" -- ");                     //spacer between address and data
  for(int i=1;i<(FRAM_Size+1);i++){         //loop count for the size of fram  
    uint8_t data = readbyteFromfram(i-1);   //fetch data at loop -1
    if(data<16){Serial.print("0");}          //leading zero for spacing //16 hai
    Serial.print(data, HEX);                //print data at the address
    Serial.print(" ");                      //space between data
    if(i<FRAM_Size){                        //check for loop end
      if(i%16==0){                          //new line ??
        Serial.println();                   //yes, send new line and carrage return
        Serial.printf("%04X ", i);          //print address of new line
        Serial.print(" -- ");               //spacer
      }
    }
  }
  uint8_t data = readbyteFromfram(FRAM_Size); //for last  byte
  if(data<8){Serial.print("0");}              //format it
  Serial.println(data, HEX);                  //print it
  return 0;
}
//
//------------------------------------------------------------------------
//-----  Function to clear Fram to 0
//
int framclear(){
  Serial.println("\n\rSetting all Fram to 0");
  for(int i=0;i<FRAM_Size+1;i++){               //loop for the size of Fram
    writebytefram(0x00, i);                     //write a 0 to address i 
    if(i%1024==0){
      Serial.print("*"); 
    }
  }
  framdump();
  Serial.println("\n\rfinished clear");
  return 0;
}
//------------------------------------------------------------------------
//------ Function to Init Fram
//
int initfram(){
 if(chkFram() == 0){
  Serial.println("\n\rFram Memory bad or not intalled\r\n Program will not function correctly\n\r");
 } 
  Serial.println("Initing Fram Memory");
  writeIDfram('B');                              //default ID = XY into Fram
  //
 // uint16_t x =  readIDfram();
 // if(x!=0x5859){
 //   Serial.println("Fram error - Fram May not be installed or bad!");
 //   Serial.println("Program will not run correctly without Fram !!!");
//}
//  Serial.println("Initing Mission clock");          //Set to "00000000"
//  writelongfram(0x66666666, MCcounter);               //Mission clock to zero
  //
  writeintfram( 0, Resetaddress);
  writeintfram( 0, PCSaddress); 
  writeintfram( 0, PCPaddress); 
  writeintfram( 0, PCDaddress);   
  //
  // head of Que pointer
//  writelongfram(0x55555555, CumUnix);       //total unix
//  writelongfram(0x77777777, PreviousUnix);  //last unix
//
//  Onlu to test addressing
//
  Serial.print("StartFram = ");Serial.println(StartFram);
  Serial.print("ID = ");Serial.println(ID);
  Serial.print("ResetAddres = ");Serial.println(Resetaddress);
  Serial.print("PCSaddress = ");Serial.println(PCSaddress);
  Serial.print("PCPaddress = ");Serial.println(PCPaddress);
  Serial.print("PCDaddress = ");Serial.println(PCDaddress);
//
  Serial.println("***** TEAM ID MUST NOW BE SET *****");
  return 0; 
}
//-------------------------------------------------------------
//----- Inc MCcounter -----
//
void incMCcounter(){
  uint32_t x = readlongFromfram(MCcounter);
  x++;
  writelongfram(x, MCcounter);
}
//-------------------------------------------------------------
//----- Inc photo number ----
//  Normal photo single shot photo number
void incphotoS(){
   uint16_t x = readintFromfram(PCSaddress);
   x++;
      if(x>99999){                          //Max number 99,999
    x=0;  
   }  
  writeintfram(x, PCSaddress);  
}  
//--------------------------------------------------------------
//----- Inc photo  D number ----
//  Data file transfer photo type
void incphotoD(){
   uint16_t x = readintFromfram(PCDaddress);
   x++;
   if(x>99999){                             //Max number 9,999 
    x=0;  
   }
  writeintfram(x, PCDaddress);  
}  
//--------------------------------------------------------------
//----- Inc photo number ----
//  Movie clip numbers
void incphotoP(){
   uint16_t x = readintFromfram(PCPaddress);
   x++;
      if(x>99999){                         //Max number 9,999
    x=0;  
   }  
  writeintfram(x, PCPaddress);  
}  
//
//-------------------------------------------------------------
//----- inc reset number
//
void incresetnumber(){
  uint16_t x = readintFromfram(Resetaddress);
  x++;
  writeintfram(x, Resetaddress);  
}
//
//------------------------------------------------------------
//---- Function to print mission clock on terminal
//  Serial.print("Mission Clock = ");
//
void printMissionclock(){
    uint32_t x = readlongFromfram(MCcounter) ;
    Serial.print("Mission seconds = ");
    Serial.print(x);
    Serial.print(" HEX = ");
    Serial.println(x, HEX);
    Serial.println();
} 
//
/*
uint8_t SystemSetup() {

  Serial.setTimeout(5000);                        //timeout to 5 seconds
  Serial.println("System Module Setup");
  Serial.print("What is you Module ID code? = ");

//
  uint8_t inputChar = Getchar();
  if(inputChar != 0xFF){
    writebytefram(inputChar, ID);                                 
  }else{
    return inputChar;                    //timeout abort
  }
  inputChar = Getchar();
  if(inputChar != 0xFF){
    writebytefram(inputChar, ID+1);                                 
  }else{
    return inputChar;                    //timeout abort
  }

return inputChar;
}
uint8_t Getchar() {

  if (Serial.available()) {  // Check if any data is available to read
    uint8_t inputChar = Serial.read();  // Read the incoming character
    Serial.print(inputChar);
    return inputChar;
    }else {
    Serial.println("Timeout occurred");    // Timeout occurred
    return 0xFF;
  }
}
*/