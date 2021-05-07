// Switch.cpp
// This software to input from switches or buttons
// Runs on TM4C123
// Program written by: Anthony Do and Alex Koo
// Date Created: 3/6/17 
// Last Modified: 1/14/21
// Lab number: 10
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data

#define PE3210 (*((volatile uint32_t *)0x4002403C))
	
void Switch_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x10;
	while((SYSCTL_RCGCGPIO_R&0x10) != 0x10){};
	GPIO_PORTE_DIR_R &= ~0x0F;
	GPIO_PORTE_DEN_R |= 0x0F;
}

uint32_t Switch_In(void){
	return PE3210;
}

void PauseButton(void){
	GPIO_PORTE_IS_R &= ~0x02;
	GPIO_PORTE_IBE_R &= ~0x02;
	GPIO_PORTE_IEV_R |= 0x02;
	GPIO_PORTE_ICR_R = 0x02; 
	GPIO_PORTE_IM_R |= 0x02;
	NVIC_PRI0_R = (NVIC_PRI0_R&0xFF0FFFFF)|0x00200000;
	NVIC_EN0_R = 0x40000000;
}