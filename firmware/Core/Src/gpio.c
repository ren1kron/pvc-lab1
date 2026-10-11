#include "gpio.h"

void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef gpio = {0};
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /* Establish OFF in the output latch before enabling output drivers. */
  HAL_GPIO_WritePin(GPIOD, LED_GREEN_Pin | LED_RED_Pin | LED_YELLOW_Pin,
                    GPIO_PIN_RESET);
  gpio.Pin = LED_GREEN_Pin | LED_RED_Pin | LED_YELLOW_Pin;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &gpio);

  gpio.Pin = BUTTON_Pin;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_NOPULL; /* R11 on processor module provides 3 kohm pull-up. */
  HAL_GPIO_Init(BUTTON_GPIO_Port, &gpio);
  /* PA13/PA14/PA15/PB3/PB4 remain in their reset JTAG functions. */
}
