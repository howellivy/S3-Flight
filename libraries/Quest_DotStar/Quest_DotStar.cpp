/*
DotStar Library 20220621 hai
*/
//
//void DotStarsend(uint8_t Bright, uint8_t Blue, uint8_t Green, uint8_t Red);
//void DotStarSendByte(uint8_t Data);   //Will send MSB first
//
#include <Quest_DotStar.h>

void DotStarInit() {
pinMode(DotStarDATAPIN,OUTPUT);       //Set DataStar data pin to output
digitalWrite(DotStarDATAPIN,HIGH);    //Set DataStar data pin to High
pinMode(DotStarCLOCKPIN,OUTPUT);      //Set DataStar clock pin to output
digitalWrite(DotStarCLOCKPIN,HIGH);   //Set DataStar clock pin to High
//
pinMode(testPIN, OUTPUT);             //
digitalWrite(testPIN,LOW);            //
}

//
int lum = 0x04;   //Base value of brightness for each DotStar LED
//

void DotStarOff(){                           // Off
DotStarsend(0xff,0x00,0x00,0x00);
}
void DotStarRed(){                           // Red
DotStarsend(0xff,0x00,0x00,lum);
}
void DotStarGreen(){                        // Green
DotStarsend(0xff,0x00,lum,0x00);
}

void DotStarYellow(){                       // Yellow
DotStarsend(0xff,0x00,lum,lum);
}

void DotStarBlue(){                     // Blue
DotStarsend(0xff,lum,0x00,0x00);
}

void DotStarMagenta(){                   // Magenta
DotStarsend(0xff,lum,0x00,lum);
}

void DotStarCyan(){                      // Cyan
DotStarsend(0xff,lum,lum,0x00);
}

void DotStarWhite(){                     // White
DotStarsend(0xff,lum,lum,lum);
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