// SlidePot.cpp
// Runs on TM4C123
// Provide functions that initialize ADC0
// Last Modified: 1/17/2021 
// Student names: Alex Koo, Anthony Do
// Last modification date: 04/15/2021

#include <stdint.h>
#include "SlidePot.h"
#include "../inc/tm4c123gh6pm.h"
// feel free to redesign this, as long as there is a class

// ADC initialization function 
// Input: sac sets hardware averaging
// Output: none
// measures from PD2, analog channel 5
volatile uint32_t delay;
void ADC_Init(uint32_t sac){ 
//*** students write this ******
	
	SYSCTL_RCGCGPIO_R |= 0x08; // Activate clock for Port D
	while ((SYSCTL_RCGCGPIO_R&0x08) == 0) {}; // Wait for it to stabilize
		
	//Port initialization - change this if you want to change the port
	// make PD2 into input
	GPIO_PORTD_DIR_R &= ~0x04; 
	// allow alternate functionality on PD2
	GPIO_PORTD_AFSEL_R |= 0x04; 
	// Disable digital I/O on PD2
	GPIO_PORTD_DEN_R &= ~0x04;  
	// Enable analog functionality on PD2
	GPIO_PORTD_AMSEL_R |= 0x04; 
	
	// Activate ADC0 -> if you want to change the pin you will change this code
	SYSCTL_RCGCADC_R |= 0x01; 
	// Wait for it to stabilize
	delay = SYSCTL_RCGCADC_R; 
	delay = SYSCTL_RCGCADC_R; 
	delay = SYSCTL_RCGCADC_R; 
	delay = SYSCTL_RCGCADC_R; 
	delay = SYSCTL_RCGCADC_R; 	
	delay = SYSCTL_RCGCADC_R; 	
	delay = SYSCTL_RCGCADC_R; 	
	delay = SYSCTL_RCGCADC_R; 	
	//set SAC to input value
	ADC0_SAC_R = sac;          
	//set 125kHz ADC conversion speed by writing 0x01
	ADC0_PC_R &= 0;
	ADC0_PC_R |= 0x01;       
	//set the priority to sequence 3
	ADC0_SSPRI_R = 0x0123;      
	//disables sample sequencer 3 during set up
	ADC0_ACTSS_R &= ~0x0008;    
	//sets the sequence 3 as software trigger
	ADC0_EMUX_R &= ~0xF000;    
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R & 0xFFFFFFF0) + 5; 
	ADC0_SSCTL3_R = 0x0006;    
	//disable SS3 interrupts
	
	//sample sequencer 3 is enabled 
	ADC0_ACTSS_R |= 0x0008; 
	
}

//------------ADCIn------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
	//initilizes the return var
	uint32_t data;         
	//starts the ADC
	ADC0_PSSI_R = 0x0008;  
	//wait for the status (busy/ready)
	while((ADC0_RIS_R&0x08)==0){};   
	//reads the input data 
	data = ADC0_SSFIFO3_R&0xFFF;    
	//this clears the flag
	ADC0_ISC_R = 0x0008;    
	//returns the read data
	return data;            

}

// constructor, invoked on creation of class
// m and b are linear calibration coefficents 
SlidePot::SlidePot(uint32_t m, uint32_t b){	
	// Set slope equal to m
	this->slope = m;
	// Set offset equal to b
	this->offset = b;

}

void SlidePot::Save(uint32_t n){	
	// Save ADC sample (n) into private variable
	this->data = n;
	// Calculates distance from ADC
	this->distance = Convert(n);
	// Set semaphore flag to 1
	this->flag = 1;
	
}
//**********place your calibration data here*************
// distance PD2       ADC  fixed point
// 0.00cm   0.000V     0        0
// 0.50cm   0.825V  1024      500
// 1.00cm   1.650V  2048     1000
// 1.50cm   2.475V  3072     1500  
// 2.00cm   3.300V  4095     2000 
uint32_t SlidePot::Convert(uint32_t n){
  // use calibration data to convert ADC sample to distance
	return (1767 * n)/4096+18;
}

void SlidePot::Sync(void){
	// Waits in loop until flag is non-zero
	while (this->flag == 0) {
	};
	// Sets flag to zero
	this->flag = 0;
	
}

uint32_t SlidePot::ADCsample(void){ // return ADC sample value (0 to 4095)
	// Returns last ADC sample (maybe)
	return this->data;

}

uint32_t SlidePot::Distance(void){  // return distance value (0 to 200), 0.01cm
	// Returns last calculated distance
	return this->distance;
	
}


