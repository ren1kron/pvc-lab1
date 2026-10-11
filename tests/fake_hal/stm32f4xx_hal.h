#ifndef TEST_HAL_H
#define TEST_HAL_H
#include <stdint.h>
#ifdef GPIO_REGISTER_MODEL
#include "gpio_model.h"
typedef GpioModel GPIO_TypeDef;
#else
typedef struct { uint16_t input; uint16_t output; } GPIO_TypeDef;
#endif
typedef enum { GPIO_PIN_RESET = 0, GPIO_PIN_SET } GPIO_PinState;
extern GPIO_TypeDef fake_c, fake_d;
#define GPIOC (&fake_c)
#define GPIOD (&fake_d)
#define GPIO_PIN_13 ((uint16_t)0x2000)
#define GPIO_PIN_14 ((uint16_t)0x4000)
#define GPIO_PIN_15 ((uint16_t)0x8000)
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pins);
void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pins, GPIO_PinState state);
#endif
