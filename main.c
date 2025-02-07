/*
 * Digital Medical Gas Alarm System.c
 *
 * Created: 26-08-2023 19:42:51
 * Author : Dott Systems
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include  <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/eeprom.h>
//Macros---------------------------
//pressure threshold---------------------
#define POSITIVE_HIGH_TH 6.0
#define POSITIVE_LOW_TH  2.5
#define NEGATIVE_LOW_TH  400
//Channel macro---------------------------------------
#define MAX_CHANNEL  6
#define MIN_CHANNEl  2
//--------------------------------
//Mode macro-----------------------------------
#define SET_UP_MODE  0x10
#define NORMAL_MODE  0x20
//PV-------------------------------
//Display variable--------------------------------------------------------------
//char display_data[10]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};
//char display_data[10]={0x3F,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
char display_data[10]={0xfc,0x60,0xda,0xf2,0x66,0xb6,0xbe,0xe0,0xff,0xf6};//actual
char mask=0x01;
char led_[4]={0xbf,0xbf,0xbf,0xbf};//Normal=0xbf ,Low=0x7f,High=0xdf	
//-------------------------------------------------------
//sensor reading variable------------------------------
float p1=0.00,p2=0.00,p3=0.00,p4=0.00,p5=0.00;
float np=0.00;
int v1=0,v2=0,v3=0,v4=0,v5=0,v6=0;
float V_sup=5.00;
//fault reading flag--------------
uint8_t fault=0;
uint8_t mute=0;
//Channel ---------------------
uint8_t channel_count=6;
uint8_t n=0;
uint8_t p3_disable=0,p4_disable=0,p5_disable=0;
//System mode---------------------------------
char mode=NORMAL_MODE;
//PVFP-----------------------------
void display_digit(char data);
void io_initialization(void);
void shift_data(void);
void latch_data(void);
void display_value_float(float value);
void display_value_int(int value);
void display_pressure( );
void adc_init();
int adc_read(int ch);
void init_led_disp(void);
void led_disp_update(void);
void fault_detect(void);
void set_channel(void);
//----------------------------------
//ADC functions------------------------------
void adc_init()
{
    ADMUX=(1<<REFS0);   
	//ADMUX = (0<<REFS0) | (0<<REFS1);
	
	// ADC Enable and prescaler of 128
	// 8000000/128 = 62500
	ADCSRA = (1<<ADEN)|(1<<ADPS0);
}
int adc_read(int ch)
{
	// select the corresponding channel 0~7
	// ANDing with '7' will always keep the value
	// of 'ch' between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch;     // clears the bottom 3 bits before ORing
	
	// start single conversion
	// write '1' to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes '0' again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}
//---------------------------------------------------
//Set channel---------------------------------------------
void set_channel(void){
	
	
	
}
//--------------------------------------------------------
//IO initialization---------------------
void io_initialization(void){
	
	//Switch 32
	//Buzzer 28
	DDRB |=(1<<0);
	DDRB |=(1<<1);
	DDRB |=(1<<2);
	//--------------------------
	DDRB |=(1<<5);
	DDRB |=(1<<6);
	DDRB |=(1<<7);
	//-----------------------------
	DDRC |=(1<<1);
	DDRC |=(1<<2);
	DDRC |=(1<<3);
	DDRC |=(1<<4);
	DDRC |=(1<<5);
	//----------------------------
    DDRD &=~(1<<2);
	DDRD |=(1<<3);
	DDRD |=(1<<4);
	DDRD |=(1<<5);
	DDRD |=(1<<6);
	DDRD |=(1<<7);
	//---------------------------
	
	PORTC &=~(1<<5);
}
//led indication init--------------------------------------
void init_led_disp(void){
	
  uint8_t k=0;
  uint8_t j=0;
  PORTC &=~(1<<2);
  PORTC &=~(1<<4);
  PORTC |=(1<<1);
  PORTC |=(1<<3);
  
  
 for(j=0;j<4;j++){
	
  for(k=0;k<8;k++){
			
	if(led_[j] & mask){
			
		PORTB |=(1<<5);
			
		}
		else{
			
		PORTB &=~(1<<5);
			
		}
		
		led_[j]=led_[j]>>1;
		
		PORTB |=(1<<6);
		_delay_us(100);
		PORTB &=~(1<<6);
		
	}
		 	
	}
 	
	 
	PORTB |=(1<<7);
	_delay_us(100);
	PORTB &=~(1<<7);
	
	
}
//Led indication update -------------
void led_disp_update(void)
{
  
     uint8_t k=0;
     uint8_t j=0;
  
  for(j=0;j<4;j++){
	  
	  for(k=0;k<8;k++){
		  
		  if(led_[j] & mask){
			  
			  PORTB |=(1<<5);
			  
		  }
		  else{
			  
			  PORTB &=~(1<<5);
			  
		  }
		  
		  led_[j]=led_[j]>>1;
		  
		  PORTB |=(1<<6);
		  _delay_us(100);
		  PORTB &=~(1<<6);
		  
	  }
	  
  }
  
  
  PORTB |=(1<<7);
  _delay_us(100);
  PORTB &=~(1<<7);

}
//shift bit -----------------
void display_digit(char data){
	uint8_t i=0;
	data=display_data[data];
	for(i=0;i<8;i++){
		if(data & mask){
			PORTD |=(1<<5);
			
		}
		else{
			
			PORTD &=~(1<<5);
			
		}
		data=data>>1;
		shift_data();
	}
	
	
}
//Shift data------------------------
void shift_data(void){
	
	PORTD |=(1<<6);
	_delay_us(100);
	PORTD &=~(1<<6);
	_delay_us(100);
	
}
//----------------------------------
//Latch data------------------------------------
void latch_data(void){
	
	PORTD |=(1<<7);
	_delay_us(100);
	PORTD &=~(1<<7);
	_delay_us(100);
	
	
}
//display float value---------------------------
void display_value_float(float value)
{
 char digit1=0,digit2=0,digit3=0;
	if(value !=0){
 value=value*10;

 int y=(int)value;
 
 if(y<10){
	 
	 digit3=0;
	 digit2=y/10;
	 digit1=y%10;
	 
 }
 else{
	 
	 digit1=y%10;
	 digit3=y/100;
	 y=y/10;
	 digit2=y%10; 
	 
 }

	}
	else{
		digit1=0;
		digit2=0;
		digit3=0;
		
	}
 display_digit((char)digit1);
 display_digit((char)digit2);
 display_digit((char)digit3);

 
}
//----------------------------------------------
//display int-----------------------------------
void display_value_int(int value)
{

	char digit1=0,digit2=0,digit3=0;

	int y=value;
	
	if(y<10){
		
		digit3=0;
		digit2=y/10;
		digit1=y%10;
		
	}
	else{
		
		digit1=y%10;
		digit3=y/100;
		y=y/10;
		digit2=y%10;
		
	}


	
	display_digit((char)digit1);
	display_digit((char)digit2);
	display_digit((char)digit3);

	
}
//---------------------------------------
//display pressure------------------------------
void display_pressure()
{
	
//----------------------
switch(channel_count){
	
	case 2:
	  //00-------p4-----
	  PORTB &=~(1<<0);
	  PORTB &=~(1<<1);
	  PORTB &=~(1<<2);
	  
	  v6=adc_read(0);
	  
	  
	  p4=V_sup*v6;
	  p4=p4/1024;
	  //p4=p4-0.5;
	  p4=p4-0.5;
	  p4=p4/0.396;
	  
	  if(p4<=0){
		  
		  p4=0.00;

	  }
	  //---------------------
	  //01------P1-----------
	  PORTB &=~(1<<1);
	  PORTB |=(1<<0);
	  PORTB &=~(1<<2);
	  
	  v1=adc_read(0);
	  
	  p1=V_sup*v1;
	  p1=p1/1024;
	  //p1=p1-0.5;
	  p1=p1-0.5;
	  p1=p1/0.396;
	  
	  if(p1<=0){
		  
		  p1=0.00;
		  
	  }
	 
	  p5=3.5;
	  p3=3.5;
	  p2=3.5;
	  np=500;
	  n=0;
	  display_value_int(np);
	  display_value_float(p5);
	  display_value_float(p3);
	  display_value_float(p2);
	  display_value_float(p4);
	  display_value_float(p1);
	   break;
    case 3:
	  //00-------p4-----
	  PORTB &=~(1<<0);
	  PORTB &=~(1<<1);
	  PORTB &=~(1<<2);
	  
	  v6=adc_read(0);
	  
	  
	  p4=V_sup*v6;
	  p4=p4/1024;
	  //p4=p4-0.5;
	  p4=p4-0.5;
	  p4=p4/0.396;
	  
	  if(p4<=0){
		  
		  p4=0.00;

	  }
	  //---------------------
	  //01------P1-----------
	  PORTB &=~(1<<1);
	  PORTB |=(1<<0);
	  PORTB &=~(1<<2);
	  
	  v1=adc_read(0);
	  
	  p1=V_sup*v1;
	  p1=p1/1024;
	  //p1=p1-0.5;
	  p1=p1-0.5;
	  p1=p1/0.396;
	  
	  if(p1<=0){
		  
		  p1=0.00;
		  
	  }
	  //---------------------


	  //----------------------
	  //100----np-------------
	  PORTB |=(1<<2);
	  PORTB &=~(1<<1);
	  PORTB &=~(1<<0);
	  
	  v4=adc_read(0);

	  np=V_sup*v4;
	  np=np/1024;
	  np=np-(0.92*5);
	  np=np/0.03826;
	  np=np*7.50062;
	  np=(-1)*np;

	  if(np<=0){
		  
		  np=0;
		  
	  }
	 
	  p5=3.5;
	  p3=3.5;
	  p2=3.5;
	  n=3;
	  
	  display_value_float(p5);
	  display_value_float(p3);
	  display_value_float(p2);
	  display_value_int(np);
	  display_value_float(p4);
	  display_value_float(p1);
	   break;
	case 4:
	  //00-------p4-----
	  PORTB &=~(1<<0);
	  PORTB &=~(1<<1);
	  PORTB &=~(1<<2);
	  
	  v6=adc_read(0);
	  
	  
	  p4=V_sup*v6;
	  p4=p4/1024;
	  //p4=p4-0.5;
	  p4=p4-0.5;
	  p4=p4/0.396;
	  
	  if(p4<=0){
		  
		  p4=0.00;

	  }
	  //---------------------
	  //01------P1-----------
	  PORTB &=~(1<<1);
	  PORTB |=(1<<0);
	  PORTB &=~(1<<2);
	  
	  v1=adc_read(0);
	  
	  p1=V_sup*v1;
	  p1=p1/1024;
	  //p1=p1-0.5;
	  p1=p1-0.5;
	  p1=p1/0.396;
	  
	  if(p1<=0){
		  
		  p1=0.00;
		  
	  }
	  //---------------------
	  //10------P2-----------
	  PORTB |=(1<<1);
	  PORTB &=~(1<<0);
	  PORTB &=~(1<<2);
	  v2=adc_read(0);
	  
	  p2=V_sup*v2;
	  
	  p2=p2/1024;
	  
	  //p2=p2-0.5;
	  p2=p2-0.5;
	  p2=p2/0.396;
	  
	  if(p2<=0){
		  
		  p2=0.00;
		  
	  }

	  //----------------------
	  //100----np-------------
	  PORTB |=(1<<2);
	  PORTB &=~(1<<1);
	  PORTB &=~(1<<0);
	  
	  v4=adc_read(0);

	  np=V_sup*v4;
	  np=np/1024;
	  np=np-(0.92*5);
	  np=np/0.03826;
	  np=np*7.50062;
	  np=(-1)*np;

	  if(np<=0){
		  
		  np=0;
		  
	  }
	  //----------------------

	  n=2;
	  p3=3.5;
	  p5=3.5;
	  display_value_float(p5);
	  display_value_float(p3);
	  display_value_int(np);
	  display_value_float(p2);
	  display_value_float(p4);
	  display_value_float(p1);
  break;
  
    case 5:
  
	  //00-------p4-----
	  PORTB &=~(1<<0);
	  PORTB &=~(1<<1);
	  PORTB &=~(1<<2);
	  
	  v6=adc_read(0);
	  
	  
	  p4=V_sup*v6;
	  p4=p4/1024;
	  //p4=p4-0.5;
	  p4=p4-0.5;
	  p4=p4/0.396;
	  
	  if(p4<=0){
		  
		  p4=0.00;

	  }
	  //---------------------
	  //01------P1-----------
	  PORTB &=~(1<<1);
	  PORTB |=(1<<0);
	  PORTB &=~(1<<2);
	  
	  v1=adc_read(0);
	  
	  p1=V_sup*v1;
	  p1=p1/1024;
	  //p1=p1-0.5;
	  p1=p1-0.5;
	  p1=p1/0.396;
	  
	  if(p1<=0){
		  
		  p1=0.00;
		  
	  }
	  //---------------------
	  //10------P2-----------
	  PORTB |=(1<<1);
	  PORTB &=~(1<<0);
	  PORTB &=~(1<<2);
	  v2=adc_read(0);
	  
	  p2=V_sup*v2;
	  
	  p2=p2/1024;
	  
	  //p2=p2-0.5;
	  p2=p2-0.5;
	  p2=p2/0.396;
	  
	  if(p2<=0){
		  
		  p2=0.00;
		  
	  }

	  //---------------------
	  
	  //11------P3------------
	  PORTB |=(1<<0);
	  PORTB |=(1<<1);
	  PORTB &=~(1<<2);
	  
	  v3=adc_read(0);
	  
	  p3=V_sup*v3;
	  p3=p3/1024;
	  //p3=p3-0.5;
	  p3=p3-0.5;
	  p3=p3/0.396;
	  
	  if(p3<=0){
		  
		  p3=0.00;
		  
	  }
	  
	  //----------------------
	  //100----np-------------
	  PORTB |=(1<<2);
	  PORTB &=~(1<<1);
	  PORTB &=~(1<<0);
	  
	  v4=adc_read(0);

	  np=V_sup*v4;
	  np=np/1024;
	  np=np-(0.92*5);
	  np=np/0.03826;
	  np=np*7.50062;
	  np=(-1)*np;

	  if(np<=0){
		  
		  np=0;
		  
	  }
	  //----------------------

	  p5=3.5;
	  n=1;
	  display_value_float(p5);
	  display_value_int(np);
	  display_value_float(p3);
	  display_value_float(p2);
	  display_value_float(p4);
	  display_value_float(p1);
	    break;
    case 6:
	 
	  //00-------p4-----
	  PORTB &=~(1<<0);
	  PORTB &=~(1<<1);
	  PORTB &=~(1<<2);
	  
	  v6=adc_read(0);
	  
	  
	  p4=V_sup*v6;
	  p4=p4/1024;
	  //p4=p4-0.5;
	  p4=p4-0.5;
	  p4=p4/0.396;
	  
	  if(p4<=0){
		  
		  p4=0.00;

	  }
	  //---------------------
	  //01------P1-----------
	  PORTB &=~(1<<1);
	  PORTB |=(1<<0);
	  PORTB &=~(1<<2);
	  
	  v1=adc_read(0);
	  
	  p1=V_sup*v1;
	  p1=p1/1024;
	  //p1=p1-0.5;
	  p1=p1-0.5;
	  p1=p1/0.396;
	  
	  if(p1<=0){
		  
		  p1=0.00;
		  
	  }
	  //---------------------
	  //10------P2-----------
	  PORTB |=(1<<1);
	  PORTB &=~(1<<0);
	  PORTB &=~(1<<2);
	  v2=adc_read(0);
	  
	  p2=V_sup*v2;
	  
	  p2=p2/1024;
	  
	  //p2=p2-0.5;
	  p2=p2-0.5;
	  p2=p2/0.396;
	  
	  if(p2<=0){
		  
		  p2=0.00;
		  
	  }

	  //---------------------
	  
	  //11------P3------------
	  PORTB |=(1<<0);
	  PORTB |=(1<<1);
	  PORTB &=~(1<<2);
	  
	  v3=adc_read(0);
	  
	  p3=V_sup*v3;
	  p3=p3/1024;
	  //p3=p3-0.5;
	  p3=p3-0.5;
	  p3=p3/0.396;
	  
	  if(p3<=0){
		  
		  p3=0.00;
		  
	  }
	  
	  //----------------------
	  //100----np-------------
	  PORTB |=(1<<2);
	  PORTB &=~(1<<1);
	  PORTB &=~(1<<0);
	  
	  v4=adc_read(0);

	  np=V_sup*v4;
	  np=np/1024;
	  np=np-(0.92*5);
	  np=np/0.03826;
	  np=np*7.50062;
	  np=(-1)*np;

	  if(np<=0){
		  
		  np=0;
		  
	  }
	  //----------------------
	  //1001-----P5-----------
	  PORTB |=(1<<0);
	  PORTB &=~(1<<1);
	  PORTB |=(1<<2);
	  
	  v5=adc_read(0);
	  
	  p5=V_sup*v5;
	  p5=p5/1024;
	  //p5=p5-0.5;
	  p5=p5-0.5;
	  p5=p5/0.396;
	  
	  if(p5<=0){
		  
		  p5=0.00;
		  
	  }
	  
	n=0;
	display_value_int(np);
	display_value_float(p5);
	display_value_float(p3);
	display_value_float(p2);
	display_value_float(p4);
	display_value_float(p1);
    break;				   	   
	
}
/*	display_value_float(p4);
	display_value_float(p5);
	display_value_float(p3);
	display_value_float(p2);
	display_value_int(np);
	display_value_float(p1);*/
	latch_data();
	 
}
//fault detect-------------------------------------------
void fault_detect(void){
//	if(channel_count==MAX_CHANNEL){
		
	if(p1>POSITIVE_HIGH_TH || p2>POSITIVE_HIGH_TH || p3>POSITIVE_HIGH_TH || p4>POSITIVE_HIGH_TH || p5>POSITIVE_HIGH_TH || np <NEGATIVE_LOW_TH)
	{
		
	  fault=1;
	  if(p1>POSITIVE_HIGH_TH)
	  {
		    PORTC &=~(1<<1);
		    PORTC |=(1<<2);
		    PORTC |=(1<<3);
	  }
	 
	  if(p2>POSITIVE_HIGH_TH)
	  {
		 led_[3]=0xdf;
	  }
	  if(p3>POSITIVE_HIGH_TH)
	  {
		  led_[2]=0xdf;	
	  }
	  if(p4>POSITIVE_HIGH_TH)
	  {
		  
		  //led_[0]=0xdf;
		   PORTD |=(1<<4);
		   PORTC &=~(1<<4);
		   PORTD &=~(1<<3);
		   
	  }
	  if(p5>POSITIVE_HIGH_TH)
	  {
		  led_[1]=0xdf;
	  }
	  if(np<NEGATIVE_LOW_TH)
	  {
		   led_[n]=0x7f;
		   // PORTC |=(1<<4);
		    
	  }
	  
	  
	}
	
	if(p1<POSITIVE_LOW_TH || p2<POSITIVE_LOW_TH || p3<POSITIVE_LOW_TH || p4<POSITIVE_LOW_TH || p5<POSITIVE_LOW_TH )
	{
		fault=1;
		if(p1<POSITIVE_LOW_TH)
		{
			  PORTC &=~(1<<3);
			
			  PORTC |=(1<<1);
			  PORTC |=(1<<2);
		}
		if(p2<POSITIVE_LOW_TH)
		{
			led_[3]=0x7f;
		}
		if(p3<POSITIVE_LOW_TH)
		{
			led_[2]=0x7f;
		}
		if(p4<POSITIVE_LOW_TH)
		{
			//led_[0]=0x7f;
			 PORTD |=(1<<4);
			 PORTC |=(1<<4);
			 PORTD |=(1<<3);
		}
		if(p5<POSITIVE_LOW_TH)
		{
			led_[1]=0x7f;
		}
		
		
	}
	if(p1>POSITIVE_LOW_TH && p1<POSITIVE_HIGH_TH)
	{
		 PORTC &=~(1<<2);
		 
		 PORTC |=(1<<1);
		 PORTC |=(1<<3);
	}
	
	if(p4>POSITIVE_LOW_TH && p4<POSITIVE_HIGH_TH)
	{
		//led_[0]=0xbf;
		 PORTD &=~(1<<4);
		 PORTC &=~(1<<4);
		 PORTD |=(1<<3);
		 
	}
	if(p2>POSITIVE_LOW_TH && p2<POSITIVE_HIGH_TH)
	{
		if(channel_count>=4){
		led_[3]=0xbf;
		}
	}
	if(p3>POSITIVE_LOW_TH && p3<POSITIVE_HIGH_TH)
	{
		if(channel_count>=5){
		led_[2]=0xbf;
		}
	}
	if(p5>POSITIVE_LOW_TH && p5<POSITIVE_HIGH_TH)
	{
		if(channel_count>=6){
		led_[1]=0xbf;
		}
		
	}
	if(np>NEGATIVE_LOW_TH)
	{
	 // PORTC &=~(1<<4);
	 led_[n]=0xbf;
	}
if((p1>POSITIVE_LOW_TH && p1<POSITIVE_HIGH_TH) && (p2>POSITIVE_LOW_TH && p2<POSITIVE_HIGH_TH) &&(p3>POSITIVE_LOW_TH && p3<POSITIVE_HIGH_TH) && (p4>POSITIVE_LOW_TH && p4<POSITIVE_HIGH_TH) && (p5>POSITIVE_LOW_TH && p5<POSITIVE_HIGH_TH) && np>NEGATIVE_LOW_TH)
{

fault=0;
mute=0;
led_[0]=0xbf;
led_[1]=0xbf;
led_[2]=0xbf;
led_[3]=0xbf;

}
//	}

  led_disp_update();
  	
}
//-----------------------------
int main(void)
{
	int digit =1;
	int count=2;
	uint8_t timer=0;
	io_initialization();
    adc_init();
	init_led_disp();
	 display_value_int(0);
	 display_value_int(0);
	 display_value_int(0);
	 display_value_int(0);
	 display_value_int(0);
	 display_value_int(0);
	 latch_data();
	_delay_ms(20000);
	
	if(!(PIND & (1<<2))){
		_delay_ms(5000);
	if(!(PIND & (1<<2))){
		 PORTC |=(1<<5);
		 _delay_ms(1000);
	     PORTC &=~(1<<5);
		mode=SET_UP_MODE;
		
		
	}	
		
	}
	
   channel_count= eeprom_read_byte((uint8_t*)46);
   channel_count=3;

    /* Replace with your application code */
    while (1) 
    {
		if(mode==NORMAL_MODE){
		display_pressure();
		 fault_detect();
		if(!(PIND & (1<<2))){
			 _delay_ms(1000);
			 if(!(PIND & (1<<2)))
			 {
			 mute=1;
			 PORTC &=~(1<<5);
			 }
		 }
		 if(mute==0 && fault==1){
			 
			 PORTC |=(1<<5);
			 
		 }
		 if(fault==0){
			 
			  PORTC &=~(1<<5);
			 
		 }
	_delay_ms(100);
		}
		
		if(mode==SET_UP_MODE){
        
         	if(!(PIND & (1<<2))){
	         	_delay_ms(100);
	         	if(!(PIND & (1<<2))){
		         	
		         count++;
				 if(count>MAX_CHANNEL){

					 count=MIN_CHANNEl;
					 
				 }
			
				 display_value_int(0);
				  display_value_int(0);
				   display_value_int(0);
				    display_value_int(0);
					 display_value_int(count);
		         display_value_int(0);
		         latch_data();
				 while((!(PIND & (1<<2)))){
					 
					 timer ++;
					 _delay_ms(500);
					 
				 }	
	         	}
	         	
         	}
		
	
				if(timer>=20){
					
					channel_count=count;
					eeprom_write_byte ((uint8_t *)46,channel_count);
					timer=0;
					count=0;
					timer=0;
					PORTC |=(1<<5);
					_delay_ms(1000);
					PORTC &=~(1<<5);
					mode=NORMAL_MODE;
				}
				timer=0;

		}
    }
}

