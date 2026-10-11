#include <assert.h>
#include <stdio.h>
#include "gpio_model.h"

int main(void)
{
  GpioModel g;
  GpioModel_Reset(&g, 'A');
  assert(GpioModel_Read(&g, GM_MODER) == 0xa8000000);
  assert(GpioModel_Read(&g, GM_PUPDR) == 0x64000000);
  GpioModel_Reset(&g, 'B'); assert(GpioModel_Read(&g, GM_OSPEEDR) == 0xc0);
  GpioModel_Reset(&g, 'D'); assert(GpioModel_Read(&g, GM_MODER) == 0);
  puts("PASS reset and debug pin defaults");

  GpioModel_Write(&g, GM_PUPDR, 1); assert(GpioModel_Read(&g, GM_IDR) & 1);
  GpioModel_Drive(&g, 1, 0); assert(!(GpioModel_Read(&g, GM_IDR) & 1));
  GpioModel_Drive(&g, 0, 0); GpioModel_Write(&g, GM_PUPDR, 2);
  assert(!(GpioModel_Read(&g, GM_IDR) & 1));
  assert(!GpioModel_Write(&g, GM_IDR, 0xffff));
  assert(!GpioModel_Write(&g, 3, 0));
  puts("PASS inputs pulls and read-only IDR");

  GpioModel_Write(&g, GM_BSRR, 1); /* Preload before changing mode. */
  GpioModel_Write(&g, GM_MODER, 1); assert(GpioModel_Read(&g, GM_IDR) & 1);
  GpioModel_Write(&g, GM_BSRR, 0x10000); assert(!(GpioModel_Pads(&g) & 1));
  GpioModel_Write(&g, GM_BSRR, 0x10001); assert(GpioModel_Pads(&g) & 1);
  assert(GpioModel_Read(&g, GM_BSRR) == 0);
  GpioModel_Write(&g, GM_ODR, 0xffffffff); assert(GpioModel_Read(&g, GM_ODR) == 0xffff);
  puts("PASS output latch atomic BSRR and set priority");

  GpioModel_Write(&g, GM_OTYPER, 1); GpioModel_Write(&g, GM_PUPDR, 0);
  GpioModel_Pads(&g); assert(g.floating & 1);
  GpioModel_Pulls(&g, 1, 0); assert(GpioModel_Pads(&g) & 1);
  GpioModel_Write(&g, GM_BSRR, 0x10000); assert(!(GpioModel_Pads(&g) & 1));
  GpioModel_Write(&g, GM_MODER, 3); assert(!(GpioModel_Read(&g, GM_IDR) & 1));
  puts("PASS open drain high impedance and analog input disable");

  GpioModel_Write(&g, GM_MODER, 2); GpioModel_Write(&g, GM_OTYPER, 0);
  GpioModel_Write(&g, GM_AFRL, 7); GpioModel_Alternate(&g, 7, 1, 1);
  assert(GpioModel_Pads(&g) & 1);
  GpioModel_Alternate(&g, 7, 1, 0); assert(!(GpioModel_Pads(&g) & 1));
  GpioModel_Drive(&g, 1, 1); GpioModel_Pads(&g); assert(g.contention & 1);
  puts("PASS alternate mux and contention detection");

  GpioModel_Reset(&g, 'D');
  GpioModel_Write(&g, GM_MODER, 1);
  GpioModel_Write(&g, GM_LCKR, 0x10001); GpioModel_Write(&g, GM_LCKR, 1);
  GpioModel_Write(&g, GM_LCKR, 0x10001);
  (void)GpioModel_Read(&g, GM_LCKR);
  assert(GpioModel_Read(&g, GM_LCKR) == 0x10001);
  GpioModel_Write(&g, GM_MODER, 0); assert(GpioModel_Read(&g, GM_MODER) == 1);
  GpioModel_Write(&g, GM_AFRL, 0xf); assert(GpioModel_Read(&g, GM_AFRL) == 0);
  GpioModel_Write(&g, GM_OTYPER, 1); assert(GpioModel_Read(&g, GM_OTYPER) == 0);
  GpioModel_Write(&g, GM_BSRR, 1); assert(GpioModel_Pads(&g) & 1);
  puts("PASS lock freezes configuration but permits output data");

  GpioModel_Reset(&g, 'D');
  GpioModel_Write(&g, GM_LCKR, 0x10001); GpioModel_Write(&g, GM_LCKR, 2);
  GpioModel_Write(&g, GM_LCKR, 0x10001);
  (void)GpioModel_Read(&g, GM_LCKR); (void)GpioModel_Read(&g, GM_LCKR);
  assert(!g.lock_active);
  GpioModel_Write(&g, GM_MODER, 1); assert(GpioModel_Read(&g, GM_MODER) == 1);
  puts("PASS invalid lock sequence and peripheral reset");
  puts("7 GPIO model test groups passed");
  return 0;
}
