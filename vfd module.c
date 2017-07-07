/************************************************
* A VFD module that displays information based on
  data from UART
* Due to limited I/O, UART TX pin on the micro is
  used as an I/O, therefore the module won't reply

Command format: (10 bytes in each command)
-----------------------------------------------------------
0xff 0x00 0x00 0xff digit1 digit2 digit3 digit4 digit5 0xff
-----------------------------------------------------------
display mapping:
================without triangles=================
0x00  -  0  |  0x01  -  1  |  0x02  -  2  |  0x03  -  3  |  0x04  -  4
0x05  -  5  |  0x06  -  6  |  0x07  -  7  |  0x08  -  8  |  0x09  -  9
0x0a  -  A  |  0x0b  -  b  |  0x0c  -  C  |  0x0d  -  c  |  0x0e  -  d
0x0f  -  E  |  0x10  -  F  |  0x11  -  h  |  0x12  -  J  |  0x13  -  L
0x14  -  n  |  0x15  -  o  |  0x16  -  P  |  0x17  -  q  |  0x18  -  r
0x19  -  u  |  0x1a  -  y  |  0x1b  - [1] |  0x1c  - [2]							*[1]:Top triangle only, [2]:Bottom triangle only
==============bottom triangles only===============
0x20  -  0  |  0x21  -  1  |  0x22  -  2  |  0x23  -  3  |  0x24  -  4
0x25  -  5  |  0x26  -  6  |  0x27  -  7  |  0x28  -  8  |  0x29  -  9
0x2a  -  A  |  0x2b  -  b  |  0x2c  -  C  |  0x2d  -  c  |  0x2e  -  d
0x2f  -  E  |  0x30  -  F  |  0x31  -  h  |  0x32  -  J  |  0x33  -  L
0x34  -  n  |  0x35  -  o  |  0x36  -  P  |  0x37  -  q  |  0x38  -  r
0x39  -  u  |  0x3a  -  y
===============top triangles only=================
0x40  -  0  |  0x41  -  1  |  0x42  -  2  |  0x43  -  3  |  0x44  -  4
0x45  -  5  |  0x46  -  6  |  0x47  -  7  |  0x48  -  8  |  0x49  -  9
0x4a  -  A  |  0x4b  -  b  |  0x4c  -  C  |  0x4d  -  c  |  0x4e  -  d
0x4f  -  E  |  0x50  -  F  |  0x51  -  h  |  0x52  -  J  |  0x53  -  L
0x54  -  n  |  0x55  -  o  |  0x56  -  P  |  0x57  -  q  |  0x58  -  r
0x59  -  u  |  0x5a  -  y
================both triangles on=================
0x60  -  0  |  0x61  -  1  |  0x62  -  2  |  0x63  -  3  |  0x64  -  4
0x65  -  5  |  0x66  -  6  |  0x67  -  7  |  0x68  -  8  |  0x69  -  9
0x6a  -  A  |  0x6b  -  b  |  0x6c  -  C  |  0x6d  -  c  |  0x6e  -  d
0x6f  -  E  |  0x70  -  F  |  0x71  -  h  |  0x72  -  J  |  0x73  -  L
0x74  -  n  |  0x75  -  o  |  0x76  -  P  |  0x77  -  q  |  0x78  -  r
0x79  -  u  |  0x7a  -  y
*************************************************
Ver 2.0
edited on 20/Oct/2015
edited on 20/May/2016
edited on 16~21/Jun/2016 <- all the old days without git!
*************************************************
update log:
21/Jun/2016
redesigned and rewrote the logic to check data integrity;
fix the potential problem may be caused by a corrupted command
************************************************/
#include <stc12c5204ad.h>

/* define I/Os, low effective */
sbit a = P1^0;
sbit b = P1^1;
sbit c = P1^2;
sbit d = P1^3;
sbit e = P1^4;
sbit f = P1^5;
sbit g = P1^6;
sbit bottomArrowOff = P1^7;	// they are the litte triangles, using 
sbit topArrowOff = P3^1;	// "triangle" as variable name is a bit too long..

sbit digit1 = P3^2;
sbit digit2 = P3^3;
sbit digit3 = P3^4;
sbit digit4 = P3^5;
sbit digit5 = P3^7;

unsigned char RxData;
unsigned char RxPos;
unsigned char RxBuff[5];

const unsigned char DisplayDict[] =
	{
	0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
	//0    1    2    3    4    5    6    7    8    9  <- actual display
	//0    1    2    3    4    5    6    7    8    9  <- index number
	
	0x88,0x83,0xc6,0xa7,0xa1,0x86,0x8e,0x8b,0xf1,0xc7,
	//A    b    C    c    d    E    F    h    J    L
	//10   11   12   13   14   15   16   17   18   19
	
	0xab,0xa3,0x8c,0x98,0xaf,0xe3,0x91,0x3f,0x3f,0x3f,0x3f,0x3f,	// 0x3f is for filling te gap
        //n    o    P    q    r    u    y    -    -    -    -    -
	//20   21   22   23   24   25   26   27   28   29   30   31
// ================================================== //
	0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10,
	//0    1    2    3    4    5    6    7    8    9
	//32   33   34   35   36   37   38   39   40   41
	
	0x08,0x03,0x46,0x27,0x21,0x06,0x0e,0x0b,0x71,0x47,
	//A    b    C    c    d    E    F    h    J    L
	//42   43   44   45   46   47   48   49   50   51
	
	0x2b,0x23,0x0c,0x18,0x2f,0x63,0x11,0x3f,0x3f,0x3f,0x3f,0x3f
	//n    o    P    q    r    u    y
	//52   53   54   55   56   57   58
        // same as above, with only lower arrow being turned on
// ================================================== //
};
	
bit RxDone;

void delay(unsigned int);
void UART_init(void);
unsigned char UART_Rx(unsigned char RxPos);
void DisplayOneDigit(unsigned char, unsigned char);
void display(unsigned char DisplayBuff[]);

void UART_R(void) interrupt 4 using 1
{
	if(RI) {
		RI = 0;
		RxData = SBUF;  // unload received data
		RxPos = UART_Rx(RxPos);
		if(RxPos > 9)	// receiving one command finished
			{
				RxPos = 0;
				RxDone = 1;
			}
	}
}

/*********************************************************************************************************************main*/
void main(void)
{
	unsigned char RxPos = 0;
	unsigned char DisplayBuff[5];
	unsigned char i;  // for iterations
	
	UART_init();
	for(i=0;i<5;i++)
		DisplayBuff[i] = 27;	// initial display set to "- - - -"
	
	while(1)
	{
		if(RxDone)
		{
			RxDone = 0;
			for(i=0;i<5;i++)
				DisplayBuff[i] = RxBuff[i];
		}
		
		display(DisplayBuff);
		
	}
}

/*****************receive data******************
arg = RxPos, indicates which byte this function is checking
ret = RxPos
* handles the command receiving and
  does the integrity check
***********************************************/
unsigned char UART_Rx(unsigned char RxPos)
{
	switch(RxPos)
	{
		case 0:
			if(RxData == 0xff)	RxPos++;	// first byte matched?
			break;
			
		case 1:
			if(RxData == 0x00)	RxPos++;	// second byte matched?
			else if(RxData == 0xff)	RxPos = 1;	// try to align
			else	RxPos = 0;
			break;
			
		case 2:
			if(RxData == 0x00)	RxPos++;	// third byte matched?
			else if(RxData == 0xff)	RxPos = 1;	// try to align
			else	RxPos = 0;
			break;
			
		case 3:
			if(RxData == 0xff)	RxPos++;	// fourth byte matched?
			else	RxPos = 0;
			break;
			
		case 4:
			if(RxData >= 0x00 && RxData < 0x1d || 	// error checking
				 RxData >= 0x20 && RxData <= 0x3a ||
				 RxData >= 0x40 && RxData <= 0x5a ||
				 RxData >= 0x60 && RxData <= 0x7a)
			{
				RxBuff[0] = RxData;
				RxPos++;
			}
			else if(RxData == 0xff)	RxPos = 1;	// try to align
			else	RxPos = 0;
			break;
			
		case 5:
			if(RxData >= 0x00 && RxData <= 0x1c ||	// error checking
				 RxData >= 0x20 && RxData <= 0x3a ||
				 RxData >= 0x40 && RxData <= 0x5a ||
				 RxData >= 0x60 && RxData <= 0x7a)
			{
				RxBuff[1] = RxData;
				RxPos++;
			}
			else if(RxData == 0xff)	RxPos = 1;	// try to align
			else	RxPos = 0;
			break;
			
		case 6:
			if(RxData >= 0x00 && RxData <= 0x1c ||	// error checking
				 RxData >= 0x20 && RxData <= 0x3a ||
				 RxData >= 0x40 && RxData <= 0x5a ||
				 RxData >= 0x60 && RxData <= 0x7a)
			{
				RxBuff[2] = RxData;
				RxPos++;
			}
			else if(RxData == 0xff)	RxPos = 1;	// try to align
			else	RxPos = 0;
			break;
			
		case 7:
			if(RxData >= 0x00 && RxData <= 0x1c ||	// error checking
				 RxData >= 0x20 && RxData <= 0x3a ||
				 RxData >= 0x40 && RxData <= 0x5a ||
				 RxData >= 0x60 && RxData <= 0x7a)
			{
				RxBuff[3] = RxData;
				RxPos++;
			}
			else if(RxData == 0xff)	RxPos = 1;	// try to align
			else	RxPos = 0;
			break;
			
		case 8:
			if(RxData >= 0x00 && RxData <= 0x1c ||	// error checking
				 RxData >= 0x20 && RxData <= 0x3a ||
				 RxData >= 0x40 && RxData <= 0x5a ||
				 RxData >= 0x60 && RxData <= 0x7a)
			{
				RxBuff[4] = RxData;
				RxPos++;
			}
			else if(RxData == 0xff)	RxPos = 1;	// try to align
			else	RxPos = 0;
			break;
			
		case 9:
			if(RxData == 0xff)	RxPos++;	// last byte matched?
			else	RxPos = 0;
			break;
	}
	return RxPos;
}
	
/****************delay function*****************/
void delay (unsigned int a){ // delay 1ms
	unsigned int i;
	while( --a != 0)
		for(i = 0; i < 600; i++); // 600 for 12MHz crystal
}

/**********************UART*********************/
void UART_init (void){
	EA = 1;
	ES = 1;
	TMOD = 0x20;
	SCON = 0x50;
	TH1 = 0xF3;
	TL1 = 0xF3;
	PCON = 0x80;
	TR1 = 1;
}

/****************display a digit*****************/
void DisplayOneDigit(unsigned char NUM,unsigned char ADDR)
{
	
	switch(ADDR)	// select the digit to display
	{
		
		case 0:
			digit1 = 0;
		  digit2 = digit3 = digit4 = digit5 = 1;
		  break;
		
		case 1:
			digit2 = 0;
		  digit1 = digit3 = digit4 = digit5 = 1;
		  break;
		
		case 2:
			digit3 = 0;
		  digit1 = digit2 = digit4 = digit5 = 1;
		  break;
		
		case 3:
			digit4 = 0;
		  digit1 = digit2 = digit3 = digit5 = 1;
		  break;
		
		case 4:
			digit5 = 0;
		  digit1 = digit2 = digit3 = digit4 = 1;
		  break;
		
		default:
			digit1 = digit2 = digit3 = digit4 = digit5 = 0;	 // just in case an error occurred
		  g = 0;
		  break;
	}
	
	if(NUM < 0x40)
	{
		P1 = DisplayDict[NUM];
		topArrowOff = 1;
	}
	
	else if(NUM > 0x3f && NUM < 0x7f)
	{
		topArrowOff = 0;
		P1 = DisplayDict[NUM - 0x40];
	}
	
	else	// just in case an error occurred
	{
		topArrowOff = 1;
		P1 = 0xff;
		g = 0;
		digit1=digit2=digit3=digit4=digit5=0;
	}
}

void display(unsigned char DisplayBuff[])
{
	unsigned char i;
	for(i=0;i<5;i++)
	{
		DisplayOneDigit(DisplayBuff[i], i);
		delay(5);	// delay 5ms
	}
}
