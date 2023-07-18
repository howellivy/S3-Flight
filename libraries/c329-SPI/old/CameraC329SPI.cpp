/**
 * ************************************************************
 * *******   QUEST S3 MICROLAB Version   **********************
 * ************************************************************
 * 
 * Date = 20220329
 * This verision of CameraC329SPI has been modified by hai to make it compatable
 * to the S3 Microlab system....  Defaulted SPI rate was to fast for camera 24mhz
 * Changed to 4mhz during camear access time
 * lines 329, 230, and 298 added  SPI.setClockDivider((F_CPU + 4000000L) / 8000000L); 
 * 
 * 
 *  
 * End of QUEST
 * --------------------------------------------------------------
 * Copyright 2011
 * Sean Voisen <http://voisen.org>
 *
 * SPI port by Roy Shilkrot 2013 <http://www.morethantechnical.com>
 *
 * Based on the original library for the now-obsolete C328R camera, also by
 * Sean Voisen in collaboration with Beatriz da Costa 
 * <http://beatrizdacosta.net>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CameraC329SPI.h"
//

//-----------------------------------------------------------------------------
//
// Constants
//
//-----------------------------------------------------------------------------

static const byte CMD_PREFIX = 0xFF;
static const byte CMD_INITIAL = 0x01;
static const byte CMD_GETPICTURE = 0x04;
static const byte CMD_SNAPSHOT = 0x05;
static const byte CMD_RESET = 0x08;
static const byte CMD_POWEROFF = 0x09;
static const byte CMD_DATA = 0x0A;
static const byte CMD_SYNC = 0x0D;
static const byte CMD_ACK = 0x0E;
static const byte CMD_NAK = 0x0F;
static const byte CMD_QUALITY = 0x10;

/**
 * Constructor
 */
CameraC329SPI::CameraC329SPI(int _spi_cs_pin):spi_cs_pin(_spi_cs_pin)
{
}
/*
/ Setup for C329 SPI Camera
* init pins for C329 SPI camera
*/
bool CameraC329SPI::CameraC329SPIinit()
{

      pinMode(SPIcamHold,INPUT);                    //Hold input (physical jumper IO0,J6-3 to CHOLD,J6-4)    
      pinMode(SPI_cam_CS, OUTPUT);                  //Set SPI camera pin to output
      pinMode(SPI_cam_Power,OUTPUT);                //Set SPI camera power control pin         
      digitalWrite(SPI_cam_Power,SPIcamOFF);        //camera power off here    
      digitalWrite(SPI_cam_CS, HIGH);               //unselect camera during power up
      return true;
}
/**
 * Synchronize with the camera. Synchronization will be attempted up to
 * MAX_SYNC_ATTEMPTS. You must synchronize with the camera before you can
 * call other methods.
 *
 * @return True if successful, false otherwise.
 */
bool CameraC329SPI::sync()
{
  uint8_t syncAttempts = 0;
  bool success = false;

  setOutputCommand(CMD_SYNC, 0, 0, 0, 0);

  while (syncAttempts < MAX_SYNC_ATTEMPTS)
  {
    sendCommand();
   // delay(1);  //*****************************

//Serial.print(syncAttempts, HEX);Serial.print(" ");    //test hai

    // Wait for ACK response
    success = waitForACK(RESPONSE_DELAY, CMD_SYNC);

    // Make sure it is an ACK
    if (success)
    {
      // Now wait for SYNC from camera
      success = waitForResponse(RESPONSE_DELAY);
      if (success && inputCommand[3] == CMD_SYNC)
      {
        // All is good, flush the buffer
        // Send ACK
        setOutputCommand(CMD_ACK, CMD_SYNC, 0, 0, 0);
        sendCommand();
        return true;
      }
    }
    syncAttempts++;
  }

  return false;
}

/**
 * Sets the camera's initial baud rate, color type, and image sizes. Call this
 * method after synchronization to set up appropriate parameters before
 * taking a snapshot.
 *
 * @param baudRate The baud rate to use for future camera communication
 * @param colorType The color type
 * @param previewResolution The preview resolution
 * @param jpegResolution The JPEG resolution
 *
 * @return True if successful, false otherwise
 */
bool CameraC329SPI::initialize(BaudRate baudRate, ColorType colorType, 
    PreviewResolution previewResolution, JPEGResolution jpegResolution)
{
  setOutputCommand(CMD_INITIAL, baudRate, colorType, previewResolution,
    jpegResolution);
  sendCommand();

  if (waitForACK(RESPONSE_DELAY, CMD_INITIAL))
    return true;

  return false;
}

/**
 * Set the quality level for compressed (JPEG) images.
 *
 * @param qualityLevel The desired quality level. Should be a value from the
 * QualityLevel enumeration.
 */
bool CameraC329SPI::setQuality(QualityLevel qualityLevel)
{
  setOutputCommand(CMD_QUALITY, qualityLevel, 0, 0, 0);
  sendCommand();

  if (waitForACK(RESPONSE_DELAY, CMD_QUALITY))
    return true;

  return false;
}

/**
 * Takes a snapshot with the camera and stores it in the camera's internal
 * data buffer. Once a snapshot has been taken, use getPicture to retrieve
 * the photo data.
 *
 * @param pictureType The picture type to take. Should be a value from the
 * PictureType enumeration.
 *
 * @return True if successful, false otherwise
 */
bool CameraC329SPI::takeSnapshot(PictureType pictureType)
{
  setOutputCommand(CMD_SNAPSHOT, pictureType, 0, 0, 0);
  sendCommand();

  if (waitForACK(RESPONSE_DELAY, CMD_SNAPSHOT))
    return true;

  return false;
}

bool CameraC329SPI::getPicture(PictureType pictureType, void (*callback)(uint32_t pictureSize, uint16_t packetSize, uint32_t packetStartPosition, byte* packet))
{
  uint32_t pictureSize = 0;

  reset(RT_STATE);		//hai

//  while (!waitForACK(RESPONSE_DELAY, CMD_GETPICTURE) && ack_counter < 100) {
//    delay(10);
//  }

  uint32_t get_pic_tries = 0;
  while(get_pic_tries++ < 10) {       //-----------------------------hai
    setOutputCommand(CMD_GETPICTURE, pictureType, 0, 0, 0);
    sendCommand();

    uint32_t max_tries = 0, total_tries = 100;  //--------------------------------------hai
    while (!(waitForResponse(RESPONSE_DELAY) && inputCommand[3] == CMD_DATA) && max_tries++ < total_tries) {
		
	#ifdef SPIcamTestOut 	
  //    Serial.print("not DATA (");
	//  Serial.print(inputCommand[3], HEX);
	//  Serial.println(")");
  Serial.print("*");		  
	#endif
	
    delay(1); // delay(10);
    }
      Serial.println(max_tries);    //-----------------------------------------------hai
    if(max_tries < total_tries) break;
  }
  if(get_pic_tries >= 10) return false;

		//Serial.println(inputCommand[7], HEX);	//hai
		//Serial.println(inputCommand[6], HEX);	//hai
		//Serial.println(inputCommand[5], HEX);	//hai


long int x=0;
long int y=0;
long int z=0;
x = inputCommand[7];
y = inputCommand[6];
z = inputCommand[5];
x=x*65536;
y=y*256;
z=z;
pictureSize=(x+y+z);

#ifdef SPIcamTestOut

	Serial.print("Data type = ");		//hai
	Serial.println(inputCommand[4]);	//hai

	Serial.print("pictureSize = ");		//hai
	Serial.println(pictureSize, HEX);		//hai
#endif

  uint32_t bytePosition = 0;
  uint8_t package[DEFAULT_PACKAGE_SIZE];

  while (bytePosition < pictureSize)
  {
    if (!waitForResponse(RESPONSE_DELAY, package, DEFAULT_PACKAGE_SIZE))
      return false;
//	Serial.println(bytePosition);		//hai found boundry problem

     SPI.setClockDivider((F_CPU + 4000000L) / 8000000L); 

    callback(pictureSize, min(DEFAULT_PACKAGE_SIZE, pictureSize - bytePosition), bytePosition, package);
    bytePosition += DEFAULT_PACKAGE_SIZE;
  }

  return true;
}

/**
 * Resets the camera.
 *
 * @param resetType The type of reset to perform (ether a "soft" reset which
 * resets only the camera's internal state machine) or a "hard" reset.
 *
 * @return True if successful, false otherwise.
 */
bool CameraC329SPI::reset(ResetType resetType)
{
  setOutputCommand(CMD_RESET, resetType, 0, 0, 0xFF);
  sendCommand();

  if (waitForACK(RESPONSE_DELAY, CMD_RESET))
    return true;

  return false;
}

/**
 * Power off the camera. The camera will be unusable after calling this method
 * until a successful re-synchronization.
 *
 * @return True if successful, false otherwise
 */
bool CameraC329SPI::powerOff()
{
  setOutputCommand(CMD_POWEROFF, 0, 0, 0, 0);
  sendCommand();

  if (waitForACK(RESPONSE_DELAY, CMD_POWEROFF))
    return true;

  return false;
}

/**
 * @private
 *
 * Sends the command that is in the outputCommand buffer. Command bytes are
 * sent in big endian order.
 */
void CameraC329SPI::sendCommand()
{
  uint8_t i;
  //Serial.print("send command ");
  // Big endian
  
  		delayMicroseconds(5);      // for Grand central hai 8.5us between bytes  hai
  
  for (i = 0; i < CMD_SIZE; i++)
  {
    //Serial.print(outputCommand[i]); Serial.print(" ");
    //cameraPort.print(outputCommand[i], BYTE);
    //cameraPort.write((uint8_t)outputCommand[i]);
	int y;
	y=digitalRead(SPIcamHold);
	if(y==LOW){ 								

	   SPI.setClockDivider((F_CPU + 4000000L) / 8000000L); 

    digitalWrite(SPI_cam_CS,LOW);
    SPI.transfer(outputCommand[i]);
    digitalWrite(SPI_cam_CS,HIGH);

  }
		delayMicroseconds(10);//5);      // for Grand central hai 8.5us between bytes  hai
		//Serial.println(" ");						//hai
  }
  
  
Serial.println();
}

bool CameraC329SPI::waitForACK(uint32_t timeout, uint8_t cmd)
{
  bool success = waitForResponse(timeout);

  if (success && inputCommand[3] == CMD_ACK && inputCommand[4] == cmd)
    return true;

  return false;
}

/**
 * @private
 *
 * Waits for a response up to timeout, and stores the response in the
 * inputCommand array.
 *
 * @return True if response is received before the timeout, false otherwise.
 */
bool CameraC329SPI::waitForResponse(uint32_t timeout)
{
  return waitForResponse(timeout, inputCommand, CMD_SIZE);
}

/**
 * @private
 *
 * Waits for a response up to timeout, and stores the response in the buffer
 * array provided.
 *
 * @param timeout The timeout
 * @param buffer The buffer within which to store the response
 * @param bufferLength The length of the provided buffer
 *
 * @return True if response is received before timeout, false otherwise
 */
bool CameraC329SPI::waitForResponse(uint32_t timeout, byte buffer[], uint16_t bufferLength)
{
  uint8_t byteCount = 0;
  unsigned long time = millis();

//Serial.print("response: ");
  while (millis() - time <= timeout)
  {
//    while (cameraPort.available() > 0)
	//delayMicroseconds(5);      // for Grand central hai 8.5us between bytes  hai
    {
	int y;
	y=digitalRead(SPIcamHold);
	if(y==LOW){
		  SPI.setClockDivider((F_CPU + 4000000L) / 8000000L); 
      digitalWrite(SPI_cam_CS,LOW);
      buffer[byteCount] = SPI.transfer(0x00); //cameraPort.read();
      digitalWrite(SPI_cam_CS,HIGH);
	}
		//Serial.print(buffer[byteCount],HEX); Serial.print(" ");      //hai
		
	delayMicroseconds(5);      // for Grand central hai 8.5us between bytes  hai
 
	byteCount++;

      if (byteCount == bufferLength) {
//Serial.println();
        return true;
      }
    }
  }
//Serial.println();
  if (byteCount > 0)
    return true;

  return false;
}

/**
 * @private
 *
 * Utility method for populating the output command buffer.
 */
void CameraC329SPI::setOutputCommand(const byte command, byte param1, byte param2, byte param3, byte param4)
{
  outputCommand[0] = CMD_PREFIX;
  outputCommand[1] = CMD_PREFIX;
  outputCommand[2] = CMD_PREFIX;
  outputCommand[3] = command;
  outputCommand[4] = param1;
  outputCommand[5] = param2;
  outputCommand[6] = param3;
  outputCommand[7] = param4;
}
