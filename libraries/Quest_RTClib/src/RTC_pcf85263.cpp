/**************************************************************************/
/***************************************************************************
 *   This is a Quest Modification of the Basic RTClib.  The modification adds the 
 *   pcf85263 to the RTClib library....  Note: the full capabilities of the pcf85263
 *   is not implimented in this version 0.1. 
 *   20220627 pcf85263 based on the DS1307 templet - only the basic function 
 *   needed to read / write the RTC in completed the mofification was performed
 *   by hai all DS1307 lines are commented out......
 * *************************************************************************
 * *************************************************************************/
//
//
#include "Quest_RTClib.h"
//#define DS1307_CONTROL 0x07 ///< Control register
//#define DS1307_NVRAM 0x08   ///< Start of RAM registers - 56 bytes, 0x08 to 0x3f

/**************************************************************************/
/*!
    @brief  Start I2C for the DS1307 and test succesful connection
    @param  wireInstance pointer to the I2C bus
    @return True if Wire can find DS1307 or false otherwise.
*/
/**************************************************************************/
boolean RTC_pcf85263::begin(TwoWire *wireInstance) {
 // if (i2c_dev)
 //   delete i2c_dev;
//  i2c_dev = new Adafruit_I2CDevice(pcf85263_ADDRESS, wireInstance);
 // if (!i2c_dev->begin())
//    return false;
  return true;
}

/**************************************************************************/
/*!
    @brief  Is the pcf85263 forced running? Check the Clock Halt bit in register 0
    @return 1 if the RTC is running, 0 if not
*/
/**************************************************************************/
//uint8_t RTC_pcf85263::isrunning(void) { return 1; }

/**************************************************************************/
/*!
    @brief  Set the date and time in the pcf85263
    @param dt DateTime object containing the desired date/time
*/

/**************************************************************************/
/*
void RTC_pcf85263::adjust(const DateTime &dt) {
  uint8_t buffer[8] = {0,
                       bin2bcd(dt.second()),
                       bin2bcd(dt.minute()),
                       bin2bcd(dt.hour()),
                       0,
                       bin2bcd(dt.day()),
                       bin2bcd(dt.month()),
                       bin2bcd(dt.year() - 2000U)};
  i2c_dev->write(buffer, 8);
}
*/
/**************************************************************************/
/*!
    @brief  Get the current date and time from the pcf85263
    @return DateTime object containing the current date and time
*/
/**************************************************************************/
//byte bcdToDec(byte value){return ((value / 16) * 10 + value % 16);}//used in time
//byte decToBcd(byte value){return (value / 10 * 16 + value % 10);}//used in time

#define pcf85263_ADDRESS 0x51 ///< I2C address for pcf85263

DateTime RTC_pcf85263::now() {
  uint8_t buffer[7];
  buffer[0] = 0;
 // i2c_dev->write_then_read(buffer, 1, buffer, 7);
    Wire.begin();                   //begin if not started already
   Wire.beginTransmission(pcf85263_ADDRESS);
   Wire.write(0x01);
   Wire.endTransmission();
   Wire.requestFrom(pcf85263_ADDRESS, 7);
   buffer[1]  = (Wire.read() & B01111111); // sec remove VL error bit
   buffer[2]  = (Wire.read() & B01111111); // minute remove unwanted bits from MSB
   buffer[3]  = (Wire.read() & B00111111); // hour
   buffer[4]  = 0;                                 // 0 place holder
   buffer[5]  = (Wire.read() & B00111111); // day of month
   buffer[6]  = (Wire.read() & B00000000); // thow away day of week
   buffer[6]  = (Wire.read() & B00011111); // month remove century bit, 1999 is over
   buffer[7]  = (Wire.read());             // year
   
/*  for test
    Serial.println();
    Serial.println(pcf85263_ADDRESS, HEX);
    Serial.println(buffer[7], HEX);
    Serial.println(buffer[6], HEX);
    Serial.println(buffer[5], HEX);
    Serial.println(buffer[4], HEX);
    Serial.println(buffer[3], HEX);
    Serial.println(buffer[2], HEX);
    Serial.println(buffer[1], HEX);
    Serial.println(buffer[0], HEX);
    Serial.println();
*/


 return DateTime(bcd2bin(buffer[7]) + 2000U, bcd2bin(buffer[6]),
                  bcd2bin(buffer[5]), bcd2bin(buffer[3]), bcd2bin(buffer[2]),
                  bcd2bin(buffer[1] & 0x7F));
 /*
  return DateTime(bcd2bin(buffer[6]) + 2000U, bcd2bin(buffer[5]),
                  bcd2bin(buffer[4]), bcd2bin(buffer[2]), bcd2bin(buffer[1]),
                  bcd2bin(buffer[0] & 0x7F));
*/
}



/**************************************************************************/
/*!
    @brief  Read the current mode of the SQW pin
    @return Mode as Ds1307SqwPinMode enum
*/
/**************************************************************************/
//Ds1307SqwPinMode RTC_DS1307::readSqwPinMode() {
//  return static_cast<Ds1307SqwPinMode>(read_register(DS1307_CONTROL) & 0x93);
//}

/**************************************************************************/
/*!
    @brief  Change the SQW pin mode
    @param mode The mode to use
*/
/**************************************************************************/
//void RTC_DS1307::writeSqwPinMode(Ds1307SqwPinMode mode) {
//  write_register(DS1307_CONTROL, mode);
//}

/**************************************************************************/
/*!
    @brief  Read data from the DS1307's NVRAM
    @param buf Pointer to a buffer to store the data - make sure it's large
   enough to hold size bytes
    @param size Number of bytes to read
    @param address Starting NVRAM address, from 0 to 55
*/
/**************************************************************************/
//void RTC_DS1307::readnvram(uint8_t *buf, uint8_t size, uint8_t address) {
  //uint8_t addrByte = DS1307_NVRAM + address;
  //i2c_dev->write_then_read(&addrByte, 1, buf, size);
//}

/**************************************************************************/
/*!
    @brief  Write data to the DS1307 NVRAM
    @param address Starting NVRAM address, from 0 to 55
    @param buf Pointer to buffer containing the data to write
    @param size Number of bytes in buf to write to NVRAM
*/
/**************************************************************************/
//void RTC_DS1307::writenvram(uint8_t address, const uint8_t *buf, uint8_t size) {
 // uint8_t addrByte = DS1307_NVRAM + address;
 // i2c_dev->write(buf, size, true, &addrByte, 1);
//}

/**************************************************************************/
/*!
    @brief  Shortcut to read one byte from NVRAM
    @param address NVRAM address, 0 to 55
    @return The byte read from NVRAM
*/
/**************************************************************************/
//uint8_t RTC_DS1307::readnvram(uint8_t address) {
 // uint8_t data;
 // readnvram(&data, 1, address);
 // return data;
//}

/**************************************************************************/
/*!
    @brief  Shortcut to write one byte to NVRAM
    @param address NVRAM address, 0 to 55
    @param data One byte to write
*/
/**************************************************************************/
//void RTC_DS1307::writenvram(uint8_t address, uint8_t data) {
 // writenvram(address, &data, 1);
//}
