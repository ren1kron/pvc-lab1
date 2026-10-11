#include "led.h"
#include "main.h"

void LED_On(LedId led)
{
  switch (led) {
    case LED_GREEN:
      HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
      break;
    case LED_RED:
      HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
      break;
    case LED_YELLOW:
      HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_SET);
      break;
    default: break;
  }
}

void LED_Off(LedId led)
{
  switch (led) {
    case LED_GREEN: HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET); break;
    case LED_RED: HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET); break;
    case LED_YELLOW: HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_RESET); break;
    default: break;
  }
}

void LED_AllOff(void)
{
  LED_Off(LED_GREEN);
  LED_Off(LED_RED);
  LED_Off(LED_YELLOW);
}
