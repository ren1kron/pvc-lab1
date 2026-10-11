/* Standalone 2026 Lab 1 part 2: GPIO register model drives the real app. */
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "button.h"
#include "morse.h"

GPIO_TypeDef fake_c, fake_d;
static uint32_t now;
static uint16_t pads(void) { return GpioModel_Pads(&fake_d); }
static void trace(const char *operation, GPIO_TypeDef *port, uint32_t offset, uint32_t value)
{
  uint16_t d = pads();
  printf("%" PRIu32 ",%s,%c,0x%02" PRIx32 ",0x%08" PRIx32 ",0x%04x,%u,%u,%u,%u,%u\n",
    now, operation, port == &fake_c ? 'C' : 'D', offset, value,
    GpioModel_Pads(port), !!(d & 0x2000), (d & 0xc000) == 0x8000,
    (d & 0xc000) == 0x4000, (unsigned)Morse_GetState(), Morse_GetLength());
}
static void write_reg(GPIO_TypeDef *port, uint32_t offset, uint32_t value)
{ assert(GpioModel_Write(port, offset, value)); trace("write", port, offset, value); }
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin)
{
  uint32_t value = GpioModel_Read(port, GM_IDR);
  trace("read", port, GM_IDR, value);
  return (value & pin) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}
void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pins, GPIO_PinState state)
{ write_reg(port, GM_BSRR, state == GPIO_PIN_SET ? pins : (uint32_t)pins << 16); }

int main(void)
{
  const struct { uint32_t time; bool pressed; } inputs[] = {
    {100,true}, {200,false}, {400,true}, {900,false}, {1100,true}, {1200,false}
  };
  const uint32_t expected_edges[] = {2730,2930,3130,3730,3930,4130};
  unsigned next = 0, edge = 0;
  bool was_green = false;
  puts("ms,operation,port,offset,value,pads,green,red,yellow,state,length");
  GpioModel_Reset(&fake_c, 'C'); GpioModel_Reset(&fake_d, 'D');
  GpioModel_Pulls(&fake_c, 0x8000, 0);
  write_reg(&fake_d, GM_BSRR, 0xe0000000U);
  write_reg(&fake_d, GM_MODER, (1U<<26) | (1U<<28) | (1U<<30));
  write_reg(&fake_d, GM_OTYPER, 0); write_reg(&fake_d, GM_OSPEEDR, 0);
  write_reg(&fake_d, GM_PUPDR, 0); write_reg(&fake_c, GM_MODER, 0);
  write_reg(&fake_c, GM_PUPDR, 0);
  Button_Init(0); Morse_Init();
  for (now = 0; now <= 4500; ++now) {
    if (next < sizeof inputs/sizeof inputs[0] && inputs[next].time == now) {
      GpioModel_Drive(&fake_c, inputs[next].pressed ? 0x8000 : 0, 0);
      trace("external", &fake_c, GM_IDR, GpioModel_Read(&fake_c, GM_IDR));
      ++next;
    }
    Button_Update(now); Morse_Update(now);
    bool green = (pads() & 0x2000) != 0;
    if (green != was_green) {
      assert(edge < sizeof expected_edges/sizeof expected_edges[0]);
      assert(now == expected_edges[edge++]);
      trace("green_edge", &fake_d, GM_ODR, GpioModel_Read(&fake_d, GM_ODR));
      was_green = green;
    }
    if (now == 230 || now == 1230) assert((pads() & 0xc000) == 0x4000);
    if (now == 930) assert((pads() & 0xc000) == 0x8000);
    assert(fake_c.contention == 0 && fake_d.contention == 0);
  }
  assert(edge == 6 && Morse_GetState() == MORSE_IDLE);
  fprintf(stderr, "GPIO simulation: dot-dash-dot, all 6 timed green edges verified\n");
  return 0;
}
