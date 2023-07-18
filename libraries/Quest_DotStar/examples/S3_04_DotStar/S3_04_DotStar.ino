// An example demonstrating how to control the adafruit Dot star RGB
// inclulded on board the Istybitsy M4 board.
//
//Use these pin definitions for the ItsyBitsy M4
#define DotStarDATAPIN    8   //Data pin
#define DotStarCLOCKPIN   6   //Clock
#define testPIN           3   //
//
//Ram locatinos to hold the color of the DotStar
uint8_t DotStarBright   = 0xE4;   //
uint8_t DotStarBlue     = 0x00;   //
uint8_t DotStarGreen    = 0x00;   //
uint8_t DotStarRed     = 0x00;   //
//
// prototype
//
//void DotStarsend(uint8_t Bright, uint8_t Blue, uint8_t Green, uint8_t Red);
//void DotStarSendByte(uint8_t Data);   //Will send MSB first
//
//AdaFruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
//


void setup() {
pinMode(DotStarDATAPIN,OUTPUT);       //Set DataStar data pin to output
digitalWrite(DotStarDATAPIN,HIGH);    //Set DataStar data pin to High
pinMode(DotStarCLOCKPIN,OUTPUT);      //Set DataStar clock pin to output
digitalWrite(DotStarCLOCKPIN,HIGH);   //Set DataStar clock pin to High
//
pinMode(testPIN, OUTPUT);             //
digitalWrite(testPIN,LOW);            //
}

void loop() {
  /*
DotStarsend(DotStarBright,DotStarBlue,DotStarGreen,DotStarRed);
DotStarBlue = DotStarBlue+1;
if(DotStarBlue == 0x00){DotStarGreen=DotStarGreen+1;}
if(DotStarGreen == 0x00){DotStarRed=DotStarRed+1;}
delay(1);
if(DotStarRed==0x00){
*/
int lum = 0x04;
delay(2000);
DotStarsend(0xff,0x00,0x00,lum);
delay(500);
DotStarsend(0xff,0x00,lum,0x00);
delay(500);
DotStarsend(0xff,0x00,lum,lum);
delay(500);
DotStarsend(0xff,lum,0x00,0x00);
delay(500);
DotStarsend(0xff,lum,0x00,lum);
delay(500);
DotStarsend(0xff,lum,lum,0x00);
delay(500);
DotStarsend(0xff,lum,lum,lum);
delay(500);
DotStarsend(0xff,0x00,0x00,0x00);
//}
}
//
// Send a Frame of data to DotStar
//
void DotStarsend(uint8_t Bright, uint8_t Blue, uint8_t Green, uint8_t Red){
    for(int i=0;i<4;i++){       //Send start frame
      uint8_t Data=0x00;
      DotStarSendByte(Data);
    }
    DotStarSendByte(Bright);   //Send 4 data bytes
    DotStarSendByte(Blue);
    DotStarSendByte(Green);
    DotStarSendByte(Red);
    for(int i=0;i<4;i++){      //Send Stop frame
       uint8_t Data=0xFF;
       DotStarSendByte(Data);
    }
}
//
//Send a byte to DotStar LED enter with value set to byte to send
//
void DotStarSendByte(uint8_t value){      //Send MSB first
  for(int i=0;i<8;i++){
    if((value & 0x80)==0x80){
      digitalWrite(DotStarDATAPIN, HIGH);   //set data pin to high
    }
    else{
      digitalWrite(DotStarDATAPIN,LOW);     //set data pin to low
    }
    digitalWrite(DotStarCLOCKPIN,LOW);      //set clock to low
    delayMicroseconds(1);                   //wait 1 microsec
    digitalWrite(DotStarCLOCKPIN,HIGH);     //set clock to high
    delayMicroseconds(1);                   //wait 1 microsec
    value = value<<1;                       //shift left 1 bit for next
  }
  digitalWrite(testPIN,HIGH);               //test pin to high
  digitalWrite(testPIN,LOW);                //test pin to low
  }
  
