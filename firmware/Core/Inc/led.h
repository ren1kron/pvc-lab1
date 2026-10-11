#ifndef LED_H
#define LED_H
typedef enum { LED_GREEN, LED_RED, LED_YELLOW } LedId;
/* On selects one colour of the shared red/yellow LED. Green is independent. */
void LED_On(LedId led);
void LED_Off(LedId led);
void LED_AllOff(void);
#endif
