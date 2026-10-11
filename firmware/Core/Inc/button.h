#ifndef BUTTON_H
#define BUTTON_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {
  uint32_t duration_ms;
  uint32_t released_at_ms; /* Time the debounced release was reported. */
} ButtonEvent;
void Button_Init(uint32_t now_ms);
void Button_Update(uint32_t now_ms);
/* Single event mailbox: consume once after every update, from the same thread. */
bool Button_GetEvent(ButtonEvent *event);
/* Includes a candidate press and a release still being debounced. */
bool Button_IsActive(void);
/* Cancel an in-flight press/event, rearm only after a stable release. */
void Button_SuppressUntilReleased(void);
#endif
