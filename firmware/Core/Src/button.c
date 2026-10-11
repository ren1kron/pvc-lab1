#include "button.h"
#include "app_config.h"
#include "main.h"
#include <stddef.h>

static bool raw_pressed, stable_pressed, armed, tracking, pending;
static uint32_t raw_since_ms, pressed_at_ms;
static ButtonEvent completed;

static bool read_pressed(void)
{
  return HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET;
}

void Button_Init(uint32_t now_ms)
{
  raw_pressed = read_pressed();
  stable_pressed = raw_pressed;
  raw_since_ms = now_ms;
  pressed_at_ms = now_ms;
  armed = !raw_pressed; /* Ignore a button already held at reset. */
  tracking = false;
  pending = false;
}

void Button_Update(uint32_t now_ms)
{
  bool sample = read_pressed();
  if (sample != raw_pressed) {
    raw_pressed = sample;
    raw_since_ms = now_ms;
  }
  if ((raw_pressed != stable_pressed) &&
      ((uint32_t)(now_ms - raw_since_ms) >= BUTTON_DEBOUNCE_MS)) {
    stable_pressed = raw_pressed;
    if (stable_pressed) {
      pressed_at_ms = raw_since_ms;
      tracking = armed;
    } else {
      if (tracking && !pending) {
        completed.duration_ms = (uint32_t)(raw_since_ms - pressed_at_ms);
        completed.released_at_ms = now_ms;
        pending = true;
      }
      tracking = false;
    }
  }
  if (!raw_pressed && !stable_pressed &&
      ((uint32_t)(now_ms - raw_since_ms) >= BUTTON_DEBOUNCE_MS)) {
    armed = true;
  }
}

bool Button_GetEvent(ButtonEvent *event)
{
  if (!pending || event == NULL) return false;
  *event = completed;
  pending = false;
  return true;
}

bool Button_IsActive(void) { return raw_pressed || stable_pressed; }

void Button_SuppressUntilReleased(void)
{
  armed = false;
  tracking = false;
  pending = false;
}
