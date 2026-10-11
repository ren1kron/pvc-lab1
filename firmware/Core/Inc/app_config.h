#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define BUTTON_DEBOUNCE_MS 30U
#define MORSE_DASH_THRESHOLD_MS 400U
#define MORSE_DOT_MS 200U
#define MORSE_DASH_MS 600U
#define MORSE_GAP_MS 200U
#define MORSE_INPUT_TIMEOUT_MS 1500U
#define MORSE_FEEDBACK_MS 150U
/* The 23 Sep 2026 manual specifies up to eight presses. Override for experiments. */
#ifndef MORSE_CAPACITY
#define MORSE_CAPACITY 8U
#endif
#if MORSE_CAPACITY < 1 || MORSE_CAPACITY > 65535
#error "MORSE_CAPACITY must fit a nonzero uint16_t"
#endif
#endif
