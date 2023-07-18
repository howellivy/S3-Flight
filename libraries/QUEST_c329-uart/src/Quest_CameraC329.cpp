/**
 * Copyright 2011
 * Sean Voisen <http://voisen.org>
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

#include "CameraC329.h"

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
CameraC329::CameraC329()
{
}

/**
 * Synchronize with the camera. Synchronization will be attempted up to
 * MAX_SYNC_ATTEMPTS. You must synchronize with the camera before you can
 * call other methods.
 *
 * @return True if successful, false otherwise.
 */
bool CameraC329::sync()
{
  uint8_t syncAttempts = 0;
  bool success = false;

  setOutputCommand(CMD_SYNC, 0, 0, 0, 0);

  while (syncAttempts < MAX_SYNC_ATTEMPTS)
  {
    sendCommand();

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
        Serial.flush();

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
bool CameraC329::initialize(BaudRate baudRate, ColorType colorType, 
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
bool CameraC329::setQuality(QualityLevel qualityLevel)
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
bool CameraC329::takeSnapshot(PictureType pictureType)
{
  setOutputCommand(CMD_SNAPSHOT, pictureType, 0, 0, 0);
  sendCommand();

  if (waitForACK(RESPONSE_DELAY, CMD_SNAPSHOT))
    return true;

  return false;
}

bool CameraC329::getPicture(PictureType pictureType, void (*callback)(uint32_t pictureSize, uint16_t packetSize, uint32_t packetStartPosition, byte* packet))
{
  uint32_t pictureSize = 0;

  setOutputCommand(CMD_GETPICTURE, pictureType, 0, 0, 0);
  sendCommand();

  if (!waitForACK(RESPONSE_DELAY, CMD_GETPICTURE))
    return false;

  if (!(waitForResponse(RESPONSE_DELAY) && inputCommand[3] == CMD_DATA))
    return false;

  pictureSize = inputCommand[7] << 8;
  pictureSize |= inputCommand[6] << 8;
  pictureSize |= inputCommand[5];

  uint32_t bytePosition = 0;
  uint8_t package[DEFAULT_PACKAGE_SIZE];

  while (bytePosition < pictureSize)
  {
    if (!waitForResponse(RESPONSE_DELAY, package, DEFAULT_PACKAGE_SIZE))
      return false;

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
bool CameraC329::reset(ResetType resetType)
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
bool CameraC329::powerOff()
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
void CameraC329::sendCommand()
{
  uint8_t i;

  // Big endian
  for (i = 0; i < CMD_SIZE; i++)
  {
    Serial.print(outputCommand[i], BYTE);
  }
}

bool CameraC329::waitForACK(uint32_t timeout, uint8_t cmd)
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
bool CameraC329::waitForResponse(uint32_t timeout)
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
bool CameraC329::waitForResponse(uint32_t timeout, byte buffer[], uint16_t bufferLength)
{
  uint8_t byteCount = 0;
  unsigned long time = millis();

  while (millis() - time <= timeout)
  {
    while (Serial.available() > 0)
    {
      buffer[byteCount] = Serial.read();
      byteCount++;

      if (byteCount == bufferLength)
        return true;
    }
  }

  if (byteCount > 0)
    return true;

  return false;
}

/**
 * @private
 *
 * Utility method for populating the output command buffer.
 */
void CameraC329::setOutputCommand(const byte command, byte param1, byte param2, byte param3, byte param4)
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
