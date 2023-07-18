#include <SD.h>
#include "Quest_CameraC329.h"

#define CAMERA_BAUD 14400

Quest_CameraC329 camera;
File photoFile;
uint32_t writtenPictureSize;

void getPicture_callback(uint32_t pictureSize, uint16_t packetSize, uint32_t packetStartPosition, byte* packet)
{
  photoFile.write(packet, packetSize);
  writtenPictureSize += packetSize;
  
  if (writtenPictureSize >= pictureSize)
  {
    photoFile.flush();
    photoFile.close();
    Serial.println("SUCCESS");
  }
}

void setup()
{
  Serial.begin(CAMERA_BAUD);
  
  if (!SD.begin(4))
  {
    Serial.println("SD initialization failed");
    return;
  }
  
  photoFile = SD.open("photo.jpg", FILE_WRITE);
  photoFile.seek(0);
  writtenPictureSize = 0;
  
  delay(2000);

  if (!camera.sync())
  {
    Serial.println("Sync failed");
    return;
  }

  if (!camera.initialize(Quest_CameraC329::BAUD14400, Quest_CameraC329::CT_JPEG, Quest_CameraC329::PR_160x120, Quest_CameraC329::JR_320x240))
  {
    Serial.println("Initialize failed");
    return;
  }
  
  if (!camera.setQuality(Quest_CameraC329::QL_BEST))
  {
    Serial.println("Set quality failed");
    return;
  }
      
  if (!camera.getPicture(Quest_CameraC329::PT_JPEG_PREVIEW, &getPicture_callback))
  {
    Serial.println("Get Picture Failed");
    return;
  }
}

void loop()
{
}
