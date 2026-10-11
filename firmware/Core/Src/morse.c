#include "morse.h"
#include "app_config.h"
#include "button.h"
#include "led.h"
#include <stdbool.h>

typedef enum { DOT, DASH } Symbol;
static Symbol sequence[MORSE_CAPACITY];
static uint16_t length, playback_index;
static MorseState state;
static bool pulse_on, feedback_on;
static uint32_t last_release_ms, playback_since_ms, feedback_since_ms;

static void feedback_off(void)
{
  LED_Off(LED_RED);
  LED_Off(LED_YELLOW);
  feedback_on = false;
}

void Morse_Init(void)
{
  LED_AllOff();
  length = 0;
  playback_index = 0;
  state = MORSE_IDLE;
  pulse_on = false;
  feedback_on = false;
  last_release_ms = playback_since_ms = feedback_since_ms = 0;
}

void Morse_Update(uint32_t now_ms)
{
  ButtonEvent event;
  bool released = Button_GetEvent(&event); /* Always drain, including playback. */

  if (feedback_on && (uint32_t)(now_ms - feedback_since_ms) >= MORSE_FEEDBACK_MS)
    feedback_off();

  if (state == MORSE_PLAYBACK) {
    uint32_t interval = pulse_on ?
        (sequence[playback_index] == DOT ? MORSE_DOT_MS : MORSE_DASH_MS) : MORSE_GAP_MS;
    if ((uint32_t)(now_ms - playback_since_ms) < interval) return;
    /* At most one transition per call. A late poll never collapses a visible gap. */
    playback_since_ms = now_ms;
    if (pulse_on) {
      LED_Off(LED_GREEN);
      pulse_on = false;
      ++playback_index;
      if (playback_index == length) {
        length = 0;
        state = MORSE_IDLE;
        Button_SuppressUntilReleased();
      }
    } else {
      LED_On(LED_GREEN);
      pulse_on = true;
    }
    return;
  }

  if (released) {
    Symbol symbol = event.duration_ms < MORSE_DASH_THRESHOLD_MS ? DOT : DASH;
    if (length < MORSE_CAPACITY) sequence[length++] = symbol;
    /* A full buffer keeps the first entries. Further releases still give feedback
     * and postpone playback, so it never starts in the middle of ongoing input. */
    last_release_ms = event.released_at_ms;
    state = MORSE_INPUT;
    feedback_off();
    LED_On(symbol == DOT ? LED_YELLOW : LED_RED);
    feedback_on = true;
    feedback_since_ms = now_ms;
  }

  if (state == MORSE_INPUT && length != 0 && !Button_IsActive() &&
      (uint32_t)(now_ms - last_release_ms) >= MORSE_INPUT_TIMEOUT_MS) {
    feedback_off();
    Button_SuppressUntilReleased();
    playback_index = 0;
    playback_since_ms = now_ms;
    pulse_on = true;
    state = MORSE_PLAYBACK;
    LED_On(LED_GREEN);
  }
}

MorseState Morse_GetState(void) { return state; }
uint16_t Morse_GetLength(void) { return length; }
