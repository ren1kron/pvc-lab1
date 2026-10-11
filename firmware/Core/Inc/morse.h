#ifndef MORSE_H
#define MORSE_H
#include <stdint.h>
typedef enum { MORSE_IDLE, MORSE_INPUT, MORSE_PLAYBACK } MorseState;
void Morse_Init(void);
void Morse_Update(uint32_t now_ms);
/* Read-only diagnostics, also useful in the debugger. */
MorseState Morse_GetState(void);
uint16_t Morse_GetLength(void);
#endif
