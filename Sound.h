// Sound.h
// Runs on TM4C123 
// Prototypes for basic functions to play sounds from the
// original Space Invaders.
// Jonathan Valvano
// 1/16/2021
#ifndef __SOUND_H__ // do not include more than once
#define __SOUND_H__
#include <stdint.h>

void Sound_Shoot(void);
void Sound_InvaderKilled(void);
//******* Sound_Init ************
// initialize a 11kHz timer, flags, pointers, counters,and the DAC
void Sound_Init(void);

//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the timer interrupt.
// It starts the sound, and the timer ISR does the output
// feel free to change the parameters
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Sound_Start(const uint8_t *pt, uint32_t count);


#endif


