#include "gpio_model.h"

void GpioModel_Reset(GpioModel *g, char port)
{
  *g = (GpioModel){0};
  if (port == 'A') {
    g->reg[GM_MODER/4] = 0xa8000000U;
    g->reg[GM_OSPEEDR/4] = 0x0c000000U;
    g->reg[GM_PUPDR/4] = 0x64000000U;
  } else if (port == 'B') {
    g->reg[GM_MODER/4] = 0x280U;
    g->reg[GM_OSPEEDR/4] = 0xc0U;
    g->reg[GM_PUPDR/4] = 0x100U;
  }
}

void GpioModel_Drive(GpioModel *g, uint16_t mask, uint16_t value)
{ g->external_mask = mask; g->external_value = value; }
void GpioModel_Pulls(GpioModel *g, uint16_t up, uint16_t down)
{ g->pull_up = up; g->pull_down = down; }
void GpioModel_Alternate(GpioModel *g, unsigned af, uint16_t mask, uint16_t value)
{ if (af < 16) { g->alternate_mask[af] = mask; g->alternate_value[af] = value; } }

uint16_t GpioModel_Pads(GpioModel *g)
{
  uint16_t result = 0;
  g->floating = g->contention = 0;
  for (unsigned pin = 0; pin < 16; ++pin) {
    uint16_t bit = (uint16_t)(1U << pin);
    unsigned mode = (g->reg[GM_MODER/4] >> (2*pin)) & 3U;
    bool drive = mode == 1;
    bool value = (g->reg[GM_ODR/4] & bit) != 0;
    if (mode == 2) {
      unsigned af = (g->reg[GM_AFRL/4 + pin/8] >> (4*(pin%8))) & 15U;
      drive = (g->alternate_mask[af] & bit) != 0;
      value = (g->alternate_value[af] & bit) != 0;
    }
    if ((g->reg[GM_OTYPER/4] & bit) && value) drive = false;
    bool external = (g->external_mask & bit) != 0;
    bool external_value = (g->external_value & bit) != 0;
    if (drive && external && value != external_value) {
      g->contention |= bit; /* Digital model reports the conflict, returns zero. */
      value = false;
    } else if (external) value = external_value;
    else if (!drive) {
      unsigned pull = mode == 3 ? 0 : (g->reg[GM_PUPDR/4] >> (2*pin)) & 3U;
      /* External resistors take precedence over the weaker internal pulls. */
      if (g->pull_down & bit) value = false;
      else if (g->pull_up & bit) value = true;
      else if (pull == 1) value = true;
      else if (pull == 2) value = false;
      else { g->floating |= bit; value = false; }
    }
    if (value) result |= bit;
  }
  return result;
}

static uint32_t configuration_lock_mask(const GpioModel *g, uint32_t offset)
{
  uint32_t mask = 0;
  for (unsigned pin = 0; pin < 16; ++pin) {
    if (!(g->locked & (1U << pin))) continue;
    if (offset == GM_OTYPER) mask |= 1U << pin;
    if (offset == GM_MODER || offset == GM_OSPEEDR || offset == GM_PUPDR)
      mask |= 3U << (2*pin);
    if (offset == GM_AFRL && pin < 8) mask |= 15U << (4*pin);
    if (offset == GM_AFRH && pin >= 8) mask |= 15U << (4*(pin-8));
  }
  return mask;
}

bool GpioModel_Write(GpioModel *g, uint32_t offset, uint32_t value)
{
  if (offset > GM_AFRH || offset % 4 != 0 || offset == GM_IDR) return false;
  if (offset == GM_BSRR) {
    g->reg[GM_ODR/4] = ((g->reg[GM_ODR/4] & ~(value >> 16)) | value) & 0xffffU;
    return true;
  }
  if (offset == GM_LCKR) {
    if (g->lock_active) return true;
    uint16_t pins = (uint16_t)value;
    bool key = (value & 0x10000U) != 0;
    if (g->lock_step == 0 && key) { g->lock_candidate = pins; g->lock_step = 1; }
    else if (g->lock_step == 1 && !key && pins == g->lock_candidate) g->lock_step = 2;
    else if (g->lock_step == 2 && key && pins == g->lock_candidate) g->lock_step = 3;
    else g->lock_step = 0;
    g->reg[GM_LCKR/4] = pins;
    return true;
  }
  if (offset == GM_ODR || offset == GM_OTYPER) value &= 0xffffU;
  uint32_t mask = configuration_lock_mask(g, offset);
  g->reg[offset/4] = (g->reg[offset/4] & mask) | (value & ~mask);
  return true;
}

uint32_t GpioModel_Read(GpioModel *g, uint32_t offset)
{
  if (offset > GM_AFRH || offset % 4 != 0 || offset == GM_BSRR) return 0;
  if (offset == GM_IDR) {
    uint32_t pins = GpioModel_Pads(g);
    for (unsigned pin = 0; pin < 16; ++pin)
      if (((g->reg[GM_MODER/4] >> (2*pin)) & 3U) == 3U) pins &= ~(1U << pin);
    return pins;
  }
  if (offset == GM_LCKR && !g->lock_active) {
    if (g->lock_step == 3) {
      g->lock_active = true;
      g->locked = g->lock_candidate;
      g->reg[GM_LCKR/4] = 0x10000U | g->locked;
    }
    g->lock_step = 0;
  }
  return g->reg[offset/4];
}
