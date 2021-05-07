// main.cpp
// Runs on TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10 in C++

// Last Modified: 1/16/2021 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* 
 Copyright 2021 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 47k  resistor DAC bit 0 on PB0 (least significant bit)
// 24k  resistor DAC bit 1 on PB1
// 12k  resistor DAC bit 2 on PB2
// 6k   resistor DAC bit 3 on PB3 
// 3k   resistor DAC bit 4 on PB4 
// 1.5k resistor DAC bit 5 on PB5 (most significant bit)

// VCC   3.3V power to OLED
// GND   ground
// SCL   PD0 I2C clock (add 1.5k resistor from SCL to 3.3V)
// SDA   PD1 I2C data

//************WARNING***********
// The LaunchPad has PB7 connected to PD1, PB6 connected to PD0
// Option 1) do not use PB7 and PB6
// Option 2) remove 0-ohm resistors R9 R10 on LaunchPad
//******************************

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "TExaS.h"
#include "SSD1306.h"
#include "Random.h"
#include "Switch.h"
#include "SlidePot.h"
#include "Images.h"
#include "Timer0.h"
#include "Timer1.h"
#include "Timer2.h"
#include "Sound.h"

//********************************************************************************
// debuging profile, pick up to 7 unused bits and send to Logic Analyzer
#define PA54                  (*((volatile uint32_t *)0x400040C0)) // bits 5-4
#define PF321                 (*((volatile uint32_t *)0x40025038)) // bits 3-1
// use for debugging profile
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PA5       (*((volatile uint32_t *)0x40004080)) 
#define PA4       (*((volatile uint32_t *)0x40004040)) 
// TExaSdisplay logic analyzer shows 7 bits 0,PA5,PA4,PF3,PF2,PF1,0 
void LogicAnalyzerTask(void){
  UART0_DR_R = 0x80|PF321|PA54; // sends at 10kHz
}
void ScopeTask(void){  // called 10k/sec
  UART0_DR_R = (ADC1_SSFIFO3_R>>4); // send ADC to TExaSdisplay
}
void Profile_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x21;      // activate port A,F
  while((SYSCTL_PRGPIO_R&0x20) != 0x20){};
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 
  GPIO_PORTF_DEN_R |=  0x0E;   // enable digital I/O on PF3,2,1
  GPIO_PORTA_DIR_R |=  0x30;   // output on PA4 PA5
  GPIO_PORTA_DEN_R |=  0x30;   // enable on PA4 PA5  
}
//********************************************************************************
 

extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C" void SysTick_Handler(void);
void Delay100ms(uint32_t count); // time delay in 0.1 seconds

//Valvano Lab Lecture
typedef enum {dead, alive} status_t;
struct sprite{
	int32_t x;
	int32_t y;
	int32_t vx;
	int32_t vy;
	const uint8_t *image;
	status_t life;
};
typedef struct sprite sprite_t;
sprite_t Enemys[2];
sprite_t Player;
sprite_t Missiles[48];

int NeedToDraw;
void Init(void){
	Player.x = 50;
	Player.y = 62;
	Player.image = PlayerShip0;
	Player.life = alive;
	int i;
	for(i = 0; i < 2; i++){
		Enemys[i].x = 20*i;
		Enemys[i].y = 6;
		if(i < 1){
			Enemys[i].vx = 1;
		}
		if(i > 1){
			Enemys[i].vx = -1;
		}
		Enemys[i].vy = 1;
		Enemys[i].image = Alien10pointA;
		Enemys[i].life = alive;
	}
	for(i = 0; i < 48; i++){
		Missiles[i].life = dead;
	}
}
int32_t abs(int32_t n){
	if(n < 0){
		return -n;
	}else{
		return n;
	}
}
int32_t max(int32_t n, int32_t m){
	if(n > m){
		return n;
	}else{
		return m;
	}
}
uint8_t points;
uint8_t win;
void Collisions(void){
	int i;
	uint32_t x1,y1,x2,y2;
	
	for(i = 0; i < 48; i++){
		if(Missiles[i].life == alive){
			x1 = Missiles[i].x;
			y1 = Missiles[i].y;
			for(int j = 0; j < 2; j++){
				x2 = Enemys[j].x + 8;
				y2 = Enemys[j].y - 4;
				if(Enemys[j].life == alive){
					if((abs(x1-x2) < 7)&&(abs(y1-y2)<2)){
						Enemys[j].life = dead;
						Missiles[i].life = dead;
						Sound_InvaderKilled();
						points += 10;
						if(points == 20){
							win = 1;
						}
						return;
					}
				}
			}	
		}
	}
}
uint8_t gameover = 0;
void Move(void){
	if(Player.life == alive){
		NeedToDraw = 1;
		uint32_t adcData = ADC_In();
		Player.x = (((127-16)*adcData)/4096);
	}
	int i;
	for(i = 0; i < 2; i++){
		if(Enemys[i].life == alive){
			NeedToDraw = 1;
			if(Enemys[i].x < 0){
				Enemys[i].x = 2;
				Enemys[i].vx = -Enemys[i].vx;
			}
			if(Enemys[i].x > 126){
				Enemys[i].x = 124;
				Enemys[i].vx = -Enemys[i].vx;
			}
			if((Enemys[i].y < 62) && (Enemys[i].y > 0)){
				Enemys[i].x += Enemys[i].vx;
				Enemys[i].y += Enemys[i].vy;
			}else{
				Enemys[i].life = dead;
				gameover = 1;
				//also gameover if they reach bottom
			}
		}
	}
	for(int j = 0; j < 48; j++){
		if(Missiles[j].life == alive){
			NeedToDraw = 1;
			if((Missiles[j].y < 64)){
				Missiles[j].x += Missiles[j].vx;
				Missiles[j].y += Missiles[j].vy;
			}else{
				Missiles[j].x = Player.x+7;
				Missiles[j].y = Player.y - 4;
				Missiles[j].vx = 0;
				Missiles[j].vy = 0;
				Missiles[j].life = dead;
				
			}
		}
	}
}
void Fire(int32_t vx1, int32_t vy1, const uint8_t *im){
	int i;
	i = 0;
	while(Missiles[i].life == alive){
		i++;
		if(i == 48){
			return;
		}
	}
	Missiles[i].x = Player.x+7;
	Missiles[i].y = Player.y - 4;
	Missiles[i].vx = vx1;
	Missiles[i].vy = vy1;
	Missiles[i].image = im;
	Missiles[i].life = alive;
	
}
void SysTick_Handler(void){
	static uint32_t last = 0;
	uint32_t in = Switch_In();
	if(((in&0x04)==0x04)&&((last&0x04) == 0)){
		Fire(0, -1, Laser0);
		Sound_Shoot();
	}
	Move();
	Collisions();
	last = in;
}

void Draw(void){
	int i;
	SSD1306_ClearBuffer();
	if(Player.life == alive){
		SSD1306_DrawBMP(Player.x, Player.y,
				Player.image, 0, SSD1306_INVERSE);
	}
	for(i = 0; i < 2; i++){
		if(Enemys[i].life == alive){
			SSD1306_DrawBMP(Enemys[i].x, Enemys[i].y,
				Enemys[i].image, 0, SSD1306_INVERSE);
		}
	}
	for(i = 0; i < 48; i++){
		if(Missiles[i].life == alive){
			SSD1306_DrawBMP(Missiles[i].x, Missiles[i].y,
				Missiles[i].image, 0, SSD1306_INVERSE);
		}
	}
	SSD1306_OutBuffer();
	NeedToDraw = 0;
}

int Language(void){
	SSD1306_SetCursor(1,1);
	SSD1306_OutString((char *)"Use buttons to ");
	SSD1306_SetCursor(1,2);
	SSD1306_OutString((char *)"choose language");
	SSD1306_SetCursor(1,4);
	SSD1306_OutString((char *)"Right: English");
	SSD1306_SetCursor(1,5);
	SSD1306_OutString((char *)"Izquierda: Español");
	uint32_t in = 0;
	while((in&0x06) == 0){
		in = Switch_In();
	}
	if((in&0x06) == 0x02){
		return 2; //spanish
	}else{
		return 1; //english
	}
}

void SysTick_Init20Hz(void){
	// Disable SysTick during setup
	NVIC_ST_CTRL_R = 0;
	// Number of counts
	NVIC_ST_RELOAD_R = 4000000 - 1;
	// any write to CURRENT clears it
	NVIC_ST_CURRENT_R = 0;
	// enable SysTick with core clock
	NVIC_ST_CTRL_R = 7;
}
/*
void GPIOPortE_Handler(void){
  GPIO_PORTE_ICR_R = 0x02;      // acknowledge flag4
  SSD1306_OutClear(); 
	SSD1306_SetCursor(1,1);
	SSD1306_OutString((char *)"Paused");
	SSD1306_SetCursor(1,2);
	SSD1306_OutString((char *)"Press left to resume");
	while((GPIO_PORTE_DATA_R&0x02) == 0){};
}*/

 int main(void){uint32_t time=0;
  DisableInterrupts();
  // pick one of the following three lines, all three set to 80 MHz
  //PLL_Init();                   // 1) call to have no TExaS debugging
  TExaS_Init(&LogicAnalyzerTask); // 2) call to activate logic analyzer
  //TExaS_Init(&ScopeTask);       // or 3) call to activate analog scope PD2
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear();   
  Random_Init(1);
  Profile_Init(); // PB5,PB4,PF3,PF2,PF1 
  /*SSD1306_ClearBuffer();
  SSD1306_DrawBMP(2, 62, SpaceInvadersMarquee, 0, SSD1306_WHITE);
  SSD1306_OutBuffer();
  EnableInterrupts();
  Delay100ms(20);*/
	
	ADC_Init(4);
	Sound_Init();
	Switch_Init();
	Init();
	EnableInterrupts();
	SSD1306_ClearBuffer();
  SSD1306_DrawBMP(2, 62, SpaceInvadersMarquee, 0, SSD1306_WHITE);
  SSD1306_OutBuffer();
	EnableInterrupts();
	Delay100ms(20);
	SSD1306_ClearBuffer();
	SSD1306_OutClear();
	int language = Language();
	/*if(language == 2){
		SSD1306_SetCursor(1, 1);
		SSD1306_OutString((char *)"Botones:");
		SSD1306_SetCursor(1, 2);
		SSD1306_OutString((char *)"Usar la derecha para disparar");
		SSD1306_SetCursor(1, 3);
		SSD1306_OutString((char *)"Usar la izquierda para pausar");
		SSD1306_SetCursor(1, 4);
		SSD1306_OutString((char *)"Utiliza el deslizador para moverte");
	}else{
		SSD1306_SetCursor(1, 1);
		SSD1306_OutString((char *)"Buttons:");
		SSD1306_SetCursor(1, 2);
		SSD1306_OutString((char *)"Use right to shoot");
		SSD1306_SetCursor(1, 3);
		SSD1306_OutString((char *)"Use left to pause");
		SSD1306_SetCursor(1, 4);
		SSD1306_OutString((char *)"Use slider to move");
	}*/
	
	SSD1306_ClearBuffer();
	SSD1306_OutClear();
	SysTick_Init20Hz();
	//PauseButton();
	while((gameover != 1)&&(win != 1)){
		SSD1306_SetCursor(19,0);
		SSD1306_OutUDec2(points);
		
		if(NeedToDraw){
			Draw();
		}
	}
	DisableInterrupts();
	SSD1306_OutClear(); 
	SSD1306_SetCursor(1, 1);
	
	if(win == 1){
		if(language == 2){
			SSD1306_OutString((char *)"GANASTE");
		}else{
			SSD1306_OutString((char *)"YOU WON");
		}
	}
	
	if(gameover == 1){
		if(language == 2){
			SSD1306_OutString((char *)"JUEGO TERMINADO");
			SSD1306_SetCursor(1, 2);
			SSD1306_OutString((char *)"Buen intento,");
			SSD1306_SetCursor(1, 3);
			SSD1306_OutString((char *)"Terrícola!");
		}else{
			SSD1306_OutString((char *)"GAME OVER");
			SSD1306_SetCursor(1, 2);
			SSD1306_OutString((char *)"Nice try,");
			SSD1306_SetCursor(1, 3);
			SSD1306_OutString((char *)"Earthling!");
		}
	}
	while(1);
  /*SSD1306_ClearBuffer();
  SSD1306_DrawBMP(47, 63, PlayerShip0, 0, SSD1306_WHITE); // player ship bottom
  SSD1306_DrawBMP(53, 55, Bunker0, 0, SSD1306_WHITE);

  SSD1306_DrawBMP(0, 9, Alien10pointA, 0, SSD1306_WHITE);
  SSD1306_DrawBMP(20,9, Alien10pointB, 0, SSD1306_WHITE);
  SSD1306_DrawBMP(40, 9, Alien20pointA, 0, SSD1306_WHITE);
  SSD1306_DrawBMP(60, 9, Alien20pointB, 0, SSD1306_WHITE);
  SSD1306_DrawBMP(80, 9, Alien30pointA, 0, SSD1306_WHITE);
  SSD1306_DrawBMP(50, 19, AlienBossA, 0, SSD1306_WHITE);
  SSD1306_OutBuffer();
  Delay100ms(30);

  SSD1306_OutClear();  
  SSD1306_SetCursor(1, 1);
  SSD1306_OutString((char *)"GAME OVER");
  SSD1306_SetCursor(1, 2);
  SSD1306_OutString((char *)"Nice try,");
  SSD1306_SetCursor(1, 3);
  SSD1306_OutString((char *)"Earthling!");
  SSD1306_SetCursor(2, 4);
  while(1){
    Delay100ms(10);
    SSD1306_SetCursor(19,0);
    SSD1306_OutUDec2(time);
    time++;
    PF1 ^= 0x02;
		//while(Player.life ==  alive){
		//if(NeedToDraw){
		//	Draw();
		//}
		//}
		//DisableInterrupts();
		//SSD1306_OutClear();
		//SSD1306_OutString("You win!");
		//while(1);
  }
	
}*/
}

// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
      time--;
    }
    count--;
  }
}


