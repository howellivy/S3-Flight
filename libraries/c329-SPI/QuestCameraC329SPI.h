/**
 * Derived from:
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
#ifndef QUESTCAMERAC329SPI_H_
#define QUESTCAMERAC329SPI_H_

#include "Arduino.h"		//hai
#include <inttypes.h>
#include <SPI.h>

const int SPI_cam_CS = 5;              //set spicamera CS pin. MK
const int SPI_cam_Power = 7;           //SPI camera power pin
const int SPIcamON = 1;                 //Power on the SPI camera    Removed for test of quest init 0503
const int SPIcamOFF = 0;                //Power off the SPI camera
const int SPIcamHold = A6;              //SPI camera Hold pin

// The byte size of UART commands
#define SPICMD_SIZE 8

// Number of sync attempts to try before giving up
#define SPIMAX_SYNC_ATTEMPTS 100

// How long to wait for  communication responses
#define SPIRESPONSE_DELAY 200

// Size (in bytes) to split up picture data into packages for processing
#define SPIDEFAULT_PACKAGE_SIZE 128				//was 64 changed to 16 for 16 boundry on command boundry hai

#define SPIcamTestOut  //enable to send statuses to Serial out


class QuestCameraC329SPI
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
      CT_JPEG = 0x87
    };
    enum PreviewResolution
    {
      PR_80x60 = 0x01,
      PR_88x72 = 0x02,
      PR_160x120 = 0x03,
      PR_176x144 = 0x04,
      PR_320x240 = 0x05,
      PR_352x288 = 0x06,
      PR_640x480 = 0x07,
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
      JR_640x480 = 0x07,
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

    QuestCameraC329SPI(int _spi_cs_pin);
    bool syncSPI();
    bool initializeSPI(BaudRate, ColorType, PreviewResolution, JPEGResolution);
    bool setQualitySPI(QualityLevel);
    bool takeSnapshot(PictureType);
    bool getPictureSPI(PictureType, void (*)(uint32_t, uint16_t, uint32_t, byte*));
    bool resetSPI(ResetType);
    bool powerOffSPI();

    bool QuestCameraC329SPIinit();

  private:
    byte outputCommand[SPICMD_SIZE];
    byte inputCommand[SPICMD_SIZE];
    void sendCommand();
    bool waitForACK(uint32_t, uint8_t);
    bool waitForResponse(uint32_t);
    bool waitForResponse(uint32_t, byte[], uint16_t);
    void setOutputCommand(const byte, byte, byte, byte, byte);

    int spi_cs_pin;
};

#endif // QuestCameraC329SPI_H_
