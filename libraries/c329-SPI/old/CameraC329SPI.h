/**
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
#ifndef CAMERAC329SPI_H_
#define CAMERAC329SPI_H_

//#include "WProgram.h"
//#include "WConstants.h"
#include "Arduino.h"		//hai
//#include <inttypes.h>
//#include <NewSoftSerial.h>
#include "QuestInit.h"         //20220425
#include <SPI.h>

const int SPI_cam_CS = 5;              //set spicamera CS pin. MK
const int SPI_cam_Power = 7;           //SPI camera power pin
const int SPIcamON = 1;                 //Power on the SPI camera    Removed for test of quest init 0503
const int SPIcamOFF = 0;                //Power off the SPI camera
const int SPIcamHold = A6;              //SPI camera Hold pin
/*
const int B_MicroSD_CS = A5;            //sd card chip select(MK)
*/


// The byte size of UART commands
#define CMD_SIZE 8

// Number of sync attempts to try before giving up
#define MAX_SYNC_ATTEMPTS 100

// How long to wait for serial communication responses
#define RESPONSE_DELAY 200

// Size (in bytes) to split up picture data into packages for processing
#define DEFAULT_PACKAGE_SIZE 16 				//was 64 changed to 16 for 16 boundry on command boundry hai

#define SPIcamTestOut  //enable to send statuses to Serial out


class CameraC329SPI
{
  public:
    enum BaudRate
    {
	  BAUD0 = 0,
      BAUD7200 = 0x08,
      BAUD14400 = 0x07,
      BAUD28800 = 0x06,
      BAUD57600 = 0x05,
      BAUD115200 = 0x04,
      BAUD230400 = 0x03,
      BAUD460800 = 0x02,
      BAUD921600 = 0x01
    };
    enum ColorType
    {
      CT_Y4 = 0x02,
      CT_Y8 = 0x03,
      CT_RGB12 = 0x05,
      CT_RGB16 = 0x06,
      CT_RGB24 = 0x08,
      CT_YUV16 = 0x09,
      CT_JPEG = 0x07
    };
    enum PreviewResolution
    {
      PR_80x60 = 0x01,
      PR_88x72 = 0x02,
      PR_160x120 = 0x03,
      PR_176x144 = 0x04,
      PR_320x240 = 0x05,
      PR_352x288 = 0x06,
      PR_640x480 = 0x87,
      PR_80x64 = 0x08,
      PR_128x96 = 0x09,
      PR_128x128 = 0x0A,
      PR_160x128 = 0x0B
    };
    enum JPEGResolution
    {
      JR_80x60 = 0x01,
      JR_88x72 = 0x02,
      JR_160x120 = 0x03,
      JR_176x144 = 0x04,
      JR_320x240 = 0x05,
      JR_352x288 = 0x06,
      JR_640x480 = 0x87,
      JR_80x64 = 0x08,
      JR_128x96 = 0x09,
      JR_128x128 = 0x0A,
      JR_160x128 = 0x0B
    };
    enum PictureType
    {
      PT_SNAPSHOT = 0x01,
      PT_PREVIEW = 0x02,
      PT_SERIAL_FLASH = 0x03,
      PT_JPEG_PREVIEW = 0x05,
      PT_PLAYBACK = 0x06
    };
    enum DataType
    {
      DT_REGISTER = 0x00,
      DT_SNAPSHOT = 0x01,
      DT_PREVIEW = 0x02,
      DT_JPEG_PREVIEW = 0x04,
      DT_PLAYBACK = 0x05
    };
    enum ResetType
    {
      RT_ALL = 0x00,
      RT_STATE = 0x01
    };
    enum QualityLevel
    {
      QL_BEST = 0x00,
      QL_BETTER = 0x01,
      QL_NORMAL = 0x02
    };

    CameraC329SPI(int _spi_cs_pin);
    bool sync();
    bool initialize(BaudRate, ColorType, PreviewResolution, JPEGResolution);
    bool setQuality(QualityLevel);
    bool takeSnapshot(PictureType);
    bool getPicture(PictureType, void (*)(uint32_t, uint16_t, uint32_t, byte*));
    bool reset(ResetType);
    bool powerOff();

    bool CameraC329SPIinit();

  private:
    byte outputCommand[CMD_SIZE];
    byte inputCommand[CMD_SIZE];
    void sendCommand();
    bool waitForACK(uint32_t, uint8_t);
    bool waitForResponse(uint32_t);
    bool waitForResponse(uint32_t, byte[], uint16_t);
    void setOutputCommand(const byte, byte, byte, byte, byte);

    int spi_cs_pin;
};

#endif // CameraC329SPI_H_
