#ifndef GPIO_MODEL_H
#define GPIO_MODEL_H
#include <stdbool.h>
#include <stdint.h>
/* RM0090 GPIO register byte offsets. API accesses are aligned 32-bit words. */
enum { GM_MODER=0x00, GM_OTYPER=0x04, GM_OSPEEDR=0x08, GM_PUPDR=0x0c,
       GM_IDR=0x10, GM_ODR=0x14, GM_BSRR=0x18, GM_LCKR=0x1c,
       GM_AFRL=0x20, GM_AFRH=0x24 };
typedef struct {
  uint32_t reg[10];
  uint16_t external_mask, external_value, pull_up, pull_down;
  uint16_t alternate_mask[16], alternate_value[16];
  uint16_t locked, lock_candidate, floating, contention;
  unsigned lock_step;
  bool lock_active;
} GpioModel;
void GpioModel_Reset(GpioModel *gpio, char port);
bool GpioModel_Write(GpioModel *gpio, uint32_t offset, uint32_t value);
uint32_t GpioModel_Read(GpioModel *gpio, uint32_t offset);
/* Strong external drive: unset mask bits are high impedance. */
void GpioModel_Drive(GpioModel *gpio, uint16_t mask, uint16_t value);
void GpioModel_Pulls(GpioModel *gpio, uint16_t up, uint16_t down);
void GpioModel_Alternate(GpioModel *gpio, unsigned af, uint16_t mask, uint16_t value);
uint16_t GpioModel_Pads(GpioModel *gpio);
#endif
