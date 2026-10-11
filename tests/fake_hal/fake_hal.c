#include "fake_hal.h"
GPIO_TypeDef fake_c, fake_d;
void Fake_Reset(void) { fake_c = (GPIO_TypeDef){0}; fake_d = (GPIO_TypeDef){0}; Fake_Button(false); }
void Fake_Button(bool pressed) { fake_c.input = pressed ? 0 : GPIO_PIN_15; }
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pins)
{ return (port->input & pins) != 0 ? GPIO_PIN_SET : GPIO_PIN_RESET; }
void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pins, GPIO_PinState state)
{
  if (state == GPIO_PIN_SET) port->output |= pins;
  else port->output &= (uint16_t)~pins;
}
/* Electrical observations deliberately use schematic bit numbers, not driver macros. */
bool Fake_Green(void) { return (fake_d.output & (1U << 13)) != 0; }
bool Fake_Red(void) { return (fake_d.output & (3U << 14)) == (1U << 15); }
bool Fake_Yellow(void) { return (fake_d.output & (3U << 14)) == (1U << 14); }
