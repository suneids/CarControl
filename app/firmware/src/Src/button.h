#ifndef BUTTON_H
#define BUTTON_H
#include "myhal.h"
#define MODE_BUTTON_DEBOUNCE_MS 150
#define LONG_PRESS_MS 1000
typedef enum{
	BTN_NO_EVENT = 0,
	BTN_SHORT_CLICK,
	BTN_LONG_CLICK
} ButtonEvent_t;

ButtonEvent_t checkButtonPress(Pin_t button);
#endif
