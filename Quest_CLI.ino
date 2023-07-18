/*******************************************************************************
   Quest institute command line interface and testing operations
   20220608 hai - created and begin consolidating different testing programs
   Added scani2c,
   20220609-13, added bme680,time
   20220613 settime,setPCF85263(),readPCF85263(),Datetime,dateTime,
   20220825 CLI_11 fixed line in so arduino ide monitor and serial terminal works with CR with every LF
            commands operationial
              ?  led  exit  scani2c  bme680 time  settime  dir  mkdir  rmdir open  erase  rtcreg  DotStar  dump
              sphoto io  ana
            next command upload flag in CLI_12
  20220831 CLI_12 added takeSphoto,  stackandheap,  initQueue to set upload flag response from microlab to upload buffer 0,1,2 also
        implimented "text" command to output text file in test...which is part of the upload information
  20220906 CLI_13 finished addding text screen
  20220906 CLI_14
  20220916 CLI_14 finished view commmand to view photos in terminal,
  20221011 CLI_17 different serial res photos, made and fixed text output with photo, created user text area, made text file
        photo file with PN (photo number), Text timing, timed charator sd writing time open,write,close 8.7ms, most time open close
        csv? did tdump command
  20230329 CLI_20 Stable test program with start of flight
  20230329 CLI_21 Add SD format(sd card) and free(space on disk free) stack and heap output, on format reset system
                  and init the system files on the SD card
  20230403 CLI_22 adding camspi working
  20230416 CLI_23 some clenup added enclude Quest_fram library
  20230503 CLI_24 re arrange working but with transfer time-outs!!!
  20230511 CLI_25 Begin to redo file transfers use output file buffer so many photos can ce stacked up not just 3
                  and ready to send....
                  added  attachInterrupt(digitalPinToInterrup- to sphoto returns
                  added Quest_test, working on timeouts??
   20230517 CLI_26 Changed FmHostRequest input structure from case to direct if statements, this eleminated false FmHostRequests if in error
                  if softuart detects error then FmHostRequest is not processed. if new FmHostRequests need to be added then new if is needed
                  changed host commands to FmHostRequest....Fix for timeout errors in normal expect times...
  20230522  CLI_27-CLI_28..break in time to Make boards.  changing Mission clock to binary counter
  20230626  CLI_29 todo: Queue, takeSpiphoto, text buffer, photo numbers, add mission clk to text.
                  added mission clock to text, added file name and numberto Spi photo also output tex with spi
                  added Queue for both serial and spi
  20230701  CLI_30 function point, need to clean up a lot of stuff....  testing flight...
  20230705  CLI_31 to fix spi photo out of Queue..fixed fram ID and check valid, added enterTemaID command
                  Run flight with Serial and SPI cameras.
  20230710  CLI_32 repaired Timeout by removing Logit next to text out, irq looks good now, eremoved multiple write to
                  user_text buffer text size same now.. Need to clean up code for humans..

 * *****************************************************************************
*/
//
int Qversion = 20230715;  //Program version
const char compile_date[] = __DATE__ " " __TIME__;
const char source_file[] = __FILE__;

//-----includes--------------------
#include <Wire.h>
#include <i2cdetect.h>
#include <SdFat.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <Quest_DotStar.h>
#include "Quest_CameraC329.h"
#include "QuestCameraC329SPI.h"
#include "Quest_fram.h"         //20230503 hai
#include "Quest_RTClib.h"
#include "Quest_Que.h"          //20230517 hai
#include "Quest_test.h"
//
#include "Quest_command.h"
#include "Quest_CLI.h"
#include "Quest_Flight.h"
//
//-----------------------------------------------
//

//
//--------------------------------System status flags
bool CmdTestOn = true;
bool testing = false;            //set when testing command line processing is active
bool flight = false;             //set flight program to false
//boolean toggle = false;          //flip on off
//
//
//-------------------------------General Setup pins
const int Cam_power = 7;          //Enable power to Serial camera
const int Cam_power_ON = 1;       //Power on to C329 Uart camera
const int Cam_power_OFF = 0;      //Power off to C329 Uart camera
//
const int B_MicroSD_CS = A5;      //sd card chip select
//
const int Sel0 = A4;              //serial select 0
const int Sel1 = 3;               //serial select 1
//
const int ANA0 = A0;              //analog 0 input
const int ANA1 = A1;              //analog 1 input
const int ANA2 = A2;              //analog 2 input
const int ANA3 = A3;              //analog 3 input
//
//-------------------------------end of general setup pins

#define IO7 (13)              // Input/Output to payload plus 
#define IO6 (12)              // Input/Output to payload plus 
#define IO5 (11)              // Input/Output to payload plus
#define IO4 (10)              // Input/Output to payload plus
#define IO3 (9)               // Input/Output to payload plus
#define IO2 (4)               // Input/Output to payload plus
#define IO1 (3)               // Input/Output to payload plus
#define IO0 (A6)              // Input/Output to payload plus 
//
//-------Host interface communication 9600 baud ----------

const byte EIC_IRQn = 0;
//const byte SerialIRQin = 0;  //  = 0 with your interrupt pin number
#define SerialIRQin  (0)        //(IO3)      //serial1 rx input used for interrupt
#define softuartrate (104)            //software loop  
#define softuartinpin (0)             //(0) //
#define softuartoutpin (1)            //(1) //

SdFat SD;
File32 file;
File32 root;
File Logfile;

//--------- define values for BME680
#define bme680address (0x76)            // BME680 address
Adafruit_BME680 bme;                    // alias
#define SEALEVELPRESSURE_HPA (1013.25)  //reference sea level pressure
//--------end of BME680 values
//
//--------------------------------C329 Serial Camera values
#define CAMERA_BAUD 14400
Quest_CameraC329 camera;
File photoFile;
File TextFile;
uint32_t writtenPictureSize;
//
//-------------------------------- SPI camera info
QuestCameraC329SPI cameraSPI(5);              //SPI_cam_CS)
//
//--------------------------------define values for pcf85263
#define PCF85263address 0x51                  //define I2C address
RTC_pcf85263 rtc;
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
const String days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
//
//-------------------------------- Terminal input information
#define LINE_BUF_SIZE 128   //Maximum input string length
#define ARG_BUF_SIZE 64     //Maximum argument string length
#define MAX_NUM_ARGS 8      //Maximum number of arguments
char line[LINE_BUF_SIZE];
char args[MAX_NUM_ARGS][ARG_BUF_SIZE];
//
//----------- System Stroage ------------------------------------
//
byte Status = 0x20;            //MicroLab Status Byte
#define Bank0status  (0x01)   //data in bank 0
//#define Bank1status  (0x02)   //data in bank 1
//#define Bank2status  (0x04)   //data in bank 2
//int Bankcount = 0x00;         //Storage bank counter 0,1,2,3,4,5,6,7
uint8_t FileReadError = 0;     //0=no error 1= error opening file
uint8_t Abortphoto = 0;      //jpg photo abort error snyc
uint32_t currentunix = 0;     //placeholder for current unix
//--------------------------------------------------------------
//    File naming templets, file Queue holds Host output format.
//
//char FileHeader[] = {'X','T','4','3','2','1','0','P',0x04,0x80};  //header string to send to host
// X = Team ID, T = photo file source, sequencial number for files, P = placeholder for future, size - lsb msb
//
char filenameS[] = "XS00000PSS";        //create filename templet for name serial, SS = size of file
char filenameP[] = "XP00000PSS";        //create filename templet for name SPI, SS = size of file
char filenameD[] = "XD00000PSS";        //create filename templet for Data only- file, SS = size of file
const char* Quefile[15];
//
#define USER_TEXT_BUF_SIZE   1024                               //Text buffer 0
char user_text_buf0 [USER_TEXT_BUF_SIZE] = "USER TEXT\r\n";     //Test Buffer 0
//
//
//  Fram System information file setup
//
//BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
//  Print output text information that is sent with photo
//    char text_buf0 [TEXT_BUF_SIZE];           //Test Buffer 0
//
//     TEXT Buffer for output and file storage
//                       01234567890123456789012345678901234567890123456789
char  text_header[]   = "\r\n*****  S3_04 System testing program  *****";
char  text_DataDown[] = "Data DownLoad number = XXxxxxx";
char  text_time[]     = "Date/Time = xx/xx/xx - xx:xx:xx";
char  text_Unix[]     = "Unix Time = Seconds since 1970: xxxxxxxxxx ";
char  text_Mission[]  = "Mission Clock =                     ";
char  text_Atemp[]    = "Ambient Temperature = xx.xx C xxx.xx F";
char  text_humidity[] = "Ambient Humidity = xx.xx %";
char  text_gas[]      = "Gas = xxx.xx Kohms";
char  text_pressure[] = "Pressure = xxxx.xx hPa";
char  text_altitude[] = "Pressure Altitude = 12345.67 meters";
char  text_Ain0[]     = "Analog input 0 = x.xx Volts";
char  text_Ain1[]     = "Analog input 1 = x.xx Volts";
char  text_Ain2[]     = "Analog input 2 = x.xx Volts";
char  text_Ain3[]     = "Analog input 3 = x.xx Volts";
char  text_buf95[] = "Source File = ";
char  text_buf96[] = "Compiled Date = ";
char  text_buf97[] = "Quest Version = ";
//
char  text_buf[50] = "--------------------";                //temp buffer for conversion
//
char* textlist[] = {text_header, text_DataDown, text_time, text_Unix, text_Mission, text_Atemp, text_humidity, text_gas,
                    text_pressure, text_altitude, text_Ain0, text_Ain1, text_Ain2, text_Ain3, text_buf95, text_buf96, text_buf97
                   };
//
char ascii[5];  // Array to store the ASCII representation (maximum 5 characters 99,999)
//
//----------  Bank buffers arrarays  -----------------------------

char FileID[] = {'H', 'I'};                                           //MicroLab "ID" for file idenification

char Bank0filename[] = {FileID[0], FileID[1], '5', '4', '3', '2', '1', '.', 'j', 'p', 'g'}; //name of file in B0
int Bank0size = 0;

char FileHeader[] = {'X', 'Y', '4', '3', '2', '1', '0', 'P', 0x04, 0x80}; //header string
//
char PN[8] = {"1234567"};   //converted photonumber into ascii to send to master controller for file name
//
//---------------- Temp number for test ------------------
// Note: download number starts at 1000000 to reserve 7 places in the ID/photonumber array
//   for a total of 7 charators, printing and outputing of array only outputs siginifite entries
//   this is a cludge and work around for now, must ba a better way....!!
//
int download_number = 1000000;
uint32_t photonumber = download_number; //photo number, will count up
int photolength = 1030;                 //photo jpg length
int photobytecount = 0;                 //for loop count of transfer
int  photodata = 0;
//
//---------------  Mission time stuff -------------------------------
uint32_t MissionUtcLast = 0;              //Last Mission time
uint32_t missionMillis = 0;               //track time since last time fired
uint32_t testtimeout = 15000;             //test time out to go to flight
//
//--------------------------------
uint16_t LEDpin = 13;
uint16_t blink_cycles = 10;               //How many times the LED will blink
bool error_flag = false;

uint32_t looptimebase = 0;               //loop heartbeat time
#define loopcount     (200)              //for testing main loop heartbeat time
uint16_t looper = 0;                     //count within loopcount
uint32_t IRQreference = 0; //
uint16_t IRQinvalid = 0;                  //set for the next incomming irq to be invalid
//-------------------------------
//  Mission times
uint8_t xd = 0;
uint8_t xh = 0;
uint8_t xm = 0;
uint8_t xs = 0;

//-------------------------------
String str;                             //universal string statement

uint16_t Tdiff = 0;                     //Mission time different

//-----------------------function prototypes
void listQue();

void help_ana();
void help_io();
void help_upload();
void help_takeSphoto();
void help_stackandheap();
void help_initQueue();
void help_text();
void help_view();
void help_tdump();
void help_info();
void help_sphoto();
void help_dump();
void help_DotStar();
void help_rtcreg();
void help_erase();
void help_open();
void help_rmdir();
void help_mkdir();
void help_dir();
void help_time();
void help_bme680();
int  initSD();
void Hostinterupt();
void softuartwrite(uint8_t x);
void send_T_host(uint8_t x);
void my_cli();
int takeSPI(void);      //-------------------------------------
//
//
/*
  ///////////////////////////////////////////////////////////////////////
  //----------- for testing for timming with a logic analizer --------
  //    located in Quest_test.h
  //
  void Pulse13(void);    //pulse pin 13 / IO7 one cpu cycle time
  void Pulse12(void);   //pulse 12 / IO6
  void Pulse11(void);   //pulse 11 / IO5
  void Pulse10(void);   //pulse 10 / IO4
  void Pulse9(void);    //pulse  9 / IO3
  void Pulse4(void);    //pulse  4 / IO2
  //
  void Pulse13high(void); //make pin 13 high
  void Pulse13low(void);  //make pin 13 low
  void Pulse12high(void); //make pin 12 high
  void Pulse12low(void);  //make pin 12 low
  void Pulse11high(void); //make pin 11 high
  void Pulse11low(void);  //make pin 11 low
  void Pulse10high(void); //make pin 10 high
  void Pulse10low(void);  //make pin 10 low
  void Pulse9high(void);  //make pin 9 high
  void Pulse9low(void); //make pin 9 low
  void Pulse4high(void);  //make pin 4 high
  void Pulse4low(void); //make pin 4 low
  //
*/
//
/*
  //
  //Function declarations
  void cli_init();
  int cmd_help();
  int cmd_led();
  int cmd_exit();
  int cmd_scani2c();
  int cmd_bme680();
  int cmd_time();
  int cmd_settime();
  int cmd_dir();
  int cmd_mkdir();
  int cmd_rmdir();
  int cmd_open();
  int cmd_erase();
  int cmd_rtcreg();
  int cmd_DotStar();        //13
  int cmd_dump();           //14
  int cmd_sphoto();         //15
  int cmd_upload();         //16
  int cmd_io();             //17
  int cmd_ana();            //18
  int cmd_takeSphoto();         //19
  int cmd_stackandheap();   //20
  int cmd_ initQueue();         //21
  int cmd_text();           //22
  int cmd_view();           //23
  int cmd_tdump();          //24
  int cmd_tc();             //25  test mission clock
  int cmd_trtc();           //26  test real time clock
  int cmd_format();         //27  format SD
  int cmd_free();           //28  free space on SD
  int cmd_takeSpiphoto();         //29  for testing SPI Camera
  int framdump();           //30  dump total contents of fram
  int initfram();           //31  Default setup for Fram memory
  int framclear();          //32  Set all of fram to "0"
  int SystemSet();          //33

  void setPCF85263();
  void readPCF85263();
  void help_led();
  void help_exit();
  void help_help();
  void help_scani2c();
  void help_settime();
*/
//List of functions pointers corresponding to each command
int (*commands_func[])() {
  &cmd_help,
  &cmd_led,
  &cmd_exit,
  &cmd_scani2c,
  &cmd_bme680,
  &cmd_time,
  &cmd_settime,
  &cmd_dir,
  &cmd_mkdir,
  &cmd_rmdir,
  &cmd_open,
  &cmd_erase,
  &cmd_rtcreg,
  &cmd_DotStar,
  &cmd_dump,
  &cmd_sphoto,
  &cmd_upload,
  &cmd_io,
  &cmd_ana,
  &cmd_takeSphoto,       //19
  &cmd_stackandheap,     //20
  &cmd_initQueue,        //21
  &cmd_text,             //22
  &cmd_view,             //23
  &cmd_tdump,            //24
  &cmd_info,             //25  
  &cmd_trtc,             //26
  &cmd_format,           //27
  &cmd_free,             //28
  &cmd_takeSpiphoto,     //29
  &framdump,             //30
  &cmd_initfram,          //31
  &framclear,            //32
  &SystemSetup,           //33
  &ReadSetup,             //34
  &cmd_listQue,           //35
  &cmd_enterTeamID        //36
};

//List of command names
const char *commands_str[] = {
  "?",
  "led",
  "exit",
  "scani2c",
  "bme680",
  "time",
  "settime",
  "dir",
  "mkdir",
  "rmdir",
  "open",
  "erase",
  "rtcreg",
  "DotStar",
  "dump",
  "sphoto",
  "upload",
  "io",
  "ana",
  "takeSphoto",           //19
  "stackandheap",         //20
  "initQueue",         //21
  "text",           //22
  "view",           //23
  "tdump",          //24
  "info",             //25
  "trtc",           //26
  "format",         //27
  "free",           //28
  "takeSpiphoto",         //29
  "framdump",       //30
  "initfram",       //31
  "framclear",      //32
  "SystemSetup",     //33
  "ReadSetup",       //34
  "listQue",         //35
  "enterTeamID"      //36
};

//List of LED sub command names
const char *led_args[] = {
  "on",
  "off",
  "blink"
};
//List of DotStar sub command names
const char *DotStar_args[] = {
  "Off",
  "Red",
  "Green",
  "Yellow",
  "Blue",
  "Magenta",
  "Cyan",
  "White"
};
//
//-------------------------------------------- for command line processor
//
int num_commands = sizeof(commands_str) / sizeof(char *);
//
//-------------------------------------------- end command line processor

//-----------------------------------------------------------------
//tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt
//-----------------------------------------------------------------
//
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
//
int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
//----------------------------------------------------------------
//----------------------------------------------------------------
//----  Send one charator to logfile ----  open+write+close
//      approx = 8.03ms in time
//---------------------------------------------------------------
void logit(uint8_t  x) {
  Logfile = SD.open("syslog.txt", FILE_WRITE);  //open log file ?
  if (Logfile) {                                //with logfile is open
    Logfile.write(x);                         //write the value to file
  }
  else {                                        //if not open say error
    Serial.println("\r\nlogit error");          //Terminal output here
  }
  Logfile.close();                              //close the log file
}
//---------------------------------------------------------------
//----  Send string to logfile ----  open+write string+close
//      approx =  in time
//---------------------------------------------------------------
void logit_string() {                           //store string
  Logfile = SD.open("syslog.txt", FILE_WRITE);  //open syslog file
  if (Logfile) {                                //can open the file
    Logfile.println();                          //add a carrage return/line feed
    delayMicroseconds(100);                     //wait 100 microsec
    for (uint8_t x = 0x20; x < 128; x++) {      //print a string to log file
      Logfile.write(x);                         //write one charator at a time
    }                                           //close string
  }                                             //close the open log file
  else {                                        //or else
    Serial.println("\r\nlogit error");          //error can not open log file
  }                                            //close error else
  Logfile.close();                             //close the log file
}                                               //
//
//
//
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//
// get current mission clock and put it into text_Mission values
//
uint16_t getmissionclk() {
  uint32_t cumunix = readlongFromfram(CumUnix);       //Get cumulative unix mission clock
  cumunix = readlongFromfram(CumUnix);          //Get cumulative
  xs = cumunix % 60; cumunix /= 60;    //get sec and /60 for min
  xm = cumunix % 60; cumunix /= 60;    //get min and /60 for hours
  xh = cumunix % 24; cumunix /= 24;    //get hours and /24 for days
  xd = cumunix;                        //get days

  itoa(xd, ascii, 10);            //convert day and place days in text_mission
  int z = 0;                     //counter of valid entries in array
  for (int i = 0; ascii[i] != '\0'; i++) {
    z++;
  };  //find firt null
  uint16_t  y = 21 - z;         //where to start in mission buffer
  for (uint16_t x = 0 ; x < z; x++) { //how many charators
    text_Mission[y] = ascii[x]; //transfer from aacii array to proper location in tex_mission
    y++;                        //pointer to text_mission array
  }
  text_Mission[21] = 'D';       //done say D for day
  //
  itoa(xh, ascii, 10);
  z = 0;
  for (int i = 0; ascii[i] != '\0'; i++) {
    z++;
  };
  y = 25 - z;         //where to start in mission buffer
  for (uint16_t x = 0 ; x < z; x++) {
    text_Mission[y] = ascii[x];
    y++;
  }
  text_Mission[25] = 'H';
  //
  itoa(xm, ascii, 10);
  z = 0;
  for (int i = 0; ascii[i] != '\0'; i++) {
    z++;
  };
  y = 29 - z;         //where to start in mission buffer
  for (uint16_t x = 0 ; x < z; x++) {
    text_Mission[y] = ascii[x];
    y++;
  }
  text_Mission[29] = 'M';
  //
  itoa(xs, ascii, 10);
  z = 0;
  for (int i = 0; ascii[i] != '\0'; i++) {
    z++;
  };
  y = 33 - z;         //where to start in mission buffer
  for (uint16_t x = 0 ; x < z; x++) {
    text_Mission[y] = ascii[x];
    y++;
  }
  text_Mission[33] = 'S';
  return 0;
}
//
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//   write to Host and Output to Terminal
//
void send_T_host(uint8_t x) {     //
  softuartwrite(x);             //send to Host
  Serial.write(x);              //send to terminal
}
//
//----------------------------------------------------------------------------
//      Call back for file timestamps.  Only called for file create and sync().
//----------------------------------------------------------------------------
void dateTime(uint16_t* date, uint16_t* time, uint8_t* ms10) {
  DateTime now = rtc.now();
  *date = FS_DATE(now.year(), now.month(), now.day());        // Return date using FS_DATE macro to format fields.
  *time = FS_TIME(now.hour(), now.minute(), now.second());    // Return time using FS_TIME macro to format fields.
  *ms10 = now.second() & 1 ? 100 : 0;                 // Return low time bits in units of 10 ms, 0 <= ms10 <= 199.
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//        Software reset function
//
void (*resetFunc)() = 0;  //point resetFunction to address 0 reset vector
//
//Software reset ARM
void SoftwareReset(void) {
  Serial.print("\r\n\n*** System Reseting - Must restart serial terminal ***\r\n\n");
  delay(2000);
  // generic way to request a reset
  //from software for ARM Cortex
  SCB->AIRCR = ((0x5FA << SCB_AIRCR_VECTKEY_Pos) |
                SCB_AIRCR_SYSRESETREQ_Msk);
  for (;;) {}     // // wait until reset will loop forever waiting for reset action
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  //    while(!Serial);       //wait for serial to be ready
  //
  delay(4000);        // Wait for usb if used and attached
  //
  //
  Serial.println("\r\n\n----------------------------------------------------------");
  Serial.println("\nSystem Startup - S3_40 Testing Command line interface 20220710");
  Serial.println("interface not complete, look for future cleaner updates, Thank you!");
  Serial.println("add to change this is your playground to learn the S3_40 system\n");
  //
  //------------ set digital IO to output and high ------------
  pinMode(IO7, OUTPUT);      //
  digitalWrite(IO7, HIGH);   //
  pinMode(IO6, OUTPUT);      //
  digitalWrite(IO6, HIGH);   //
  pinMode(IO5, OUTPUT);      //
  digitalWrite(IO5, HIGH);   //
  pinMode(IO4, OUTPUT);      //
  digitalWrite(IO4, HIGH);   //
  pinMode(IO3, OUTPUT);      //
  digitalWrite(IO3, HIGH);   //
  pinMode(IO2, OUTPUT);      //
  digitalWrite(IO2, HIGH);   //
  pinMode(IO1, OUTPUT);      //
  digitalWrite(IO1, HIGH);   //
  pinMode(IO0, OUTPUT);      //
  digitalWrite(IO0, HIGH);   //
  //
  PulseInit();    //????????????????????????????????????????????????????????????
  //
  //---  Default pin defination and direction
  //
  pinMode(LEDpin, OUTPUT);                    //IO7 and RED LED on PCB
  pinMode(B_MicroSD_CS, OUTPUT);              //MicroSD enable pin
  digitalWrite(B_MicroSD_CS, HIGH);           //MicroSD pin Disable
  pinMode(Cam_power, OUTPUT);                 //Serial and SPI camera power pin
  digitalWrite(Cam_power, Cam_power_OFF);     //Serial and SPI camera power off

  pinMode(Sel0, OUTPUT);                      //Selection serial camera or serial from host
  digitalWrite(Sel0, LOW);                    //default low connect serial to host

  pinMode (SPI_cam_CS, OUTPUT);                   //set spicamera CS pin. output
  digitalWrite(SPI_cam_CS, HIGH);
  //SPI_cam_Power = 7;                            //SPI camera power pin
  const int SPIcamON = 1;                         //Power on the SPI camera    Removed for test of quest init 0503
  const int SPIcamOFF = 0;                        //Power off the SPI camera
  pinMode( SPIcamHold, INPUT);                    //SPI camera Hold pin



  pinMode(softuartoutpin, OUTPUT);            //for Host interface
  digitalWrite(softuartoutpin, HIGH);         //for Host interface
  DotStarInit();                              //set up DotStar
  DotStarOff();                               //Set to Off
  Serial.println("DotStar OK");
  //
  //---Set RTC quartz oscillator for proper load capacitance
  Wire.begin();
  Wire.beginTransmission(PCF85263address);
  Wire.write(byte (0x25));                            //Osc Register
  Wire.write(byte (0x03));                            //Set to 12.5pf
  Wire.endTransmission();                             //end xmission
  Serial.println("RTC xyal set");
  //
  //
  // initfram();                                 //check FRAM ??
  Serial.println("FRAM Inited");                //CLI_24
  //
  //
  //---  Wake up SD card
  if (!SD.begin(B_MicroSD_CS))
  {
    Serial.println("SD not present or initialization failed");
    while (!SD.begin(B_MicroSD_CS)) {
      Serial.println("Please install SD card");
      delay(2000);
    }
  }
  Serial.println("SD installed OK");
  initSD();                             //set up SD files and system files for operation
  //
  /*
    //----------------------------------------------------------------------
    //-------- Check for Cumulitive Mission Time file if no exist Create it
    //
    File CumMisClk = SD.open("CumMisClk.txt");   //name of file
    if (CumMisClk){
      Serial.println("CumMisClk file exist");
    }
    else{
      Serial.println("Made CumMisClk file");
      FsDateTime::setCallback(dateTime);            //set time and date for file
      File CumMisClk = SD.open("CumMisClk.txt", FILE_WRITE);
      str=String(1);
          Serial.println(str);
          str.toCharArray(text_buf,11);
          CumMisClk.print(text_buf);

      }
    CumMisClk.close();
    //
    Serial.println("CumMisClk.tct written");
  */
  //----------------------------------------------------------------------
  //
  //
  //----------- System test flages ---------------------------------------
  //
  testing = false;    //reset active command line processing flag
  flight = false;     //reset flight active flag

  IRQreference = millis();          //capture millis start of interrupt(millis not availiable in IRQ)
  attachInterrupt(digitalPinToInterrupt(SerialIRQin), Hostinterupt, FALLING);
  Serial.println("Interrupts enabled ");
}
//
//-----------------------------------------------------------------------------------
//
void loop() {

  Serial.print("\nSystem Boot Version ");
  Serial.println(Qversion);
  Serial.print("Compiled Date => "); Serial.println(compile_date);
  Serial.print("Source file => "); Serial.println(source_file);
  Serial.println();
  Serial.print("Free Memory = "); Serial.print(freeMemory(), HEX);
  Serial.print(" HEX or "); Serial.print(freeMemory()); Serial.println(" DEC");
  cmd_stackandheap();
  //

  Serial.println("\r\n\nInput 'T' to enter test within 15 seconds");
  //
  uint32_t currentMillis = millis();          //this is time now

  //??????????????????????????????????????????????????????????????????????????????????????????????????????
  //   test for Flying();                           //Set Flying active and go to flight programming
  //??????????????????????????????????????????????????????????????????????????????????????????????????????
  //
  testing = false;                          //default no testing
  while ((millis() - currentMillis) < testtimeout) {
    if (Serial.available()) {
      char inbyte = Serial.read();              //get the input charator
      if (inbyte == 'T') {                      //if it is a T go to testing
        testing = true;                         //set testing flag to true
        my_cli();                               //execute test command line processor
        SoftwareReset();
      }
    }
  }
  if (testing == true) {
    SoftwareReset();
  }
  Flying();                 //Set Flying active and go to flight programming
  SoftwareReset();          //here when aborted from Flying
}


//=========================================================================================
//
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

void cli_init() {
  Serial.println("\n\n*****************************************");
  Serial.println("Quest Institute command line interface");
  Serial.println("  \"?\" = list of commands.");
}

//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

void my_cli() {
  cli_init();
  while (testing == true) {
    //     stackandheap();             //output stack and heap
    Serial.print("=>");
    read_line();
    //      Serial.println();   //
    if (!error_flag) {

      parse_line();
    }
    if (!error_flag) {
      execute();
    }
    memset(line, 0, LINE_BUF_SIZE);
    memset(args, 0, sizeof(args[0][0]) * MAX_NUM_ARGS * ARG_BUF_SIZE);
    error_flag = false;
  }
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

void read_line() {
  String line_string;
  char inbyte = 0;

  while (inbyte != ('\r')) {
    //Serial.print("*");
    // Here waiting for an input from terminal, no input blink DotStar LED
    while (!Serial.available()) {                                     //Waiting for iput Blink led for CLI Heart Beat
      if (millis() - looptimebase > loopcount) {                     //loop heartbeat timer
        looptimebase = millis();                                    //reset time
        looper++;                                                   //Routine to make the DotStar blink to indicate in CLI Mode
        if (looper == 1) {
          DotStarBlue(); //on 10%
        }//digitalWrite(LEDpin,HIGH);}
        if (looper == 2) {
          DotStarOff(); //off 10%
        }//digitalWrite(LEDpin,LOW);}
        if (looper == 3) {
          DotStarBlue(); //on 10%
        }//digitalWrite(LEDpin,HIGH);}
        if (looper == 4) {
          DotStarOff(); //off 70%
        }//digitalWrite(LEDpin,LOW);}
        if (looper > 9) {
          looper = 0;
        }
      }
    }
    //Here to get input from terminal, check for end of line then prac the line
    if (Serial.available()) {
      inbyte = Serial.read();          //fetch the input byte
      if (inbyte == ('\r')) {
        line_string = line_string + (" ");
        if (line_string.length() < LINE_BUF_SIZE) {            //check for oversize buffer input
          line_string.toCharArray(line, LINE_BUF_SIZE);       //convert string to char array
        }
        else {
          Serial.println("Input string too long.");
          error_flag = true;
        }
        Serial.print('\n');
      }
      line_string = line_string + inbyte;   //add inbyte to string
      Serial.write(inbyte);
    }
  }
}

//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

void parse_line() {
  char *argument;
  int counter = 0;

  argument = strtok(line, " ");

  while ((argument != NULL)) {


    if (counter < MAX_NUM_ARGS) {
      if (strlen(argument) < ARG_BUF_SIZE) {
        strcpy(args[counter], argument);
        argument = strtok(NULL, " ");     ///////
        counter++;
      }
      else {
        Serial.println("Input string too long.");
        error_flag = true;
        break;
      }
    }
    else {
      break;
    }
  }
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

int execute() {
  for (int i = 0; i < num_commands; i++) {
    if (strcmp(args[0], commands_str[i]) == 0) {
      return (*commands_func[i])();
      //Serial.print(args[0]);       //???????????????????????????????????????????????
    }
  }

  Serial.println("Invalid command. Type \"?\" for more!!!.");
  return 0;
}

//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

int cmd_help() {
  if (args[1] == NULL) {
    help_help();
  }
  else if (strcmp(args[1], commands_str[0]) == 0) {
    help_help();
  }
  else if (strcmp(args[1], commands_str[1]) == 0) {
    help_led();
  }
  else if (strcmp(args[1], commands_str[2]) == 0) {
    help_exit();
  }
  else if (strcmp(args[1], commands_str[3]) == 0) {
    help_scani2c();
  }
  else if (strcmp(args[1], commands_str[4]) == 0) {
    help_bme680();
  }
  else if (strcmp(args[1], commands_str[5]) == 0) {
    help_time();
  }
  else if (strcmp(args[1], commands_str[6]) == 0) {
    help_settime();
  }
  else if (strcmp(args[1], commands_str[7]) == 0) {
    help_dir();
  }
  else if (strcmp(args[1], commands_str[8]) == 0) {
    help_mkdir();
  }
  else if (strcmp(args[1], commands_str[9]) == 0) {
    help_rmdir();
  }
  else if (strcmp(args[1], commands_str[10]) == 0) {
    help_open();
  }
  else if (strcmp(args[1], commands_str[11]) == 0) {
    help_erase();
  }
  else if (strcmp(args[1], commands_str[12]) == 0) {
    help_rtcreg();
  }
  else if (strcmp(args[1], commands_str[13]) == 0) { //DotStar
    help_DotStar();
  }
  else if (strcmp(args[1], commands_str[14]) == 0) { //dump
    help_dump();
  }
  else if (strcmp(args[1], commands_str[15]) == 0) { //sphoto
    help_sphoto();
  }
  else if (strcmp(args[1], commands_str[16]) == 0) { //upload
    help_upload();
  }
  else if (strcmp(args[1], commands_str[17]) == 0) { //io
    help_io();
  }
  else if (strcmp(args[1], commands_str[18]) == 0) { //ana
    help_ana();
  }
  else if (strcmp(args[1], commands_str[19]) == 0) { //takeSphoto
    help_takeSphoto();
  }
  else if (strcmp(args[1], commands_str[20]) == 0) { //stackandheap
    help_stackandheap();
  }
  else if (strcmp(args[1], commands_str[21]) == 0) { //initQueue
    help_initQueue();
  }
  else if (strcmp(args[1], commands_str[22]) == 0) { //text
    help_text();
  }
  else if (strcmp(args[1], commands_str[23]) == 0) { //view
    help_view();
  }
  else if (strcmp(args[1], commands_str[24]) == 0) { //tdump
    help_tdump();
  }
  else if (strcmp(args[1], commands_str[25]) == 0) { //info
    help_info();
  }
  else if (strcmp(args[1], commands_str[26]) == 0) { //trtc
    help_trtc();
  }
  else if (strcmp(args[1], commands_str[27]) == 0) { //format
    help_format();
  }
  else if (strcmp(args[1], commands_str[28]) == 0) { //free
    help_free();
  }
  else if (strcmp(args[1], commands_str[29]) == 0) { //takeSpiphoto
    help_takeSpiphoto();
  }
  else if (strcmp(args[1], commands_str[30]) == 0) { //framdump
    help_framdump();
  }
  else if (strcmp(args[1], commands_str[31]) == 0) { //initfram
    help_initfram();
  }
  else if (strcmp(args[1], commands_str[32]) == 0) { //framclear
    help_framclear();
  }
  else if (strcmp(args[1], commands_str[33]) == 0) { //SystemSetup
    help_SystemSetup();
  }
  else if (strcmp(args[1], commands_str[34]) == 0) { //ReadSetup
    help_ReadSetup();
  }
  else if (strcmp(args[1], commands_str[35]) == 0) { //listQue
    help_listQue();
  }
  else if (strcmp(args[1], commands_str[36]) == 0) { //enterTeamID
    help_enterTeamID();
  }
  else {
    help_help();
  }
  return 0;
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

void help_help() {
  char string_buffer[80];
  Serial.println("The following commands are available:");

  for (uint16_t i = 0; i < num_commands; i++) {

    snprintf(string_buffer, 80, "%-15s", commands_str[i]);
    Serial.print(string_buffer);
    
    if (i % 6 == 0) {
      Serial.println();
    }
  }
  Serial.println("\n\nType \"? led\" for more info on the LED command.\n\r");
}

void help_led() {
  Serial.print("Control the on-board LED, either on, off or blinking ");  //help for led
  Serial.print(blink_cycles);
  Serial.println(" times:");
  Serial.println("  led on");
  Serial.println("  led off");
  Serial.println("  led blink hz");
  Serial.println("    where \"hz\" is the blink frequency in Hz.");
}
void help_DotStar() {
  Serial.println("Control the on-board DotStar LED");           //help for DotStar
  Serial.println("DotStar Off => Turn DotStar Off");
  Serial.println("DotStar Red => Make DotStar Red");
  Serial.println("DotStar Green => Make DotStar Green");
  Serial.println("DotStar Yellow => Make DotStar Yellow");
  Serial.println("DotStar Blue => Make DotStar Blue");
  Serial.println("DotStar Magenta => Make DotStar Magenta");
  Serial.println("DotStar Cyan => Make DotStar Cyan");
  Serial.println("DotStar White => Make DotStar White");
}

void help_exit() {
  Serial.println("This command will exit the Command Line Interface (CLI) and do a reset.");
  Serial.println("Then enter a 'T' within 15 seconds to enter Test of the CLI interface again");
  Serial.println("or do nothing and the program will automatically enter flight Mode"); 
}
void help_scani2c() {
  Serial.println("Will scan I2C addresses and indicate which address is alive");
}
void  help_settime() {
  Serial.println("\nEnter \"settime YYMMDDHHMMSS\" where:");
  Serial.println("  YY is year, MM is month, DD is day, HH is hour(24), MM is minute, SS is seconds");
}
void help_bme680() {
  Serial.println("\nOutputs of temperature, humidity, pressure, and gas sensor readings");
}
void help_time() {
  Serial.println("\nOutputs now time and unix time");
}
void help_dir() {
  Serial.println("\nOutputs SD card directory and files");
}
void help_mkdir() {
  Serial.println("\nTo make a directory or folder \"mkdir foldername\"");
}
void help_rmdir() {
  Serial.println("\nRemoves a directory or folder \"rmdir foldernamd\"");
}
void help_open() {
  Serial.println("\nCreate a file within a folder/directory \"open filename\"");
}
void help_erase() {
  Serial.println("\nTo remove/delete a file \"erase filename\"");
}
void help_rtcreg() {
  Serial.println("\n list all the registors in the PCF85263 Real Time Clock");                                                                                                     
}
void help_dump() {
  Serial.println("Will dump contents of an SD file to the terminal in HEX");
  Serial.println("Example =>dump filename.ext");
}
void help_sphoto() {
  Serial.println("Take a photo and store it on SD card");
  Serial.println("Example =>sphoto filename.jpg");
}
void help_upload() {
  Serial.println("Upload a file from SD card to the Host controller");
  Serial.println("Example =>upload filename");
  Serial.println("will find file on SD card, upload a jpg with the file name");
  Serial.println("and a text file with the same file name");
}
void help_io() {
  Serial.println("io \"IOnumber\" \"H\" or \"L\"");
  Serial.println("\"Example =>io 7 L\"  will cause IO7 to go Low");
}
void help_ana() {
  Serial.println("Sample A0 through A3 analog input");
  Serial.println("print the voltage values on earch input");
}
void help_takeSphoto() {
  Serial.println("Take a photo using the Serial C329 camera");
  Serial.println("Place the photo header in the output Queue");
  Serial.println("the photo will be picked up and sent to the Host");
  Serial.println("during the next request for data from the Host");
}
void help_stackandheap() {
  Serial.println("will printout both stack pointer ahd heap pointer ");
}
void help_initQueue() {
  Serial.println("Setup the Photo Queue's space and pointers");
  Serial.println("Set to empty Queue");
}
void help_text() {
  Serial.println("Create and Output to the terminial the Text message");
  Serial.println("that will be appended to the photo putput");
}
void help_view() {
  Serial.println("Must use the python terminal, it will take a serial  photo");
  Serial.println("place the photo on the SD card, then will output the jpg image");
  Serial.println("in HEX code to the terminal, the terminial will then take the HEX");
  Serial.println("code into a jpg file in a folder on the terminal computer. The jpg file");
  Serial.println("can then be clicked on to open the photo on the terminal screen");
}
void help_tdump() {
  Serial.println("Output to terminal the contents of a file in Ascii Text Mode");
}
void help_info() {
  Serial.println("Send to Terminal, information header for this program version");
}
void help_trtc() {
  Serial.println("Look at the bits set in the RTC control registers");
}
void help_format() {
  Serial.println("Will format the SD card with Fat32 format, all");
  Serial.println("will be erased and directories removed");
}
void help_free() {
  Serial.println("Will send to terminial the free space on the SD card");
}
void help_takeSpiphoto() {
  Serial.println("Will take a photo using c329 spi camera, will store");
  Serial.println("it on the SD card, place it in the Queue to send it ");
  Serial.println("the Host on the next request from Host");
}
void help_framdump() {
  Serial.println("Will send to the terminal the first- 512 bytes of fram");
  Serial.println("memory space, this is where all non volatile variables");
  Serial.println("are stored"); 
}
void help_initfram() {
  Serial.println("this will clear, set to 0, all photo counters,");
  Serial.println("reset number counter, and the mission clock");
  Serial.println("this should be executed before FLIGHT operations");
}
void help_framclear() {
  Serial.println("This will clear all of fram to zero, for the fram");
  Serial.println("to be set up for flight a initfram must be performed");
}
void help_SystemSetup() {
  Serial.println("Comming Soon");
}
void help_ReadSetup() {
  Serial.println("Comming Soon");
}
void help_listQue() {
  Serial.println("Will list all the photo files in the Queue waiting");
  Serial.println("to be sent the the Host");
}
void help_enterTeamID() {
  Serial.println("Using the form 'enterTeamID D' with D being");
  Serial.println("your Teams Idenification Number A through O");
}
//
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
int cmd_tdump() {                       //Download SD file in ASCII
  for (int x = 0; x < 80; x++) {
    Serial.print("^");
  }
  Serial.println();
  File dataFile = SD.open(args[1]);
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();                   //close the data file
  }
  else {
    Serial.println("error opening file");
  }
  Serial.println();
  for (int x = 0; x < 80; x++) {
    Serial.print("^");
  }
  Serial.println();
  return 0;
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
int cmd_dir() {                           //list the SD card directory of files
  detachInterrupt(digitalPinToInterrupt(SerialIRQin));  //detach the serialin IRQ during SD operations - SD uses IRQ itself
  SD.ls(LS_DATE | LS_SIZE | LS_R);      //list directory command
  Serial.println(F("Done"));            //say done
  attachInterrupt(digitalPinToInterrupt(SerialIRQin), Hostinterupt, FALLING); //reattach interrupts to Host
  return 0;                               //return no error
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
int cmd_enterTeamID() {                      //Enter Team ID
  char x = args[1][0];
  writebytefram(x, ID);
  //writeIDfram(args[1]);
  Serial.print("Team ID set to: ");
  Serial.print(static_cast<char>(readIDfram()));
  Serial.println();
  return 0;
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
int cmd_mkdir() {                                 //Make a new directory of sub directory
  detachInterrupt(digitalPinToInterrupt(SerialIRQin));  //detach the serialin IRQ during SD operations - SD uses IRQ itself
  FsDateTime::setCallback(dateTime);            //set time and date for file
  if (!SD.mkdir(args[1])) {                     //make directory with 1st argument of input
    Serial.println("Create Folder failed");     //show could not create
    attachInterrupt(digitalPinToInterrupt(SerialIRQin), Hostinterupt, FALLING); //reattach interrupts to Host
    return 1;                                   //return with error
  }
  attachInterrupt(digitalPinToInterrupt(SerialIRQin), Hostinterupt, FALLING); //reattach interrupts to Host
  return 0;                                     //return no error
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

int cmd_rmdir() {
  detachInterrupt(digitalPinToInterrupt(SerialIRQin));  //detach the serialin IRQ during SD operations - SD uses IRQ itself
  if (!SD.rmdir(args[1])) {
    Serial.println("rmdir failed\n");
    attachInterrupt(digitalPinToInterrupt(SerialIRQin), Hostinterupt, FALLING); //reattach interrupts to Host
    return 1;                                  //return with error
  }
  attachInterrupt(digitalPinToInterrupt(SerialIRQin), Hostinterupt, FALLING); //reattach interrupts to Host
  return 0;                                   //return no error
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

int cmd_open() {                                // Create File in current directory.
  detachInterrupt(digitalPinToInterrupt(SerialIRQin));  //detach the serialin IRQ during SD operations - SD uses IRQ itself
  FsDateTime::setCallback(dateTime);            //set time and date for file
  if (!file.open(args[1], O_WRONLY | O_CREAT)) { //Create and write only
    Serial.println("create File failed");
    attachInterrupt(digitalPinToInterrupt(SerialIRQin), Hostinterupt, FALLING); //reattach interrupts to Host
    return 1;
  }
  file.println("Testing 1,2,3...");
  file.close();                               //close file just opened
  attachInterrupt(digitalPinToInterrupt(SerialIRQin), Hostinterupt, FALLING); //reattach interrupts to Host
  return 0;                                   //return no error
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

int cmd_erase() {                            //remove or erase a file in current dir
  detachInterrupt(digitalPinToInterrupt(SerialIRQin));  //detach the serialin IRQ during SD operations - SD uses IRQ itself
  if (!SD.remove(args[1])) {                  //erase the file in current directory
    Serial.println("erase file failed");
    attachInterrupt(digitalPinToInterrupt(SerialIRQin), Hostinterupt, FALLING); //reattach interrupts to Host
    return 1;
  }
  attachInterrupt(digitalPinToInterrupt(SerialIRQin), Hostinterupt, FALLING); //reattach interrupts to Host
  return 0;
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

int cmd_led() {
  if (strcmp(args[1], led_args[0]) == 0) {
    Serial.println("Turning on the LED.");
    digitalWrite(LEDpin, HIGH);
  }
  else if (strcmp(args[1], led_args[1]) == 0) {
    Serial.println("Turning off the LED.");
    digitalWrite(LEDpin, LOW);
  }
  else if (strcmp(args[1], led_args[2]) == 0) {
    if (atoi(args[2]) > 0) {
      Serial.print("Blinking the LED ");
      Serial.print(blink_cycles);
      Serial.print(" times at ");
      Serial.print(args[2]);
      Serial.println(" Hz.");

      int delay_ms = (int)round(1000.0 / atoi(args[2]) / 2);

      for (int i = 0; i < blink_cycles; i++) {
        digitalWrite(LEDpin, HIGH);
        delay(delay_ms);
        digitalWrite(LEDpin, LOW);
        delay(delay_ms);
      }
    }
    else {
      Serial.println("Invalid frequency.");
    }
  }
  else {
    Serial.println("Invalid command. Type \"? led\" to see how to use the LED command.");
  }
  return 0;
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
int cmd_DotStar() {
  if (strcmp(args[1], DotStar_args[0]) == 0) {  //DotStar commands for color
    DotStarOff();
  }
  else if (strcmp(args[1], DotStar_args[1]) == 0) {
    DotStarRed();
  }
  else if (strcmp(args[1], DotStar_args[2]) == 0) {
    DotStarGreen();
  }
  else if (strcmp(args[1], DotStar_args[3]) == 0) {
    DotStarYellow();
  }
  else if (strcmp(args[1], DotStar_args[4]) == 0) {
    DotStarBlue();
  }
  else if (strcmp(args[1], DotStar_args[5]) == 0) {
    DotStarMagenta();
  }
  else if (strcmp(args[1], DotStar_args[6]) == 0) {
    DotStarCyan();
  }
  else if (strcmp(args[1], DotStar_args[7]) == 0) {
    DotStarWhite();
  }
  else {
    Serial.println("unknown command");
  }
  delay(1500);
  return 0;
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

int cmd_exit() {
  Serial.println("Exiting Command Line processor\n");
  testing = false;
  return 0;
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

int cmd_scani2c() {
  Serial.print("Scanning address range 0x03-0x77\n\n");
  i2cdetect();  // default range from 0x03 to 0x77
  return 0;
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//  Read the temperature, humidity, pressure, gas
//
int read_bme680() {
  if (!bme.begin(bme680address)) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    return -1;
  }
  bme.setTemperatureOversampling(BME680_OS_8X);   //Set up for reading
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);                     // 320*C for 150 ms
  //
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return -1;
  }
  int i = (bme.temperature * 1000);          //place temperature in text output buffer
  int C = i / 10;                               // save for f temp
  text_Atemp[22] = (i / 10000) + '0';
  i = i - ((i / 10000) * 10000);
  text_Atemp[23] = (i / 1000) + '0';
  i = i - ((i / 1000) * 1000);
  text_Atemp[25] = (i / 100) + '0';
  i = i - ((i / 100) * 100);
  text_Atemp[26] = (i / 10) + '0';
  // for f
  i = (C * 9 / 5) + 3200;
  text_Atemp[30] = (i / 10000) + '0';
  i = i - ((i / 10000) * 10000);
  text_Atemp[31] = (i / 1000) + '0';
  i = i - ((i / 1000) * 1000);
  text_Atemp[32] = (i / 100) + '0';
  i = i - ((i / 100) * 100);
  text_Atemp[34] = (i / 10) + '0';
  text_Atemp[35] = (i % 10) + '0';
  //
  i = (bme.humidity * 1000);                      //place humidity in text output buffer
  text_humidity[19] = (i / 10000) + '0';
  i = i - ((i / 10000) * 10000);
  text_humidity[20] = (i / 1000) + '0';
  i = i - ((i / 1000) * 1000);
  text_humidity[22] = (i / 100) + '0';
  i = i - ((i / 100) * 100);
  text_humidity[23] = (i / 10) + '0';
  //
  i = (bme.gas_resistance);                        //place gas resistance in text output buffer
  text_gas[6] = (i / 100000) + '0';
  i = i - ((i / 100000) * 100000);
  text_gas[7] = (i / 10000) + '0';
  i = i - ((i / 10000) * 10000);
  text_gas[8] = (i / 1000) + '0';
  i = i - ((i / 1000) * 1000);
  text_gas[10] = (i / 100) + '0';
  i = i - ((i / 100) * 100);
  text_gas[11] = (i / 10) + '0';
  //
  i = (bme.pressure);                        //place pressure hpa in text output buffer
  text_pressure[11] = (i / 100000) + '0';
  i = i - ((i / 100000) * 100000);
  text_pressure[12] = (i / 10000) + '0';
  i = i - ((i / 10000) * 10000);
  text_pressure[13] = (i / 1000) + '0';
  i = i - ((i / 1000) * 1000);
  text_pressure[14] = (i / 100) + '0';
  i = i - ((i / 100) * 100);
  text_pressure[16] = (i / 10) + '0';
  text_pressure[17] = (i % 10) + '0';
  //
  i = (bme.readAltitude(SEALEVELPRESSURE_HPA)) * 100;
  text_altitude[20] = (i / 1000000) + '0';
  i = i - 1000000;
  if (i < 0) {
    i = i + 1000000;
  }
  text_altitude[21] = (i / 100000) + '0';
  i = i - 100000;
  if (i < 0) {
    i = i + 100000;
  }
  text_altitude[22] = (i / 10000) + '0';
  i = i - ((i / 10000) * 10000);
  if (i < 0) {
    i = i + 10000;
  }
  text_altitude[23] = (i / 1000) + '0';
  i = i - ((i / 1000) * 1000);
  if (i < 0) {
    i = i + 1000;
  }
  text_altitude[24] = (i / 100) + '0';
  i = i - ((i / 100) * 100);
  text_altitude[25] = '.';
  if (i < 0) {
    i = i + 100;
  }
  text_altitude[26] = (i / 10) + '0';
  i = i - ((i / 10) * 10);
  text_altitude[27] = (i % 10) + '0';

  //

  //
  return 0;
}


//--------------------------------------------------------------------------
int cmd_bme680() {
  read_bme680();

  Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");
  Serial.println();

  return 0;
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
byte bcdToDec(byte value) {
  return ((value / 16) * 10 + value % 16); //used in time
}
byte decToBcd(byte value) {
  return (value / 10 * 16 + value % 10); //used in time
}

void readPCF85263()          // this gets the time and date from the PCF85263
{
  Wire.beginTransmission(PCF85263address);
  Wire.write(0x01);
  Wire.endTransmission();
  Wire.requestFrom(PCF85263address, 7);
  second     = bcdToDec(Wire.read() & B01111111); // remove VL error bit
  minute     = bcdToDec(Wire.read() & B01111111); // remove unwanted bits from MSB
  hour       = bcdToDec(Wire.read() & B00111111);
  dayOfMonth = bcdToDec(Wire.read() & B00111111);
  dayOfWeek  = bcdToDec(Wire.read() & B00000111);
  month      = bcdToDec(Wire.read() & B00011111); // remove century bit, 1999 is over
  year       = bcdToDec(Wire.read());


  if (month < 10) {                           //add month to char output buffer
    text_time[12] = '0';
  }
  else {
    text_time[12] = (month / 10) + '0';
  }
  text_time[13] = (month % 10) + '0';
  //
  if (dayOfMonth < 10) {                     //add day to char output buffer
    text_time[15] = '0';
  }
  else {
    text_time[15] = (dayOfMonth / 10) + '0';
  }
  text_time[16] = (dayOfMonth % 10) + '0';
  //
  text_time[18] = (year / 10) + '0';     //add year to char output buffer
  text_time[19] = (year % 10) + '0';
  //
  if (hour < 10) {                           //add hour to char output buffer
    text_time[23] =  '0';
  }
  else {
    text_time[23] = (hour / 10) + '0';
  }
  text_time[24] = (hour % 10) + '0';
  //
  if (minute < 10) {                           //add minute to char output buffer
    text_time[26] =  '0';
  }
  else {
    text_time[26] = (minute / 10) + '0';
  }
  text_time[27] = (minute % 10) + '0';
  //
  if (second < 10) {                           //add second to char output buffer
    text_time[29] =  '0';
  }
  else {
    text_time[29] = (second / 10) + '0';
  }
  text_time[30] = (second % 10) + '0';
  //
  //
  DateTime now = rtc.now();                 //add Unix time to char output buffer
  str = String(now.unixtime());
  str.toCharArray(text_buf, 11);
  int y = 32;                           //set destination pointer into array
  for (int i = 0; i < 11; i++) {
    text_Unix[y] = text_buf[i];          //move data to new array
    y++;                                 //move destination pointer
  }
  //


}
int cmd_time() {
  readPCF85263();
  //   Serial.print(days[dayOfWeek]);
  //   Serial.print(" ");
  if (month < 10)
  {
    Serial.print("0");
  }
  Serial.print(month, DEC);
  Serial.print("/");
  if (dayOfMonth < 10)
  {
    Serial.print("0");
  }
  Serial.print(dayOfMonth, DEC);
  Serial.print("/20");
  Serial.print(year, DEC);
  Serial.print(" - ");
  Serial.print(hour, DEC);
  Serial.print(":");
  if (minute < 10)
  {
    Serial.print("0");
  }
  Serial.print(minute, DEC);
  Serial.print(":");
  if (second < 10)
  {
    Serial.print("0");
  }
  Serial.println(second, DEC);
  //RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR

  DateTime now = rtc.now();                                 //%%%
  //     dateTime(uint16_t* date, uint16_t* time, uint8_t* ms10)
  Serial.print("Unix Time = Seconds since 1970: ");         //%%%
  Serial.println(now.unixtime());                           //%%%


  return 0;
}
//
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
void setPCF85263()            // this function sets the time and date to the PCF85263
{
  Wire.beginTransmission(PCF85263address);
  Wire.write(0x01);
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(dayOfMonth));
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.endTransmission();
}
//
int cmd_settime() {
  Serial.println("YYMMDDHHMMSS");                   //print header for referance
  Serial.println(args[1]);                          //print input to asure correct
  year = ((args[1][1]) + (args[1][0] * 10) - 16);   //set year (why + 16 ???)
  month = (args[1][3] + (args[1][2] * 10) - 16);    //set month (don't know why always +16)
  dayOfMonth = (args[1][5] + (args[1][4] * 10) - 16); //place holder in read, not used
  hour = (args[1][7] + (args[1][6] * 10) - 16);     //set hour (still +16?)
  minute = (args[1][9] + (args[1][8] * 10) - 16);   //set minute (+16??)
  second = (args[1][11] + (args[1][10] * 10) - 16); //set second (+16??)
  setPCF85263();                                    //write the time to pcf85263
  cmd_time();                                       //print out the new time
  return 0;                                         //return no error
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

int cmd_rtcreg() {            //read status Ram byte from RTC
  Wire.begin();                                    //begin I2C operation
  Wire.beginTransmission(PCF85263address);         //address the RTC
  Wire.write(byte (0x00));                         //Ram in RTC
  Wire.endTransmission();
  Wire.requestFrom(PCF85263address, 0x2F);
  for (int i = 0; i <= 0x2F; i++) {
    Serial.print(i, HEX);
    Serial.print(" = ");
    uint8_t x = (Wire.read());
    Serial.print(x < 16 ? "0" : "");
    Serial.println(x, HEX);
  }
  return 0;
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

int cmd_dump() {                          //Download SD file in HEX

  File dataFile = SD.open(args[1]);
  if (dataFile) {
    int i = 0;
    while (dataFile.available()) {
      if (i % 16 == 0) {                //formatting for each line
        Serial.println();
        if (i < 0x0F) {
          Serial.print("0"); //leading "0" formatting for line address
        }
        if (i < 0xFF) {
          Serial.print("0");
        }
        if (i < 0xFFF) {
          Serial.print("0");
        }
        Serial.print(i, HEX);           //Hex address of line
        Serial.print("  ");             //spacer between address and data
      }
      uint8_t x = dataFile.read();
      if (x < 16) {
        Serial.print("0"); //leading "0" on data
      }
      Serial.print(x, HEX);             //print it
      Serial.print(" ");                //space between data
      i++;                              // to next data to form 16 colums
    }
    dataFile.close();                   //close the data file
  }
  else {
    Serial.println("error opening file");
  }
  Serial.println();
  return 0;
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//--------------------------------------------------------
void getPicture_callback(uint32_t pictureSize, uint16_t packetSize, uint32_t packetStartPosition, byte* packet)
{
  photoFile.write(packet, packetSize);
  writtenPictureSize += packetSize;
  Serial.print("*");
  if (writtenPictureSize >= pictureSize)
  {
    photoFile.flush();
    photoFile.close();
    //    Serial.println("SUCCESS");
  }
}
//
//-------------------------------------------------------------------------------------------
// enter with args[1]  set to file name without extension
//
int cmd_sphoto() {                                    //take C329 Serial Photo and Store on SD
  detachInterrupt(digitalPinToInterrupt(SerialIRQin));  //detach the serialin IRQ during SD operations - SD uses IRQ itself
  digitalWrite(Sel0, HIGH);                             //high to select serial to C329 camera
  Serial1.begin(CAMERA_BAUD);                           //Set default Baud rate to camera
  if (!SD.begin(B_MicroSD_CS))                          //Make sure SD is in and operational
  {
    Serial.println("SD initialization failed");         //  SD failed
    CameraCleanReturn();                                // Clean up camera set up
    attachInterrupt(digitalPinToInterrupt(SerialIRQin), Hostinterupt, FALLING); //reattach interrupts to Host
    return 0;         //return with 0 is error

  }
  //  Serial.println("SD OK");                          //testing
  //
  FsDateTime::setCallback(dateTime);                    //get time and date for file
  //
  //
  strcat(args[1], ".jpg");                             //add .jpg to file name
  //
  if (SD.exists(args[1])) {                       //check if file already exists
    SD.remove(args[1]);                                       //will erase existing file
    Serial.println("\r\nerasing existing photoFile");            //say it
  }
  //
  // ----- File name in args[1} -
  photoFile = SD.open(args[1], FILE_WRITE);             //Open to write file named photoFile
  photoFile.seek(0);                                    //Point to first entry in file
  writtenPictureSize = 0;                               //Clear the Pitchure Sixe for counting
  //  Serial.println("open OK");                          //testing
  digitalWrite(Cam_power, Cam_power_OFF);             //Serial and SPI camera cycle power off
  delay(100);                                           //wait for good reset time 100ms
  digitalWrite(Cam_power, Cam_power_ON);              //Serial and SPI camera cycle power ON
  delay(200);                                           //Camera Boot Time
  if (!camera.sync())                                   //Send sync bytes to camera
  {
    Serial.println("Sync failed");                      //did not sync, something wrong
    CameraCleanReturn();                                //clean up camera set up
    return 0;                                           //return with 0 is error
  }
  //  Serial.println("Sync OK");                          //testing
  if (!camera.initialize(Quest_CameraC329::BAUD14400, Quest_CameraC329::CT_JPEG, Quest_CameraC329::PR_160x120, Quest_CameraC329::JR_640x480)) //write initJR_320x240
    //
  {
    Serial.println("Initialize failed");                //did not initialize, something wrong
    CameraCleanReturn();                                //clean up camera set up
    return 0;                                           //return with 0 is error
  }
  //  Serial.println("init OK");                          //testing
  if (!camera.setQuality(Quest_CameraC329::QL_BEST))    //Set Quality, amount of compression
  {
    Serial.println("Set quality failed");               //quality failed, something wrong
    CameraCleanReturn();                                //clean up camera set up
    return 0;                                           //return with 0 is error
  }
  //  Serial.println("Qual OK");                          //testing
  if (!camera.getPicture(Quest_CameraC329::PT_JPEG_PREVIEW, &getPicture_callback)) //Take and get the picture
  {
    Serial.println("Get Picture Failed");                 //Could not get the picture
    CameraCleanReturn();                                  //clean up camera set up
    return 0;                                             //return with 0 is error
  }
  //  Serial.println("Picture OK");                       //testing
  CameraCleanReturn();                                //Got here all is good, clean up

  EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(0);     //reset the IRQ Flag
  attachInterrupt(digitalPinToInterrupt(SerialIRQin), Hostinterupt, FALLING);

  return 1;
}
void CameraCleanReturn() {                              //Clean up Camera setup to return
  digitalWrite(Cam_power, Cam_power_OFF);         //Serial and SPI camera power OFF
  digitalWrite(Sel0, LOW);                          //LOW on select to connect to Host
  IRQreference = millis();                          //capture millis for next time, start of interrupt
  //
  Bank0size = writtenPictureSize;     //save the size for later
  //
  Serial.print("\n\rphotosize = "); Serial.println(Bank0size, HEX);
  Serial.print("filename is = "); Serial.println(args[1]);
  //

  WriteText();

  // char filenameS[] = "XS00000PSS";        //create filename templet for name SPI, SS = size of file for Queue
  // all ok add to host Queue
  //
  filenameS[7] = 'P';
  filenameS[8] = (writtenPictureSize / 256);
  filenameS[9] = (writtenPictureSize % 256);
  filenameS[10] = '\0';
  //
  addFileToQueue(filenameS);    //, writtenPictureSize);
  //
  Serial.println(filenameS);
  //
}
//
//++++++++++++++++++++++++++++++++++++++++++
//  No matter what create a text file when a photo is taken, photo file closed
//  Enter with args[1] setto filename.xxx, will change it to .txt
//++++++++++++++++++++++++++++++++++++++++++
//
void  WriteText(void) {
  fill_text();           //fill data in system text buffer and send it to the terminal
  //----------------------------------------------------------------
  //  Text buffer now filled with current data send it with the photo
  //--  Send the Text file to the SD Card   ---------
  //  here args[1] contains file name with .jpg extension
  //
  int x = strlen(args[1]);                             //get the length of the name + ext
  args[1][x - 3] = ('t');                              //rewrite extension to txt
  args[1][x - 2] = ('x');
  args[1][x - 1] = ('t');

  if (SD.exists(args[1])) {                       //check if file already exists
    SD.remove(args[1]);                                       //will erase existing file
    Serial.println("erasing existing TextFile");            //say it
  }


  TextFile = SD.open(args[1], FILE_WRITE);             //Open to write file named TextFile
  TextFile.seek(0);                                    //Point to first entry in file
  if (TextFile) {                                      //if can open good
    //                                                 //file can open now write stock text
    for (int y = 0; y < 14; y++) {                    //buffer to SD file .txt
      TextFile.println(*(textlist + y));              //write to file lines pointed to by list
    }                                                 //full stock Text file completed here
    TextFile.println("----------------------------------------\n\r");  //spacer between text blocks
    //
    //  Now add User buffer at end of text file
    //
    strcat(user_text_buf0, ("\n\r** END **"));          // note end of text file
    //            strcat(user_text_buf0,(NULL));                    //set end of text marker
    for (int x = 0; x < strlen(user_text_buf0); x++) {  //Look at user text buffer
      TextFile.write(user_text_buf0[x]);            //write the data in the text buffer to SD
    }
    //
    TextFile.flush();                                 //clear SD file buffer
    TextFile.close();                                 //close the file here
    //
    // Clear the buffer
    user_text_buf0[0] = '\0';
    //         memset(user_text_buf0, 0x00, sizeof(user_text_buf0));                             //clear the user buffer
    //
  }
  //            Pulse11high(); //testing


  //   IRQinvalid = 1;    //during this time an IRQ has probabily been generat and pending, disrefard this IRQ
  //this was a request from the HOST when the Microlab was busy, the microlab can not
  //respond properly to the IRQ if it is already gone..will catch it next time.
  //

}
//  }

//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
int cmd_upload() {                      //command to upload a file to the Host use SD for now.
  detachInterrupt(digitalPinToInterrupt(SerialIRQin));  //detach the serialin IRQ during SD operations - SD uses IRQ itself

  Serial.println("upload command");
  IRQreference = millis();                          //capture millis for next time, start of interrupt

  attachInterrupt(digitalPinToInterrupt(SerialIRQin), Hostinterupt, FALLING); //reattach interrupts to Host
  return 1;
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
const int IO[] = {A6, 3, 4, 9, 10, 11, 12, 13};

int cmd_io() {                      //command to upload a file to the Host use SD for now.


  int x = atoi(args[1]);
  if (x < 0 || x > 7) {
    Serial.println("input only 0 to 7");
    return 0;
  }
  Serial.println(IO[x]);

  int y = (IO[x]);
  Serial.println(y);

  //
  char* cp = (args[2]);             //make cp point to beginneing args[2]
  char z = *cp;                     //z is the value at pointer
  if (z == 'H') {
    Serial.println("was a H");
    digitalWrite(y, HIGH);
    return 0;
  }
  if (z == 'L') {
    Serial.println("was a L");
    digitalWrite(y, LOW);
    return 0;
  }
  else {
    Serial.println("input only H or L accepted");
    return 1;
  }

}


//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
int cmd_ana() {                      //command to upload a file to the Host use SD for now.
  Serial.println("ana command");

  int sensorValue = analogRead(A0);   //read the digital value of the input
  float voltage = sensorValue * (3.3 / 1023);
  Serial.print("A0 = "); Serial.print(voltage); Serial.print("  ");
  sensorValue = analogRead(A1);   //read the digital value of the input
  voltage = sensorValue * (3.3 / 1023);
  Serial.print("A1 = "); Serial.print(voltage); Serial.print("  ");
  sensorValue = analogRead(A2);   //read the digital value of the input
  voltage = sensorValue * (3.3 / 1023);
  Serial.print("A2 = "); Serial.print(voltage); Serial.print("  ");
  sensorValue = analogRead(A3);   //read the digital value of the input
  voltage = sensorValue * (3.3 / 1023);
  Serial.print("A3 = "); Serial.print(voltage); Serial.println("  ");
  return 1;
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
// first take a photo and put on SD for now... need to put in buffer for speed later
//----------  real photo and header here ------------------------
//char FileHeader[] = {'X','T','4','3','2','1','0','P',0x04,0x80};  //header string to send to host
//file on disk = {FileID[0],FileType,'5','4','3','2','1','.','j','p','g'};   //type S,P,D
//
int cmd_takeSphoto() {        //19 - take a serial photo get a file name then place it in the Host Queue
  Serial.println("Got to cmd_takeSphoto() -- line 1921>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  //
  Serial.print("P0Maddress = ");                //testing out put
  Serial.println(readintFromfram(PCSaddress));
  Serial.print("Serial photo ID = ");
  Serial.println((char)readbyteFromfram(ID));           //Team ID Letter
  //
  int x = (readintFromfram(PCSaddress));    //get next serial photo name
  x++;
  if (x > 99999) {
    x = 0;
  };
  writeintfram(x, PCSaddress);              //inc and write for next time
  //
  itoa(x, ascii, 10);                      //convert PCM photo count x to ascii string
  int z = 0;                               //to count number of valid entries in array
  for (int i = 0; ascii[i] != '\0'; i++) {
    z++;
  };  //find null char, z now points to pointer
  uint16_t  y = 7 - z;                     //where to place lsd in mission buffer
  for (uint16_t x = 0 ; x < z; x++) {      //how many charators
    filenameS[y] = ascii[x];           //transfer from aacii array to proper location in text_mission
    y++;                               //pointer to text_mission array
  }
  //
  filenameS[0] = readbyteFromfram(ID);    //Get team ID letter, place it as first letter of file name
  //
  //Serial.println(filenameS) is ready to use as name on file

  for (int x = 0; x < 10; x++) { //clear args[1]
    args[1][x] = '\0';          //clear args[1]
  }
  for (int x = 0; x < 7; x++) { //move number of chartors
    args[1][x] = filenameS[x]; //move the filenameS into args[1]
  }
  //
  Serial.println (args[1]);          //output the file name part of the header no extension
  //
  cmd_sphoto();                     //take the serial photo and store it on SD with the args[1].jpg name
  //
  //  now take SD file and send to to Queue
  //
  Bank0size = writtenPictureSize;     //save the size for later
  //
  // now have name and size...
  // now have the information to put into Queue

  Serial.println("Got past cmd_sphoto() --- line 1890>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");

  // Status = Status | Bank0status;            //data in bank 0
  // Serial.print("Setting Buffer 0 status to = ");
  //  Serial.println(Status, HEX);

  return 1;
}

//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//
//
//      byte FileHeader[] = {'X','Y','4','3','2','1','0','P',0x04,0x80};  //header string
//      Enter with args[1] set will save that file name
//      Enter without args[1] set will assign VUXXXXX file name with x being count up
//
int cmd_view() {

  //
  if (strlen(args[1]) == 0) {         //if no file name given, give it VU perfix for view file
    itoa(photonumber, PN, 10);       //Place counting up int photonumber into the PN char array
    photonumber++;                    // inc to next number
    PN[0] = 'V'  ;                    //force ID code to view code for file storage
    PN[1] = 'C'  ;                    //forced ID code
    strcat(args[1], PN);
  }
  Serial.println("vc start");
  //
  cmd_sphoto();                     //take the serial photo and store it on SD with the args[1].jpg name
  //
  //  replace extension to jpg
  int x = strlen(args[1]);                             //get the length of the name + ext
  args[1][x - 3] = ('j');                              //rewrite extension to jpg
  args[1][x - 2] = ('p');
  args[1][x - 1] = ('g');
  FsDateTime::setCallback(dateTime);            //set time and date for file************************************
  File dataFile = SD.open(args[1]);                 //file name here
  x = strlen(args[1]);                             //get the length of the name + ext
  args[1][x - 4] = ('\0');                         //rewrite extension from jpg to header header info
  Serial.print (args[1]);                         //output the file name part of the header
  Serial.print('P');                              // P char place holder for photo type file
  Serial.print(writtenPictureSize / 256, HEX);    // output
  Serial.print(writtenPictureSize % 256, HEX);    //output

  if (dataFile) {
    int i = 0;
    while (dataFile.available()) {      //data still in file
      if (i % 32 == 0) {                //formatting for each line
        Serial.println();               //send 32 char per line
      }
      uint8_t x = dataFile.read();      //get the data from file
      if (x < 16) {
        Serial.print("0"); //leading "0" on data
      }
      Serial.print(x, HEX);             //print it in hex to terminal
      i++;                              // to next data to form 16 colums
    }
    dataFile.close();                   //close the data file
  }
  else {
    Serial.println("error opening file");
  }

  Serial.println();
  //  Serial.println(writtenPictureSize, HEX);

  return 1;
}
//
//-----------------------------------------------------------------------------------------
//IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
// Interupt handler Values
//IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
//
//----- Header Infromation ---------------------------------------------------------
// Testing output testing
//
//---- Communication command structure -------------------------
//
#define GetStatus (0x10)
#define DownLoadfile (0x12)   //Download a file from master to microlab
#define GetMLID (0x11)
#define Ack (0x5A)
#define Syncback (0x55)
#define SetTime (0x14)
#define GetV0 (0x15)
#define GetV1 (0x16)
#define GetV2 (0x17)

//----- Real time clock structure from Master controller  -------------

#define RTCsec    (0)
#define RTCmin    (1)
#define RTChour   (2)
#define RTCday    (3)
#define RTCmonth  (4)
#define RTCyear   (5)
int RTCarray[] = {1, 2, 3, 4, 5, 6}; //real time clock array storage locations RTCsec

//---------  Photo Header structure -----------------------------
#define ID0  (0);  //("X")  index to 1st ID Letter
#define ID1  (1);  //("Y")  index to 2nd ID Letter
#define PN5  (2);  // photo number
#define PN4  (3);
#define PN3  (4);
#define PN2  (5);
#define PN1  (6);
#define PN0  (7);
#define TY   (8);   //type "P" photo
#define MSD  (9);   //length of jpg file
#define LSD  (10);
//
//


/*
  //----------------- Hardware definitions
  #define IO7 (13)              // Input/Output to payload plus
  #define IO6 (12)              // Input/Output to payload plus
  #define IO5 (11)              // Input/Output to payload plus
  #define IO4 (10)              // Input/Output to payload plus
  #define IO3 (9)              // Input/Output to payload plus
  #define IO2 (4)              // Input/Output to payload plus
  #define IO1 (3)              // Input/Output to payload plus
  #define IO0 (A6)              // Input/Output to payload plus
*/
//---- Serial input from Master controller

//#define Sel0 (A4)           //serial selector i/o select
#define MasterIn (HIGH)
#define SerialIRQin  (0) //(IO3)   //serial 1 rx input used for interrupt

const long FmHostRequestTimeout = 5000;    //Time to wait for serial command timeout Micro sec
const int long valuemasterlowabort = 100000;
//
#define TimeTimeout    (30)    //Time to wait for time abort fails 
#define ArmFmHostRequest     (10)   // if serial input idle for > 100ms must be command
#define loopcount1     (1000)    //for testing main loop heartbeat time

//-----FmHostRequest Storage ------------------------------------------

int FmHostRequest = 0x00;           //for command byte from master
int Armed = 0x00;             //Flag to Arm the command input
int Timeout = 1;              //
int val = 0;                  //
int TempReg0 = 0;             // temperary variable
int ErrorCode = 0;            // starting error
//unsigned long looptimebase = 0;         // loop heartbeat time
unsigned long entrytime = 0;  //
//unsigned long int IRQreference = 0; // 1st read in setup for proper counting
//boolean toggle = false;
int softuarterror = 0;      //0 = no error, 1 = timeout error, >1 = ?? not defined
long long softtimeout = 2500000;    //  Microseconds softtimeout for received data was 15 n0w 25
long CharWait = 1100;     //microsec between charators wait time 100;//500;//
//
unsigned long int testtime = 0;  //used for test looptiming only
#define testlooptime  (40000) //
uint16_t testcount = 0;       // used in test to count number of loops
//
//#define softuartrate (104) //software loop
//#define softuartinpin (0)//(0) //
//#define softuartoutpin (1)//(1) //
char data = 0;
//

//----------------- Prototype functions ----------------------------------------------
void Readmaster();
void serial1Flush();
void printError(int x);
void Hostinterupt();
//void GetFmHostRequest();
//

//
//---------------------------------------------------------------------------------
void  softuartwrite(uint8_t data);
byte  softuartread();

void Chardelay() {
  delayMicroseconds(CharWait);           //delay to let master to recover
}
//*****************************************
// SOFTWARE SERIAL READ INPUT
//*****************************************
//
byte softuartread() {
  byte setbit = 0b00000001;                    //Mask of bit to set for incomming data
  byte dataByte = 0;                           //Clear data receive byte
  unsigned long i = softtimeout;               //set softread timeout counter
  while (digitalRead(softuartinpin) == HIGH) {   //Waiting for startbit
    delayMicroseconds(1);
    i--;                                          //count down to error
    if (i == 0) {
      softuarterror = 1;                        //set timeout error
      return 0;
    }
  }
  // found start bit falling edge
  delayMicroseconds(softuartrate / 2);          //one half bit time
  delayMicroseconds(softuartrate);              //delay past start bit into 1 data bit
  for (int i = 0; i < 8; i++) {                 //now do it for 8 bit times
    if (digitalRead(softuartinpin) == HIGH) {   //look at the bit in the center
      dataByte = dataByte + setbit;             //Set bit in dataByte
      setbit = setbit * 2;                      //advance bit to next location
    }
    else {
      setbit = setbit * 2;                      //No set bit just advance
    }
    delayMicroseconds(softuartrate);              //wait a bit time
  }
  softuarterror = 0;                            //reset error to no error data ok
  return dataByte;                                  //return with received data
}
//
//********************************************
//  SOFTWARE SERIAL WRITE OUTPUT
//********************************************
//
void  softuartwrite(uint8_t data) {
  pinMode(softuartoutpin, OUTPUT);
  digitalWrite(softuartoutpin, LOW);                  //Start bit
  delayMicroseconds(softuartrate);                    //Start bit time
  for (int i = 0; i < 8; i++) {
    digitalWrite(softuartoutpin, (bitRead(data, i))); //read byte lsb first
    delayMicroseconds(softuartrate);                  //bit time
  }
  digitalWrite(softuartoutpin, HIGH);                 //now time for stop bit
  delayMicroseconds(softuartrate);                    //1 stop bit
  delayMicroseconds(softuartrate);                    //1 stop bit
}
//////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
// Interupt handler (approx 24 sec interval from Host Controller)
//    THIS IS THE FALLINE EDGE OF THE HOST SERIAL INPUT.
//IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
//
//
void Hostinterupt() {
  Pulse13low(); //
  if (IRQinvalid == 1) {      //is this edge a valid IRQ check for invalid
    IRQinvalid = 0;         //yes invalid reset the invalid flag
    //       Pulse13low();           //testing cause P13 to go low %%%%%%%  indicate invalid IRQ %%%%%%%%%%%%%%%%%%%%%
  }
  else {
    // Pulse13low(); //
    ////////////////////////////////////////////////////////////////////////
    //  here every falling edge of host serial in
    //  if the time is greater then it must be a new FmHostRequest from the
    //  master controller to the microlab
    //  Note: millis do not count in IRQ, counts outside IRQ
    //
    if (millis() - IRQreference > ArmFmHostRequest) {   //must be greater than FmHostRequest Arm time to process it

      IRQreference = millis();                    //capture millis for next time, start of interrupt
      Chardelay();                                //wait one charator time for Host to process
      softuartwrite(Ack);                         //Acknowledge attention send Ack of low on receive line
      //this will release the serialin line and cause the Host to pull
      //the serialin line high
      //check here that the serial line goes back high, if not Host error and abort the IRQ
      int long masterlowabort = valuemasterlowabort;       //init abort timeout Host low pulse over max time
      while (digitalRead(SerialIRQin) == LOW) {           //check for Host low after the edge is Host OK
        masterlowabort --;                              //dec the low time counter
        if (masterlowabort = 0) {                       //check for time out, must be error
          Serial.println("Error Host low to long");     //say error on terminal
          //                   Pulse13high();           //testing  P13 high abort %%%%%%%%%%%%%%%%%%%%%%%%%%%
          EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(0);     //reset the IRQ Flag
          return;                                       //return From IRQ
        }   //end abort
      }   //end while
      //
      // Host Serial in line is high, now get the serial byte being sent from the Host, if not sent in time abort
      FmHostRequest = softuartread();                     //wait for FmHostRequest from master controller
      if (softuarterror == 1) {                           //Timeout processing if FmHostRequest does not come in abort
        Serial.println("FmHostRequest timeout error");  //send error after IRQ
        //                Pulse13high();           //testing  P13 high abort %%%%%%%%%%%%%%%%%%%%%%%%%%%
        EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(0);      //reset the IRQ Flag
        return;                                        //return From IRQ
      }   // end softerror
      //
      //--------------------
      //  FmHostRequest processing, reject all invalid FmHostRequests
      //--------------------
      if ((!softuarterror) == 1) {                //if no error from soft uart fail safe
        if (FmHostRequest == GetStatus) {     //check the Host request for Get Status request
          Chardelay();                //Is Status wait one charator delay for system sync and processing
          Chardelay();                //wait one charator delay for system sync and processing
          //
          // what is the status of the output Que
          //
          getQueStatus();
          if (getQueStatus() == 0) {
            softuartwrite(0x21);      // Que has a file waiting
            Serial.println("Que something");
          }
          else {
            softuartwrite(0x20);      //Que has no file waiting
            Serial.println("Que empty");
          }
          // softuartwrite(Status);      //send status via softserial to master controller
        }
        //=====================================================================================
        if (FmHostRequest == SetTime) {
          TempReg0 = 0;               //clear to use as pointer into array
          while (TempReg0 < 6) {                    //Get 6 bytes of time
            RTCarray[TempReg0] = softuartread();  //receive byte and place into array
            TempReg0++;                           //inc pointer
            if (softuarterror == 1) {             //check for serial time out
              Serial.println("TIME timeout error");  //Say error
              //                        Pulse13high();           //testing  P13 high abort %%%%%%%%%%%%%%%%%%%%%%%%%%%
              EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(0);      //abort reset the IRQ Flag
              return;                                        //return From IRQ
            }                                    //end softuart error
          }                                        //  end while
          Serial.print("\r\nSetTime ");            //testing only output the time array
          for (int i = 0; i < 6; i++) {            //Print out the time with spaces
            Serial.print(RTCarray[i], HEX);       // s m h d m y
            Serial.print(" ");
          }                                        //  end int
          Serial.println();                        //do carrage return for formatting
          for (int i = 0; i < 20; i++) {
            Chardelay();                          //delay to let master to recover 10ms
          }                                        //  end chardelay
          softuartwrite(Ack);                     //send ack to host to complete transfer

        }                                             //  end SetTime
        //=============================================================================
        if (FmHostRequest == Ack) {
          Chardelay();           //delay to let master to recover
          Chardelay();           //delay to let master to recover
          softuartwrite(Ack);
          //               Pulse13high();           //testing  P13 high abort %%%%%%%%%%%%%%%%%%%%%%%%%%%
          EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(0);      //abort reset the IRQ Flag
          return;                                        //return From IRQ  +l
        }                          //  end Ack
        //
        //============================================================================
        if (FmHostRequest == GetV0) {
          //
          //Pick up next photo/text to output to Host from photo/Text file list....
          //   Now just deal with buffer 0  for growth testing
          //
          FileReadError = 0;                                  //reseet error if set
          //
          //
          //--------------------------
          //
          //   here get file name from Queue
          //   inc Queue
          //
          getQueStatus();             //make sure there is something in Que, error in not
          if (getQueStatus() == 1) {
            Serial.println("Requested but nothing in Que");
            //retutn 0;
          }
          else {            //something in Queue
            Serial.print("got to Que");                       //here, a file name in que
            uint16_t retval;                                  //meke return error test reg
            retval = getFilefromQue(args[1]);                 //get the file name in args[1]
            if (retval == 0) {                                //returned value ==0, got a regonized name
              Serial.print("got a filename:");                //sayso
              Serial.println(args[1]);                        //print out the name
            } else {
              Serial.println("err: got no filename");         //error does not know file
            }
            //
            for (int i = 0; i < 10; i++) {
              softuartwrite(args[1][i]);                    //send Header to Host(has length)
            }
            photobytecount = (args[1][8]) * 256;            //let's get photo byte count from header
            photobytecount = photobytecount + (args[1][9]); //lsb
            //
            args[1][7] = '\0';                              //remove none file info from header
            strcat(args[1], (".jpg"));                       // append type of file name
            Serial.println (args[1]);                        //lets print the filename to know
            //
            Chardelay();           //delay to let master to recover
            Chardelay();           //delay to let master to recover

            //
            // now open photo file to send
            //
            File dataFile = SD.open(args[1]);   //???????????????????????????????????????????????????????????????????????????????????????????????
            if (dataFile) {
              photodata = 0;                    //set photo data to clear
              Abortphoto = 0;                   //-------- No abort test ---------
              //                   photobytecount = photobytecount-1;  //photo count -1 for techque
              while (photobytecount > 0) {
                if (Abortphoto == 1) {   //---- test ----
                  EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(0);      //abort reset the IRQ Flag
                  return;                                        //return From IRQ
                }                       //  end abort
                while (photobytecount > 1024) {              //Send photo data to Host in 1024 blocks
                  if (Abortphoto == 1) {   //---- test ----
                    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(0);      //abort reset the IRQ Flag
                    return;                                        //return From IRQ
                  }                       //end abort
                  for (int x = 0; x < 1024; x++) {        //for the length of photolength buffer send
                    softuartwrite(dataFile.read());       //read fm SD and send jpg file to Host here
                    photobytecount--;                     //dec total byte count
                  }                       //end 1024
                  char y = softuartread();                //wait for acknowledge of 1024 bytes
                  if (y != 0x5A) {
                    Serial.println("Not 0x5A (ACK) during photo upload !!!!");
                    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(0);      //abort reset the IRQ Flag
                    return;                                        //return From IRQ
                  }
                  Serial.print("*");             //testing
                  if (softuarterror == 1) {
                    Serial.println("softuarttimeout photo");//set timeout error
                    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(0);      //abort reset the IRQ Flag
                    return;                                        //return From IRQ
                  }                              //  end softerror
                }                                   //  end while
                while (photobytecount > 0) {                //send last less than 1024 bytes
                  softuartwrite(dataFile.read());           //send it
                  photobytecount--;                         //looking for 0
                }                                   //  end while
              }                                       //  end photocount
              dataFile.close();                   //close the photo data file data file
            }                                           //  end datafile
            else {                                 //error opening file
              Serial.println("error opening photo file to Host- = ");
              Serial.println(args[1]);
              Pulse13high();           //testing  P13 high abort %%%%%%%%%%%%%%%%%%%%%%%%%%%
              EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(0);      //abort reset the IRQ Flag
              return;                                        //return From IRQ
            }
            //
            //---------wait ?? -------------------------------------------------------------------------
            //
            for (int x = 0; x < 4; x++) {            //delay time between jpg and txt file---------------------
              Chardelay();
            }                               //end int
            //
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            //++++++++++  Need to send text file to Host controller
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            //
            if (FileReadError == 0) {                           //abort if open or read error
              int x = strlen(args[1]);                      //get the length of the name + ext
              args[1][x - 3] = ('t');                       //rewrite extension to txt
              args[1][x - 2] = ('x');
              args[1][x - 1] = ('t');
              TextFile = SD.open(args[1]);
              if (TextFile) {
                int x;
                while (TextFile.available()) {
                  Chardelay();
                  x = (TextFile.read());         //send it
                  softuartwrite(x);
                  Serial.write(x);
                }                             //end while
                softuartwrite(0xFF);          //force end of text file
                softuartwrite(0xFF);          //always force end of text file for sure
                TextFile.close();
              } else {                                //end TextFile then else
                Serial.println("error opening TextFile to Host = ");
                Serial.println(args[1]);
                EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(0);      //abort reset the IRQ Flag
                return;                                        //return From IRQ
              }                                  //end of else
            }                                      //end if FileReadError==0
            if (FileReadError == 0) {                           //abort if open or read error
              Serial.print("\n\rDone ");
              Serial.print(Status, HEX);
              Status = 0x20;            //reset Data availiable
            }                             //end FileReadError==0
          }                                 //end GetV0
          // Here if FmHostRequest is known or unknown
          Serial.print("FmHostRequest from host = ");
          Serial.println(FmHostRequest, HEX);
        }                               //end softuarterror=1
        softuarterror = 0;              //reset error to no error data ok###################################
      }                               //end milli-IRQreference
    }                                 //end millis test
    IRQinvalid = 0;
    Pulse13high();
  }                                    //end of else
}                                     //end of Hostinterrupt
//
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

int cmd_text() {
  fill_text();
  //
  Serial.println(text_header);
  Serial.println (text_DataDown);
  Serial.println (text_time);
  Serial.println (text_Unix);
  Serial.println (text_Mission);
  Serial.println (text_Atemp);
  Serial.println (text_humidity);
  Serial.println (text_gas);
  Serial.println (text_pressure);
  Serial.println (text_altitude);
  Serial.println (text_Ain0);
  Serial.println (text_Ain1);
  Serial.println (text_Ain2);
  Serial.println (text_Ain3);
  //
  Serial.print (text_buf95);
  Serial.println (source_file);
  Serial.print (text_buf96);
  Serial.println (compile_date);
  Serial.print (text_buf97);
  Serial.println (Qversion);
  //
  return 0;
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//  Format SD Card
//
int cmd_format() {
  Serial.println("Initializing SD card...");

  if (!SD.begin(B_MicroSD_CS)) { // Check if the SD card is present and can be initialized
    Serial.println("SD card initialization failed.");
    Serial.println("Check or install SD Card");
    return 1;
  }

  Serial.println("SD card initialized.");
  Serial.println("Type 'f' to format, anything else aborts");

  // Wait for serial input to initiate formatting of the SD card
  while (Serial.available() == 0) {
  }
  char input = Serial.read();
  if (input == 'f') {
    Serial.println("Formatting SD card...");
    if (!SD.format()) {         // Format the SD card
      Serial.println("SD card formatting failed.");
      return 1;
    }
  }
  else {
    Serial.println("Fromat Aborted");
    return 1;
  }
  SoftwareReset();
  //  Serial.println("SD card formatted successfully.");
  //  initSD();                                             //set up system and operational files
  //  Serial.println("SD system files set up");
  return 0;
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//      print free space on sd card
//
int cmd_free() {
  Serial.println("Initializing SD card...");
  if (!SD.begin(B_MicroSD_CS, SPI_HALF_SPEED)) { // check if SD card is present and can be initialized
    Serial.println("SD card initialization failed.");
    return 1;     //error return
  }
  Serial.println("SD card initialization done.");
  uint32_t freeBlocks = SD.vol()->freeClusterCount(); // get the number of free blocks on the SD card
  Serial.print("FreeBlocks = ");
  Serial.println(freeBlocks);
  uint32_t freeKBytes = (freeBlocks * 32.768); // convert free blocks to kilobytes
  Serial.printf("Free space: %d KB\n", (freeKBytes)); // print the amount of free space on the SD card
  return 0;   //no error return
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
void  readAin() {                           //this gets analog 0,1,2,3
  int i = analogRead(ANA0) * 100;
  i = i * (3.3 / 1023);
  text_Ain0[17] = (i / 100) + '0';
  i = i - ((i / 100) * 100);
  text_Ain0[19] = (i / 10) + '0';
  i = i - ((i / 10) * 10);
  text_Ain0[20] = (i % 10) + '0';
  //
  i = analogRead(ANA1) * 100;
  i = i * (3.3 / 1023);
  text_Ain1[17] = (i / 100) + '0';
  i = i - ((i / 100) * 100);
  text_Ain1[19] = (i / 10) + '0';
  i = i - ((i / 10) * 10);
  text_Ain1[20] = (i % 10) + '0';
  //
  i = analogRead(ANA2) * 100;
  i = i * (3.3 / 1023);
  text_Ain2[17] = (i / 100) + '0';
  i = i - ((i / 100) * 100);
  text_Ain2[19] = (i / 10) + '0';
  i = i - ((i / 10) * 10);
  text_Ain2[20] = (i % 10) + '0';
  //
  i = analogRead(ANA3) * 100;
  i = i * (3.3 / 1023);
  text_Ain3[17] = (i / 100) + '0';
  i = i - ((i / 100) * 100);
  text_Ain3[19] = (i / 10) + '0';
  i = i - ((i / 10) * 10);
  text_Ain3[20] = (i % 10) + '0';
}
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//    fill the text buffer with the proper data
//
//
//
void fill_text() {
  getmissionclk();                  //place mission clock in output text array

  //--- do photo number ----

  Serial.print("args[1] for text output = "); Serial.println(args[1]);

  //str=String(args[1]);                  //get download number to put in PN buffer
  //str.toCharArray(text_buf,9);          //string to charArray in text_buf 9 places
  int y = 23;                           //set destination pointer into array for photo number
  for (int i = 0; i < 7; i++) {         //set output text_DataDown
    text_DataDown[y] = args[1][i];      //move data to new array
    y++;                                 //move destination pointer next location
  }
  //--- photo number done ---
  //--- over write the file ID ----
  //     text_DataDown[23] = FileID[0];        //Set ID of download 1st Charator
  //     text_DataDown[24] = FileID[1];        //Set ID of download 2nd Charator
  //
  //
  // for(int i=0;i<sizeof(text_Mission);i++){
  //  Serial.print(text_Mission[i]);
  //
  //--- Read Date/Time and put in text buffer ---
  readPCF85263();                       // this gets the time and date and put in text buffer
  //
  //--- Read temp humidity and altutude
  read_bme680();                        //this gets temp,humidy,pressure,gas
  //
  //--- Read Analog voltages
  readAin();                            //this gets analog 0,1,2,3
  //
  Serial.println("last line of fill_text");                      //print cr lf
}
///////////////////////////////////////////////////////////////////////////////
//     Mission clock operations  20221014
// first time through, if file not there make it and store the UTC time
// then next time take stored UTC time and subtract it from now time  NOT GOOD
// no power down??????????????????????????
///////////////////////////////////////////////////////////////////////////////
//
int cmd_info() {
  Serial.print("\nSystem Boot Version ");
  Serial.println(Qversion);
  Serial.print("Compiled Date => "); Serial.println(compile_date);
  Serial.print("Source file => "); Serial.println(source_file);
  Serial.println();
  Serial.print("Free Memory = "); Serial.print(freeMemory(), HEX);
  Serial.print(" HEX or "); Serial.print(freeMemory()); Serial.println(" DEC");
  return 0;
}
//
//--------------------------------------------------------------------------------
//
//----------  RTC Functions  ----------
//
void RTCregW(uint8_t x, uint8_t y) {
  Wire.begin();                                 //begin I2C operation
  Wire.beginTransmission(PCF85263address);      //address the RTC
  Wire.write(x);                                //point to register 0
  Wire.write(y);                               //point to register 0
  Wire.endTransmission();
}
uint8_t RTCregR(uint8_t x) {
  Wire.begin();//begin I2C operation
  Wire.beginTransmission(PCF85263address);      //address the RTC
  Wire.write(x);                              //point to register 0
  Wire.endTransmission();
  Wire.requestFrom(PCF85263address, x);
  x = (Wire.read());
  return x;
}
//---------------------------------------------------------
//   testing for real time clock register operations
//---------------------------------------------------------
//
//--- RTC mode Time and date registers ---
#define RTC_100th             0x00  //100th _seconds BCD 0-00 
#define RTC_Seconds           0x01  //Seconds counter BCD 0-59
#define RTC_OS                0x80  //status Oscillator stop bit in Secounts
#define RTC_Minutes           0x02  //Minutes BCD 0-59 
#define RTC_EMON              0x80  //status event monitor in Minutes
#define RTC_Hours             0x03  //Hours BCD 0-23 or am/pm 12
#define RTC_Days              0x04  //Days BCD 0-31
#define RTC_Weekdays          0x05  //Weekdays BCD 0-6 Sunday=0
#define RTC_Months            0x06  //Months BCD 1-12
#define RTC_Years             0x07  //BCD 0-99
// Stop-watch mode time registers with RTC_FR_RTCM = 1
//  Seconds and Minutes are the same as RTC mode
//  0x03 Hours BCD 0-99
//  0x04 Hours BCD 0-99
//  0x05 Hours BCD 0-99
//  0x06 and 0x07 not used
//
//--- Alarms in RTC mode --- Alarm1 ---
#define RTC_Second_alarm1     0x08  //BCD 0-59
#define RTC_Minute_alarm1     0x09  //BCD 0-59
#define RTC_Hour_alarm1       0x0A  //BCD 0-23 ampm 0-12
#define RTC_Day_alarm1        0x0B  //BCD 0-31
#define RTC_Month_alarm1      0x0C  //BCD 1-12
//--- RTC Alarm2 registers ---
#define RTC_Minute_alarm2     0x0D  //BCD 0-59
#define RTC_Hour_alarm2       0x0E  //BCD 0-23 ampm 0-12
#define RTC_Weekday_alarm2    0x0F  //BCD 0-6
//--- Alarm1 and alarm2 control in RTC mode ---
#define RTC_Alarm_enables     0x10  //alarm enable control register
#define RTC_WDAY_A2E          0x80  //Weekday alarm 2 enable =1
#define RTC_HR_A2E            0x40  //Hour alarm2 enable = 1
#define RTC_MIN_A2E           0x20  //minute alarm2 enable = 1
#define RTC_MON_A1E           0x10  //month alarm1 enable = 1
#define RTC_DAY_A1E           0x08  //day alarm1 enable = 1
#define RTC_HR_A1E            0x04  //hour alarm1 enable = 1
#define RTC_MIN_A1E           0x02  //Minute alarm1 enable =1
#define RTC_SEC_A1E           0x01  //Second alarm1 enable = 1
//--- Alarm1 and alarm2 control in stop-watch mode ---
#define RTC_Second_alm1       0x08  //BCD 0-59
#define RTC_Minute_alm1       0x09  //BCD 0-59
#define RTC_xx_xx_00_alm1     0x0A  //BCD 0-99
#define RTC_xx_00_xx_alm1     0x0B  //BCD 0-31
#define RTC_00_xx_xx_alm1     0x0C  //BCD 1-12
//--- RTC Alarm2 registers ---
#define RTC_Minute_alm2       0x0D  //BCD 0-59
#define RTC_xx_00_alm2        0x0E  //BCD 0-23 ampm 0-12
#define RTC_00_xx_alm2        0x0F  //BCD 0-6
//--- Alarm1 and alarm2 control in stop-watch mode ---
#define RTC_HR_00_xx_A2E      0x80  //thousands of hours alarm 2 enable = 1
#define RTC_HR_xx_00_A2E      0x40  //tens of hours alarm2 enable = 1
#define RTC_MIN_A2E           0x20  //minute alarm2 enable = 1
#define RTC_HR_00_xx_xx_A1E   0x10  //100 thousands of hours alarm1 enable = 1
#define RTC_HR_xx_00_xx_A1E   0x08  //thousands of hours alarm1 enable = 1
#define RTC_HR_xx_xx_00_A1E   0x04  //tens of hour alarm1 enable = 1
#define RTC_MIN_A1E           0x02  //minute alarm1 enable = 1
#define RTC_SEC_A1E           0x01  //second alarm1 enable = 1
//

//


//

//
//--- PCF85263 RTC Function Register ---
#define RTC_FR                0x28  //RTC mode register
#define RTC_FR_100th_enable   0x80  //enable 100th second mode
#define RTC_FR_I1ps           0x20  //Periodic interrupt once per second
#define RTC_FR_I1pm           0x40  //Periodic interrupt once per minute
#define RTC_FR_I1ph           0x60  //periodic interrupt once per hour
#define RTC_FR_RTCM           0x10  //100th realtime 0 , 1 stop-watch mode
#define RTC_FR_STOPM          0x08  //RTC stop controlled by STOP bit or TS
#define RTC_FR_32768          0x00        //32768 Frequency selection
#define RTC_FR_16384          0x01        //
#define RTC_FR_8192           0x02        //
#define RTC_FR_4096           0x03        //
#define RTC_FR_2048           0x04        //
#define RTC_FR_1024           0x05        //
#define RTC_FR_1              0x06        //
#define RTC_FR_0              0x07        //
//
//--- PCF85263 RTC WatchDog Register
#define RTC_WD                0x2D        //WatchDog register
#define RTC_WD_WDM            0x80        //1 = repeat mode
#define RTC_WD_WDR            0x7C        //WatchDog register bits 0-1f
#define RTC_WD_WDS            0x03        //WatchDog step size
#define RTC_WD_WDS4s          0x00        // 4 seconds clock (.25hz)
#define RTC_WD_WDS1s          0x01        // 1 second clock (1hz) 
#define RTC_WD_WDS_4          0x02        //1/4 second clock (4hz)
#define RTC_WD_WDS_16         0x03        //1/16 second clock (16hz)
//
//--- PCF85263 RTC Ram location ---
#define RTC_RAM               0x2C        //8-bit ram location
//
//--- PCF85263 INTA_enable Register ---
#define RTC_INTA_enable       0x29        //INTA interrupt enable Regiater
#define RTC_INTB_enable       0x2A        //INTB interrupt enable Register
#define RTC_INT_ILP           0x80        //level or pulse mode(1-level 0=pulse)
#define RTC_INT_PIE           0x40        //Periodic interrupt enable
#define RTC_INT_OIE           0x20        //offset correction interrupt enable
#define RTC_INT_A1IE          0x10        //Alarm 1 interrupt enable
#define RTC_INT_A2IE          0x08        //Alarm 2 interrupt enable
#define RTC_INT_TSRIE         0x04        //Timestamp register interrupt enable
#define RTC_INT_BSIE          0x02        //Battery switch interrupt enable
#define RTC_INT_WDIE          0x01        //WatchDog interrupt enable
//
//--- PCF85263 RTC Pin_IO register ---
#define RTC_Pin_IO            0x27        //Pin_IO pin input output control register
#define RTC_Pin_IO_CLKPM      0x80        //1=disable CLK pin
#define RTC_Pin_IO_TSPULL     0x40        //pullup 0=80k 1=40k
#define RTC_Pin_IO_TSL        0x20        //TS sense 0=active high 1=low
#define RTC_Pin_IO_TSPM0      0x00        //INTB disable
#define RTC_Pin_IO_TSPM1      0x04        //INTB output
#define RTC_Pin_IO_TSPM2      0x08        //INTB Clock output
#define RTC_Pin_IO_TSPM3      0x0C        //INTB input mode
#define RTC_Pin_IO_INTAPM0    0x00        //INTA CLK output mode
#define RTC_Pin_IO_INTAPM1    0x01        //INTA Battery mode indication
#define RTC_Pin_IO_INTAPM2    0x02        //INTA output
#define RTC_Pin_IO_INTAPM3    0x03        //INTA Hi-Z
//
int cmd_trtc() {                                                          // testing RTC entry point
  RTCregW(RTC_FR, RTC_FR_0);                                           //set output pulse rate
  Serial.print("rtc value = "); Serial.println(RTCregR(RTC_WD), HEX);  //print register
  RTCregW(RTC_WD, RTC_WD_WDR + RTC_WD_WDM + RTC_WD_WDS1s);            // Set values in watchdog register
  Serial.print("rtc value = "); Serial.println(RTCregR(RTC_WD), HEX);  //print register
  RTCregW(RTC_Pin_IO, RTC_Pin_IO_INTAPM2);                             //Set INTA output to INTA
  Serial.print("RTC_Pin_IO value = "); Serial.println(RTCregR(RTC_Pin_IO), HEX); //print INTA register
  RTCregW(RTC_INTA_enable, RTC_INT_WDIE);                              //enable Watch Dog interrupts on A
  Serial.print("INTA value = "); Serial.println(RTCregR(RTC_INTA_enable), HEX); //INTA enable register

  /*
     Wire.begin();                                    //begin I2C operation
     Wire.beginTransmission(PCF85263address);         //address the RTC
     Wire.write(byte (RTCmode));                      //point to register 0
     Wire.write(byte (RTC1+RTCrealtime));             //point to register 0
     Wire.endTransmission();
  */
  return 0;
}
//
//----------------------------------------------------------
//
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//  Init SD card with operational and system files
//
int   initSD() {
  //---------------------------------------------------------------
  //   SD init file  and  Boot logging
  //
  //  Set up array of data that needs to be setup each time power on
  //  or new power up and periodly updated with information
  //
  FsDateTime::setCallback(dateTime);            //set time and date for file
  Serial.println("SD init");
  Logfile = SD.open("syslog.txt", FILE_WRITE);
  if (Logfile) {
    Serial.println("syslog.txt created");
    Logfile.print("\r\nBooted ");
    readPCF85263();
    for (int x = 12; x < sizeof(text_time); x++) {
      Logfile.print(text_time[x]);
    }
    Logfile.println();
  } else {
    Serial.println("\r\nLog file on Boot did not open");
  }
  Logfile.close();
  Serial.println("Logfile written");
  //
  //-----------------  erase LastClkRead.txt file --------------
  /*
    //
    //args[1] == NULL;                                    //to erase last mission clock read file
    strcat(args[1],"LastClkRead.txt");                    //need to reset it to now time
    cmd_erase();          //erase the file
      Serial.println("LastClkRead.txt file erased");
    //
    //---------------- Setup LastClkRead.txt is power on ----------
    //

    File LastClkRead = SD.open("LastClkRead.txt", FILE_WRITE);   //
      if (LastClkRead){
          Serial.println("creating power up time reference file LastClkRead.txt");
          DateTime now = rtc.now();
          str=String(now.unixtime());
          Serial.println(str);
          str.toCharArray(text_buf,11);
          LastClkRead.print(text_buf);

      }
      else {
        Serial.println("can not create LastClkRead file");
      }
      LastClkRead.close();
      Serial.println("New LastClkRead.txt file created");

  */
  return 1;
}

//-------------------------------------------------------------------------------
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//  print stack and heap pointers
//
int  cmd_stackandheap() {
  // get the current stack pointer value
  uint32_t current_sp = __get_MSP();
  // get the end address of the heap
  extern uint32_t __HeapLimit;
  uint32_t heap_end = (uint32_t) &__HeapLimit;

  // print the stack and heap pointer addresses
  Serial.print("Stack Pointer 0x");
  Serial.println(current_sp, HEX);
  Serial.print("Heap Pointer: 0x");
  Serial.println(heap_end, HEX);
  return 0;
}
//
//-----------------------------------------------------------------------------------------
//******** getPicture Function ************
void SPIgetPicture_callback(uint32_t pictureSize, uint16_t packetSize, uint32_t packetStartPosition, byte* packet)
{
  photoFile.write(packet, packetSize);
  //  Serial.write (packet, packetSize);
  writtenPictureSize += packetSize;
  if (writtenPictureSize >= pictureSize) {
    photoFile.flush();
    photoFile.close();
  }
}
//
//------------------------ spicam command -----------------
int cmd_takeSpiphoto() {
  Serial.println("SPI Camera Test Command");
  digitalWrite(SPI_cam_Power, SPIcamON);          //camera power on here
  int x = takeSPI();
  digitalWrite(SPI_cam_Power, LOW); //camera power off here ??????????
  Serial.println(x, HEX);

  return 0;
}
//******************************************************************************************************
/***********************************************************************************************************
   This is a test program for the C329 spi camera
   uses camera select pin 5
   power on off pin 7
   camera hold line pin A6
   sd card select pin A5
     Error  1 = Serial.println("SD initialization failed or Missing SD card");
            2 = Serial.println("could not create file");
            3 = Serial.println("SPI Sync failed");

  int spiphoto(filename,numphoto)()

*/
//
//-----------------------------------------------------------------------------
// take a SPI photo-  SPI file name
//-----------------------------------------------------------------------------
//
int takeSPI() {
  detachInterrupt(digitalPinToInterrupt(SerialIRQin));  //detach the serialin IRQ during SD operations - SD uses IRQ itself
  //
  Serial.print("PCS count = ");                //testing out put
  Serial.println(readintFromfram(PCSaddress));  //testing out put
  Serial.print("PCP count = ");                //testing out put
  Serial.println(readintFromfram(PCPaddress));
  Serial.print("PCD count = ");
  Serial.println(readintFromfram(PCDaddress));
  Serial.print("SPI photo ID = ");
  Serial.println(readbyteFromfram(ID));           //Team ID Letter
  //
  char filenameP[] = "XP00000PSS";        //create filename templet for name SPI, SS = size of file for Queue
  //
  int x = (readintFromfram(PCPaddress));  //to get next photo address
  x++;
  if (x > 99999) {
    x = 0;
  };
  writeintfram(x, PCPaddress);            //x = next photo number
  itoa(x, ascii, 10);                      //convert PCM photo count x to ascii string
  //
  //
  int z = 0;                               //to count number of valid entries in array
  for (int i = 0; ascii[i] != '\0'; i++) {
    z++;
  };  //find null char, z now points to pointer
  uint16_t  y = 7 - z;                     //y=points to lsd of where to place lsd in mission buffer
  for (uint16_t x = 0 ; x < z; x++) {      //how many charators
    filenameP[y] = ascii[x];           //transfer from aacii array to proper location in text_mission
    y++;                               //pointer to text_mission array
  }
  filenameP[0] = readbyteFromfram(ID);      //add team ID letter, place it as first letter of file name
  //
  filenameP[7] = '.';
  filenameP[8] = 'j';
  filenameP[9] = 'p';
  filenameP[10] = 'g';
  filenameP[11] = '\0';

  Serial.println(filenameP);

  photoFile = SD.open(filenameP, FILE_WRITE);
  photoFile.seek(0);
  writtenPictureSize = 0;
  Serial.println("opened picture file");
  //
  if (!photoFile) {
    Serial.println("could not create file");
    return 2;   // Error 2
  }
  Serial.println("card initialized.");
  Serial.print("Logging to: ");
  Serial.println(filenameP);
  //
  delay(1000);                              //power up delay  Camera Boot time  not less< 150ms
  //
  //
  if (!cameraSPI.syncSPI())                  //SPI camera sync
  {
    Serial.println("SPI Sync failed");
    cameraSPI.powerOffSPI();
    return 3;
  }
  else {
    Serial.println("SPI Camera sync");
  }
  //
  //  if (!camera.initialize(Quest_CameraC329::BAUD14400, Quest_CameraC329::CT_JPEG, Quest_CameraC329::PR_160x120, Quest_CameraC329::JR_640x480))
  if (!cameraSPI.initializeSPI(QuestCameraC329SPI::BAUD0, QuestCameraC329SPI::CT_JPEG, QuestCameraC329SPI::PR_160x120, QuestCameraC329SPI::JR_320x240)) //QuestCameraC329SPI::BAUD921600
  {
    Serial.println("SPI Initialize failed");
    return 4;
  }
  else {
    Serial.println("SPI Camera initialized");
  }
  //
  //  if (!camera.setQuality(Quest_CameraC329::QL_BEST))    //Set Quality, amount of compression
  if (!cameraSPI.setQualitySPI(QuestCameraC329SPI::QL_BEST))  //hai
  {
    Serial.println("SPI Set quality failed");
    return 5;
  }
  else {
    Serial.println("SPI Camera quality set");
  }
  //
  if (!cameraSPI.getPictureSPI(QuestCameraC329SPI::PT_JPEG_PREVIEW, &SPIgetPicture_callback))
  {
    Serial.println("SPI Get Picture Failed");
    return 6;
  }
  else {
    Serial.println ("SPI Picture taken");
  }
  if (!cameraSPI.resetSPI(QuestCameraC329SPI::RT_STATE))
    photoFile.close();
  Serial.println("SPI Finished writing data to file");
  //
  Bank0size = writtenPictureSize;     //save the size for later
  strcpy(args[1], filenameP);
  //
  Serial.print("\n\rphotosize = "); Serial.println(Bank0size, HEX);
  Serial.print("filename is = "); Serial.println(args[1]);
  //

  WriteText();
  //
  // char filenameP[] = "XP00000PSS";        //create filename templet for name SPI, SS = size of file for Queue
  // all ok add to host Queue
  //
  filenameP[7] = 'P';
  filenameP[8] = (writtenPictureSize / 256);
  filenameP[9] = (writtenPictureSize % 256);
  filenameP[10] = '\0';
  //
  addFileToQueue(filenameP);    //, writtenPictureSize);
  //
  Serial.println(filenameP);
  //

  EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(0);     //reset the IRQ Flag
  attachInterrupt(digitalPinToInterrupt(SerialIRQin), Hostinterupt, FALLING);

  return 0;
}
//****************************************************************************************
//****************************************************************************************
//
//
int cmd_initfram() {        //to reset all counter in fram to proper values
  initfram();
  DateTime now = rtc.now();                   //get time now
  currentunix = (now.unixtime());             //get current unix time, don't count time not flying
  writelongfram(currentunix, PreviousUnix);   //set fram Mission time start now
  writelongfram(0, CumUnix);                  //set cumulitive to 0
  return 0;
}
//
//
//****************************************************************************************
//   File Queue operations here, Queue is in fram for robust operation
//========================================================================================
//  Need to init Queue
//
//char testentry["xx00000.jpg",2048];
//
//char testque[
//
int cmd_initQueue() {
  InitQue();
  addFileToQueue("xx00000.jpgxx");
  addFileToQueue("xx00000.jpgxx");
  addFileToQueue(args[1]);
  addFileToQueue(filenameS);


  return 0;

}
//
int cmd_listQue() {
  listQue();
  return 0;
}
//
//  check Queue to see if a file is ready to send to Host
//
//
//
//  Add a file and length to the Queue;
//
//
//
//  get Queue status
//
int getQstatus() {
  if (getQueStatus() == 0) {
    Serial.println("Queue has file");
  }
  else {
    Serial.println("Queue has NO file");
  }
  return getQueStatus();
}
//
//=======================================================================================
int SystemSetup() {

  Serial.setTimeout(50000);                        //timeout to 5 seconds
  Serial.println("System Module Setup");
  Serial.print("What is you Module ID code? = ");

  //
  while (Serial.available()) { // Check if any data is available to read
    uint8_t inputChar = Serial.read();  // Read the incoming character
  }

  //=========================================================================
  uint8_t inputChar = Getchar();
  Serial.print(">>>>>>>>>>>>>>>>>1st Charator = "); Serial.println(inputChar, HEX);
  if (inputChar != 0xFE) {
    writebytefram(inputChar, ID);
  } else {
    return inputChar;                    //timeout abort
  }
  inputChar = Getchar();
  Serial.print(">>>>>>>>>>>>>>>>>2nd Charator = "); Serial.println(inputChar, HEX);
  if (inputChar != 0xFE) {
    writebytefram(inputChar, ID + 1);  //
  } else {
    return inputChar;                    //timeout abort
  }

  ReadSetup();

  return inputChar;
}


//===================================================================================
uint8_t Getchar() {
  uint16_t TIMEOUT_DURATION = 50000;
  unsigned long startTime = millis();  // Initialize the start time

  if (Serial.available()) {             // Check if any data is available to read
    uint8_t inputChar = Serial.read();  // Read the incoming character
    Serial.print(">>>>>>>>>>>>>>>>>1st Charator = "); Serial.println(inputChar, HEX);
    Serial.print(inputChar);
    return inputChar;
  } else {
    if (millis() - startTime >= TIMEOUT_DURATION) {
      // Handle the timeout abort here
      // For example, you can print an error message and take appropriate actions
      Serial.println("Timeout occurred");
      // Perform necessary cleanup or return to the calling code
      Serial.println("Timeout occurred");    // Timeout occurred
      return 0xFF;
    }
  }
  return 0;
}

//=============================================================================
//
int ReadSetup() {
  Serial.print("\rthis Module ID is = "); Serial.println(readIDfram());
  return 0;

}
//
//
//*****************************************************************************
//
//




//
//
