#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "fake_hal.h"
#include "button.h"
#include "led.h"
#include "morse.h"
#include "app_config.h"

static unsigned tests;
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __func__, __LINE__, #x); exit(1); } } while (0)
#define RUN(f) do { f(); ++tests; puts("PASS " #f); } while (0)
static uint32_t now;
static void driver_tick(uint32_t ms) { now += ms; Button_Update(now); }
static void reset_driver(uint32_t start, bool held)
{ Fake_Reset(); Fake_Button(held); now = start; Button_Init(now); }

static void led_truth_table(void)
{
  Fake_Reset(); LED_AllOff(); CHECK(fake_d.output == 0);
  LED_On(LED_GREEN); CHECK(Fake_Green());
  LED_On(LED_YELLOW); CHECK(Fake_Yellow() && !Fake_Red() && Fake_Green());
  LED_On(LED_RED); CHECK(Fake_Red() && !Fake_Yellow() && Fake_Green());
  LED_Off(LED_YELLOW); CHECK(Fake_Red());
  LED_Off(LED_RED); CHECK(!Fake_Red() && !Fake_Yellow());
  LED_On((LedId)99); CHECK(Fake_Green());
  LED_Off(LED_GREEN); CHECK(fake_d.output == 0);
  LED_On(LED_YELLOW); LED_AllOff(); CHECK(fake_d.output == 0);
}
static void button_stable_debounce_and_duration(void)
{
  ButtonEvent event;
  reset_driver(0, false);
  Fake_Button(true); driver_tick(100); driver_tick(29);
  CHECK(Button_IsActive()); CHECK(!Button_GetEvent(&event));
  driver_tick(1); driver_tick(70);
  Fake_Button(false); driver_tick(0); driver_tick(29);
  CHECK(Button_IsActive()); CHECK(!Button_GetEvent(&event));
  driver_tick(1); CHECK(!Button_IsActive()); CHECK(Button_GetEvent(&event));
  CHECK(event.duration_ms == 100); CHECK(event.released_at_ms == 230);
  CHECK(!Button_GetEvent(&event));
}
static void button_bounce_and_single_event(void)
{
  ButtonEvent event;
  reset_driver(0, false);
  Fake_Button(true); driver_tick(10); Fake_Button(false); driver_tick(10);
  driver_tick(30); CHECK(!Button_GetEvent(&event));
  Fake_Button(true); driver_tick(10); driver_tick(30); driver_tick(40);
  Fake_Button(false); driver_tick(10); Fake_Button(true); driver_tick(10);
  Fake_Button(false); driver_tick(10); driver_tick(29);
  CHECK(!Button_GetEvent(&event)); driver_tick(1);
  CHECK(Button_GetEvent(&event)); CHECK(event.duration_ms == 100);
  for (unsigned i = 0; i < 100; ++i) { driver_tick(1); CHECK(!Button_GetEvent(&event)); }
}
static void button_held_at_boot_and_suppression(void)
{
  ButtonEvent event;
  reset_driver(0, true); driver_tick(3000); CHECK(!Button_GetEvent(&event));
  Fake_Button(false); driver_tick(0); driver_tick(30); CHECK(!Button_GetEvent(&event));
  Fake_Button(true); driver_tick(20); driver_tick(30);
  Button_SuppressUntilReleased(); driver_tick(1000);
  Fake_Button(false); driver_tick(0); driver_tick(30); CHECK(!Button_GetEvent(&event));
  Fake_Button(true); driver_tick(10); driver_tick(30); driver_tick(70);
  Fake_Button(false); driver_tick(0); driver_tick(30);
  CHECK(!Button_GetEvent(NULL)); CHECK(Button_GetEvent(&event)); CHECK(event.duration_ms == 100);
}
static void button_wraparound(void)
{
  ButtonEvent event;
  reset_driver(UINT32_MAX - 50U, false);
  Fake_Button(true); driver_tick(1); driver_tick(30); driver_tick(470);
  Fake_Button(false); driver_tick(0); driver_tick(30);
  CHECK(Button_GetEvent(&event)); CHECK(event.duration_ms == 500);
}

/* Production Button_Update and Morse_Update, with only time and GPIO simulated. */
static void tick(void) { Button_Update(now); Morse_Update(now); }
static void advance(uint32_t ms) { for (uint32_t i = 0; i < ms; ++i) { ++now; tick(); } }
static void reset_app(uint32_t start) { reset_driver(start, false); Morse_Init(); tick(); }
static void press(uint32_t duration)
{
  Fake_Button(true); tick(); advance(duration);
  Fake_Button(false); tick(); advance(BUTTON_DEBOUNCE_MS);
}
static void morse_dot_feedback(void)
{
  reset_app(0); press(100); CHECK(Morse_GetLength() == 1);
  CHECK(Morse_GetState() == MORSE_INPUT); CHECK(Fake_Yellow() && !Fake_Red());
  advance(149); CHECK(Fake_Yellow()); advance(1); CHECK(!Fake_Yellow());
}
static void morse_dash_boundary(void)
{
  reset_app(0); press(399); CHECK(Fake_Yellow());
  press(400); CHECK(Fake_Red() && !Fake_Yellow());
  CHECK(Morse_GetLength() == 2);
}
static void morse_empty_and_one_event(void)
{
  reset_app(0); advance(3000); CHECK(Morse_GetState() == MORSE_IDLE);
  CHECK(!Fake_Green()); press(100); advance(1000); CHECK(Morse_GetLength() == 1);
}
static void morse_order_and_exact_timing(void)
{
  reset_app(0); press(100); advance(100); press(500); advance(100); press(100);
  CHECK(Morse_GetLength() == 3); advance(1499); CHECK(!Fake_Green());
  advance(1); CHECK(Morse_GetState() == MORSE_PLAYBACK && Fake_Green());
  advance(199); CHECK(Fake_Green()); advance(1); CHECK(!Fake_Green());
  advance(199); CHECK(!Fake_Green()); advance(1); CHECK(Fake_Green());
  advance(599); CHECK(Fake_Green()); advance(1); CHECK(!Fake_Green());
  advance(199); CHECK(!Fake_Green()); advance(1); CHECK(Fake_Green());
  advance(199); CHECK(Fake_Green()); advance(1); CHECK(!Fake_Green());
  CHECK(Morse_GetState() == MORSE_IDLE && Morse_GetLength() == 0);
  advance(4000); CHECK(Morse_GetState() == MORSE_IDLE && !Fake_Green());
}
static void morse_timeout_while_held(void)
{
  reset_app(0); press(100); advance(1490);
  Fake_Button(true); tick(); advance(3000);
  CHECK(Morse_GetState() == MORSE_INPUT && !Fake_Green());
  Fake_Button(false); tick(); advance(30);
  CHECK(Morse_GetLength() == 2); CHECK(Fake_Red());
  advance(1499); CHECK(!Fake_Green()); advance(1); CHECK(Fake_Green());
}
static void morse_bounce_cannot_add_symbol(void)
{
  reset_app(0); Fake_Button(true); tick(); advance(29);
  Fake_Button(false); tick(); advance(2000);
  CHECK(Morse_GetLength() == 0 && Morse_GetState() == MORSE_IDLE);
}
static void morse_overflow_preserves_first_symbols(void)
{
  reset_app(0);
  for (unsigned i = 0; i < MORSE_CAPACITY; ++i) press(70);
  press(500); CHECK(Morse_GetLength() == MORSE_CAPACITY);
  /* Rejected release restarts the timeout; its classification still has feedback. */
  CHECK(Fake_Red()); advance(1499); CHECK(!Fake_Green()); advance(1);
  for (unsigned i = 0; i < MORSE_CAPACITY; ++i) {
    CHECK(Fake_Green()); advance(199); CHECK(Fake_Green());
    advance(1); CHECK(!Fake_Green());
    if (i + 1 < MORSE_CAPACITY) advance(200);
  }
  CHECK(Morse_GetState() == MORSE_IDLE && Morse_GetLength() == 0);
}
static void morse_repeated_transmissions(void)
{
  reset_app(0);
  for (unsigned i = 0; i < 3; ++i) {
    advance(31); press(500); advance(1500); CHECK(Fake_Green());
    advance(599); CHECK(Fake_Green()); advance(1);
    CHECK(Morse_GetState() == MORSE_IDLE && !Fake_Green());
  }
}
static void morse_wraparound_all_timers(void)
{
  /* Place wrap inside debounce, feedback, inactivity, pulse and gap respectively. */
  const uint32_t offsets[] = {20, 200, 1000, 2220, 2430};
  for (unsigned i = 0; i < sizeof offsets / sizeof offsets[0]; ++i) {
    reset_app(UINT32_MAX - offsets[i]); press(100); press(500);
    advance(1500); CHECK(Fake_Green()); advance(200); CHECK(!Fake_Green());
    advance(200); CHECK(Fake_Green()); advance(600);
    CHECK(Morse_GetState() == MORSE_IDLE && !Fake_Green());
  }
}
static void morse_ignores_playback_press(void)
{
  reset_app(0); press(500); advance(1500); CHECK(Fake_Green());
  press(100); CHECK(Morse_GetLength() == 1);
  advance(470); CHECK(Morse_GetState() == MORSE_IDLE);
  advance(2000); CHECK(Morse_GetLength() == 0 && !Fake_Green());
}
static void morse_ignores_press_spanning_playback_end(void)
{
  reset_app(0); press(100); advance(1500); advance(195);
  Fake_Button(true); tick(); advance(1000);
  CHECK(Morse_GetState() == MORSE_IDLE);
  Fake_Button(false); tick(); advance(30);
  CHECK(Morse_GetLength() == 0 && !Fake_Red() && !Fake_Yellow());
  press(100); CHECK(Morse_GetLength() == 1 && Fake_Yellow());
}
static void morse_accepts_first_press_after_playback(void)
{
  reset_app(0); press(100); advance(1500); advance(200);
  CHECK(Morse_GetState() == MORSE_IDLE);
  /* Fresh edge at the very next poll, without an extra released poll to rearm. */
  Fake_Button(true); advance(1); advance(100);
  Fake_Button(false); tick(); advance(30);
  CHECK(Morse_GetLength() == 1 && Fake_Yellow());
}
static void morse_late_poll_preserves_visible_gap(void)
{
  reset_app(0); press(100); press(100); advance(1500);
  now += 1000; tick(); CHECK(!Fake_Green());
  advance(199); CHECK(!Fake_Green()); advance(1); CHECK(Fake_Green());
  advance(200); CHECK(Morse_GetState() == MORSE_IDLE);
}

int main(void)
{
  RUN(led_truth_table);
  RUN(button_stable_debounce_and_duration);
  RUN(button_bounce_and_single_event);
  RUN(button_held_at_boot_and_suppression);
  RUN(button_wraparound);
  RUN(morse_dot_feedback);
  RUN(morse_dash_boundary);
  RUN(morse_empty_and_one_event);
  RUN(morse_order_and_exact_timing);
  RUN(morse_timeout_while_held);
  RUN(morse_bounce_cannot_add_symbol);
  RUN(morse_overflow_preserves_first_symbols);
  RUN(morse_repeated_transmissions);
  RUN(morse_wraparound_all_timers);
  RUN(morse_ignores_playback_press);
  RUN(morse_ignores_press_spanning_playback_end);
  RUN(morse_accepts_first_press_after_playback);
  RUN(morse_late_poll_preserves_visible_gap);
  printf("%u tests passed\n", tests);
  return 0;
}
