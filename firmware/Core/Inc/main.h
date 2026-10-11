#ifndef MAIN_H
#define MAIN_H
#include "stm32f4xx_hal.h"

/* SDK-1.1M schematic: GLC, YARC and YCRA nets respectively.
 * The two-colour LED is antiparallel: its other pin must be low. */
#define LED_GREEN_Pin GPIO_PIN_13
#define LED_GREEN_GPIO_Port GPIOD
#define LED_YELLOW_Pin GPIO_PIN_14
#define LED_YELLOW_GPIO_Port GPIOD
#define LED_RED_Pin GPIO_PIN_15
#define LED_RED_GPIO_Port GPIOD
#define BUTTON_Pin GPIO_PIN_15
#define BUTTON_GPIO_Port GPIOC
void Error_Handler(void);
#endif
