#include "STC15Wxx_STC15Fxx.h"
#include "intrins.h"
#include "math.h"
#include "LT89XX_DRV.h"

#define cycle 1390L
#define FOSC 11059200L
#define BAUD 115200
#define NONE_PARITY	0
#define ODD_PARITY 1
#define EVEN_PARITY 2
#define MARK_PARITY 3
#define SPACE_PARITY 4
#define PARITYBIT NONE_PARITY
#define S2RI 0x01
#define S2TI 0x02
#define TX_DAT1 0x01	 		
#define TX_DAT2 0x02     
#define INT_RX  0x03     
#define RX_MOD  0x00     
#define FALSE 0
#define TRUE 1

//PWM DECLARATION
sbit test = P2^5;
sbit PWM5 = P2^3;
sbit PWM2 = P3^7;
sbit PWM3 = P2^1;
sbit PWM4 = P2^2;
unsigned char check_data;
unsigned char idata RowH, RowL, PitchL, PitchH, YawL, YawH,TH, TL;
double angle_x, angle_y, angle_z, temperature;
unsigned int idata num;
unsigned char receive_done = 0;
bit busy;
unsigned char index_state; 
unsigned char useful_data[33];
unsigned char idata RBUF[32];
unsigned int flag;
unsigned int counter = 0;
unsigned char time_5ms_flag;
unsigned char RegH;
unsigned char RegL;              //Used to store the Registor Value which is read.
unsigned char tran_flag=0;
unsigned char xdata data_bag[8];
unsigned char send_done=0;

// Function definition: 
//Delay function
void Delay_10us(unsigned char T )
{
  for(;T>0;T--)
  {
  _nop_();
  _nop_();
  _nop_();
  _nop_();
  _nop_();
  _nop_();
  _nop_();
  _nop_();  
  _nop_();
  _nop_();
  }
}

void Delay_ms(unsigned short T)
{
  for(;T>0;T--)
   {
   Delay_10us(100);
   }
}
//LT89XX SPI 
/*****************************************/
// SPI Write and Read ,as well as Read8  // 
/*****************************************/

void SPI_WriteReg(unsigned char addr, unsigned char H, unsigned char L)
{
  int i;
  SPI_SS = 0;
  for(i = 0; i < 8; ++ i)
	{
	  MOSI = addr & 0x80;
	  SPI_CLK = 1;                       //capturing at the down side.
	  SPI_CLK = 0;
	  addr = addr << 1;                    //There is no Delay here. determines the rate of SPI.
	}
  for(i = 0; i < 8; ++i)                 //Write H
	{
	  MOSI = H & 0x80;
	  SPI_CLK = 1;
	  SPI_CLK = 0;
	  H = H << 1;
	}
  for(i = 0; i < 8; ++i)                 //Write L
	{  
	  MOSI = L & 0x80;
	  SPI_CLK = 1;
	  SPI_CLK = 0;
	  L = L << 1;
	}
  SPI_SS = 1;
}

void SPI_ReadReg(unsigned char addr)
{
  int i;
  SPI_SS = 0;
  addr += 0x80;                    //when reading a Register,the Address should be added with 0x80.

  for(i = 0; i < 8; ++ i)
  {
	 MOSI = addr & 0x80;
	 SPI_CLK = 1;
	 SPI_CLK = 0;
	 addr = addr << 1;                      //Move one bit to the left.
  }
  for(i = 0; i < 8; ++ i)
  {
	SPI_CLK = 1;                         //transmit at the up side.
	SPI_CLK = 0;
  	RegH = RegH << 1;  
	RegH |= MISO;
  }
  for(i = 0; i < 8; ++ i)
  {
	SPI_CLK = 1;                         //transmit at the up side.
	SPI_CLK = 0;
  RegL = RegL << 1; 
	RegL |= MISO;
  }
  SPI_SS = 1;
}

void LT8900_Init(void)
{
  RST  = 0;
	Delay_ms(2);
	RST  = 1;
	Delay_ms(5);
	PKT = 1;

	SPI_WriteReg( 0, 0x6f, 0xef );
	SPI_WriteReg( 1, 0x56, 0x81 );
	SPI_WriteReg( 2, 0x66, 0x17 );
	SPI_WriteReg( 4, 0x9c, 0xc9 );
	SPI_WriteReg( 5, 0x66, 0x37 );
	SPI_WriteReg( 7, 0x00, 0x00 );							  //channel is 2402Mhz
	SPI_WriteReg( 8, 0x6c, 0x90 );
	SPI_WriteReg( 9, 0x48, 0x00 );			  				  //PA -12.2dbm
	SPI_WriteReg(10, 0x7f, 0xfd );
	SPI_WriteReg(11, 0x00, 0x08 );
	SPI_WriteReg(12, 0x00, 0x00 );
	SPI_WriteReg(13, 0x48, 0xbd );
	SPI_WriteReg(22, 0x00, 0xff );
	SPI_WriteReg(23, 0x80, 0x05 );
	SPI_WriteReg(24, 0x00, 0x67 );
	SPI_WriteReg(25, 0x16, 0x59 );
	SPI_WriteReg(26, 0x19, 0xe0 );
	SPI_WriteReg(27, 0x13, 0x00 );
	SPI_WriteReg(28, 0x18, 0x00 );
	SPI_WriteReg(32, 0x58, 0x00 );
	SPI_WriteReg(33, 0x3f, 0xc7 );
	SPI_WriteReg(34, 0x20, 0x00 );
	SPI_WriteReg(35, 0x0a, 0x00 );				    
	SPI_WriteReg(36, 0x03, 0x80 );
	SPI_WriteReg(37, 0x03, 0x80 );
	SPI_WriteReg(38, 0x5a, 0x5a );
	SPI_WriteReg(39, 0x03, 0x80 );
	SPI_WriteReg(40, 0x44, 0x02 );
	//SPI_WriteReg(41, 0xb4, 0x00 );	                /*CRC is ON; scramble is OFF; AUTO_ACK is OFF*/
	SPI_WriteReg(41, 0xb8, 0x00 );	                    /*CRC is ON; scramble is OFF; AUTO_ACK is ON*/
    #ifdef LT8910
	SPI_WriteReg(42, 0xfd, 0xff );					
    #else
	SPI_WriteReg(42, 0xfd, 0xb0 );					
	#endif
	
	SPI_WriteReg(43, 0x00, 0x0f );
}

void Init_Timer(void)
{
  time_5ms_flag=FALSE;		             
}

//UART initialization
void uart_ini()
{
  #if(PARITYBIT == NONE_PARITY)
  	 S2CON = 0x50;
  #elif(PARITYBIT == ODD_PARITY)||(PARITYBIT == EVEN_PARITY)||(PARITYBIT == MARK_PARITY)
  	 S2CON = 0xda;
  #elif(PARITYBIT == SPACE_PARITY)
  	 S2CON = 0xd2;
  #endif
  T2L = (65536-(FOSC/4/BAUD));
  T2H = (65536-(FOSC/4/BAUD))>>8;
  AUXR = 0x14;
//  AUXR |= 0x01;
  EA = 1;
  IE2 = 1;
}
//receive 1 bit of data in every interrupt

void Uart() interrupt 8 using 1
{
  unsigned int i;
  if(receive_done == 0)
  {
    if (S2CON & S2RI)
	  {
      S2CON &= ~S2RI;
	    if((S2BUF == 0x55)||(flag == 1))
	    {
	      useful_data[counter] = S2BUF; //data output
	      counter ++;
	      flag = 1;
	      if(counter>32)
	      {
	        counter = 0;
		      flag = 0;
		      receive_done = 1;
	      }
	    }
    }
  }
	
	if(receive_done == 1 )
	{
    for(i=0;i<33;i++)
	  { 
	    if(useful_data[i]==0x55)
	   	{
		    check_data = 0;
		  }
	    if((check_data == 0)&&(useful_data[i+1]==0x53))
	    {
		    RowL = useful_data[i+2];
		    RowH = useful_data[i+3];
		    PitchL = useful_data[i+4];
		    PitchH = useful_data[i+5];
		    YawL = useful_data[i+6];
		    YawH = useful_data[i+7];
			  TL = useful_data[i+8];
			  TH = useful_data[i+9];
			}
			if(send_done == 0)
			{ 
   			data_bag[0] = useful_data[i+2];
		    data_bag[1] = useful_data[i+3];
		    data_bag[2] = useful_data[i+4];
		    data_bag[3] = useful_data[i+5];
		    data_bag[4] = useful_data[i+6];
		    data_bag[5] = useful_data[i+7];
			  data_bag[6] = useful_data[i+8];
			  data_bag[7] = useful_data[i+9];
			  send_done = 1;
				temperature = ((short)((TH<<8)|TL))/340.0 + 36.25;		  
	      angle_x = ((short)(RowH<<8|RowL))/32768.0*180 + 90;
				angle_y = ((short)(PitchH<<8|PitchL))/32768.0*180 + 91;
	      angle_z = ((short)(YawH<<8|YawL))/32768.0*180 +90;
			} 
	  }	
		receive_done = 0;	
  }	
	  if(S2CON & S2TI)
  {
    S2CON &= ~S2TI;
  }
}

void pwmini()			//pwm initialization
{
  P0M0 = 0x00;
  P0M1 = 0x00;
  P1M0 = 0x00;
  P1M1 = 0x00;
  P2M0 = 0x00;
  P2M1 = 0x00;
  P3M0 = 0x00;
  P3M1 = 0x00;
  P4M0 = 0x00;
  P4M1 = 0x00;
  P5M0 = 0x00;
  P5M1 = 0x00;
  P_SW2  |= 0x80;
  PWMCFG = 0x00;
  PWMCKS = 0x0f;
  PWMC = cycle;
  PWM2CR = 0x00;
  PWM2T1 = 0x0001;
  PWM2T2 = 0;
  PWM3CR = 0x00;
  PWM3T1 = 0x0001;
  PWM3T2 = 0;
  PWM4CR = 0x00;
  PWM4T1 = 0x0001;
  PWM4T2 = 0;
  PWM5CR = 0x00;
  PWM5T1 = 0x0001;
  PWM5T2 = 0;
  PWMCR |= 0x80;
  P_SW2 &= ~0x80;
}

void pwm2(unsigned int duty)
{
  P_SW2|=0X80;
  PWM2T1 = 0X0001;
  PWM2T2 = cycle * duty /1000;
  P_SW2 &= ~0X80;
  PWMCR |= 0X01;
}

void pwm3(unsigned int duty)
{
  P_SW2|=0x80;
  PWM3T1 = 0X0001;
  PWM3T2 = cycle * duty /1000;
  P_SW2 &= ~0X80;
  PWMCR |= 0x02;
}

void pwm4(unsigned int duty)
{
  P_SW2|=0x80;
  PWM4T1 = 0X0001;
  PWM4T2 = cycle * duty /1000;
  P_SW2 &= ~0X80;
  PWMCR |= 0x04;
}

void pwm5(unsigned int duty)
{
  P_SW2|=0x80;
  PWM5T1 = 0X0001;
  PWM5T2 = cycle * duty/1000;
  P_SW2 &= ~0X80;
  PWMCR |= 0x08;
}

void main()
{
//declaration
	unsigned int idata num2,num3,num4,num5;
	unsigned char i,j,count, data_count=0;
  num2 = 250;
	num3 = 250;
	num4 = 250;
	num5 = 250;
  
//Initialization
	uart_ini();
	pwmini();		
  Init_Timer();
  Delay_ms(10);
  LT8900_Init();
  #ifdef LT8910
  SPI_WriteReg(44, 0x10, 0x00);
  SPI_WriteReg(45, 0x05, 0x52);	
  #endif
	//-----------------
  SPI_ReadReg(40);
  if (RegH==0x44 && RegL==0x02)
	{}
  count=0; 
	//index_state = INT_RX;
	index_state = TX_DAT1;
  while(1)
  {
	 //4 PWM output    
   pwm2(num2);
   pwm3(num3);
   pwm4(num4);
   pwm5(num5);
   switch(index_state)
   {
	  /////////////////////////////////////////////////////////
	  case INT_RX:
  	  SPI_WriteReg(52, 0x80, 0x80);
      SPI_WriteReg(7, 0x00, 0x80 + 0x20);	
	    index_state=RX_MOD;
	  break;

	  /////////////////////////////////////////////////////////	
    case RX_MOD:
	  //checking if receive the data
		 if(PKT == 1)
	   {
	     i=0;
		   SPI_ReadReg(50);
	     j=RegH;//read the data
		   while(i<j)
		  {
		   //read data
		    SPI_ReadReg(50);
		    RBUF[i++]=RegH;
		    RBUF[i++]=RegL;
		    if(i==32)
		    break;
		  }  
	    //Test CRC
		  SPI_ReadReg(48);
		  if((RegH&0x80)==0)
		  {
		    if(RBUF[0]==1)
			  {
				  test = ~test;
		  	  num2 = num2 + 10;
			 	  num3 = num3 + 10;
				  num4 = num4 + 10;
				  num5 = num5 + 10;
				  Delay_ms(20);
		 	  }
		    else if(RBUF[0]==2)
		 	  {
			    count=0;
			    num2 = num2 - 10; 
				  num3 = num3 - 10;
				  num4 = num4 - 10;
				  num5 = num5 - 10;
				  Delay_ms(20);
			  }
			
	    }
		 //initialize to receive mode
			 index_state=INT_RX;
     }
		 break;

		 /////////////////////////////////////////////////////////////////
		 case TX_DAT1:
		   //send the data
		    SPI_WriteReg(52, 0x80, 0x80);
		    SPI_WriteReg(50, 9,0);//including the first bit which indicating the length of the data
		  //write the data into reg	
		    SPI_WriteReg(50,temperature,angle_x);
		    SPI_WriteReg(50, angle_y,angle_z);
		    SPI_WriteReg(50, 0,0);			
		    SPI_WriteReg(50, 0,0);
       //start sending
		   SPI_WriteReg(7, 0x01, 0x20);
		   while (PKT == 0); //waiting for completing
			 send_done = 0;
		   SPI_ReadReg(52);
		   if((RegH& 0x3F)==0)
		   	{
		   	  count=0;
		   	}
			//	index_state = INT_RX;
		   break;

		  default:break;
		}
	} 
} 