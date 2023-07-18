/*----------------------------------------------------------------------------
// Quest_Test- this is a library that will allow the insert of a pulse on
// different IO lines that can be used to test the time and if a function is
// being called
//
// Pulse13();.......Pulse11();
// 
//
//20230516	hai
*///-------------------------------------------------------------------------
//
//
#include "Quest_test.h"
//
//
void PulseInit(){
     pinMode(13, OUTPUT);  // Define the pin as an output
     pinMode(12, OUTPUT);  // Define the pin as an output
     pinMode(11, OUTPUT);  // Define the pin as an output
     digitalWrite(13, HIGH);
     digitalWrite(13, HIGH);
     digitalWrite(13, HIGH);
}
//
//
void Pulse13()				//pulse 13 / IO7
{	
     uint8_t val = digitalRead(13);    /////////////// pulse  //////////////////////
     digitalWrite(13, !val);    /////////////////  pulse //////////////////////
     digitalWrite(13, val);    //////////////////  pulse //////////////////////
}
void Pulse12()		//pulse 12 / IO6
{	
     uint8_t val = digitalRead(12);    /////////////// pulse  //////////////////////
     digitalWrite(12, !val);    /////////////////  pulse //////////////////////
     digitalWrite(12, val);    //////////////////  pulse //////////////////////
}
void Pulse11()		//pulse 11 / IO5
{	
     uint8_t val = digitalRead(11);    /////////////// pulse  //////////////////////
     digitalWrite(11, !val);    /////////////////  pulse //////////////////////
     digitalWrite(11, val);    //////////////////  pulse //////////////////////
}
void Pulse10()		//pulse 10 / IO4
{	
     uint8_t val = digitalRead(10);    /////////////// pulse  //////////////////////
     digitalWrite(10, !val);    /////////////////  pulse //////////////////////
     digitalWrite(10, val);    //////////////////  pulse //////////////////////
}
void Pulse9()		//pulse  9 / IO3
{	
     uint8_t val = digitalRead(9);    /////////////// pulse  //////////////////////
     digitalWrite(9, !val);    /////////////////  pulse //////////////////////
     digitalWrite(9, val);    //////////////////  pulse //////////////////////
}
void Pulse4()		//pulse  4 / IO2
{	
     uint8_t val = digitalRead(4);    /////////////// pulse  //////////////////////
     digitalWrite(4, !val);    /////////////////  pulse //////////////////////
     digitalWrite(4, val);    //////////////////  pulse //////////////////////
}
//
//
void Pulse13high(){				//set pin high
	digitalWrite(13, HIGH);
}
void Pulse13low(){				//set pin low
	digitalWrite(13, LOW);
}
//
void Pulse12high(){				//set pin high
	digitalWrite(12, HIGH);
}
void Pulse12low(){				//set pin low
	digitalWrite(12, LOW);
}
//
void Pulse11high(){				//set pin high
	digitalWrite(11, HIGH);
}
void Pulse11low(){				//set pin low
	digitalWrite(11, LOW);
}
//
void Pulse10high(){				//set pin high
	digitalWrite(10, HIGH);
}
void Pulse10low(){				//set pin low
	digitalWrite(10, LOW);
}
//
void Pulse9high(){				//set pin high
	digitalWrite(9, HIGH);
}
void Pulse9low(){				//set pin low
	digitalWrite(9, LOW);
}
//
void Pulse4high(){				//set pin high
	digitalWrite(4, HIGH);
}
void Pulse4low(){				//set pin low
	digitalWrite(4, LOW);
}//
//

