#ifndef FAKE_HAL_H
#define FAKE_HAL_H
#include <stdbool.h>
#include "stm32f4xx_hal.h"
void Fake_Reset(void);
void Fake_Button(bool pressed);
bool Fake_Green(void);
bool Fake_Red(void);
bool Fake_Yellow(void);
#endif
