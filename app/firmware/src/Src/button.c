#include "button.h"
ButtonEvent_t checkButtonPress(Pin_t button){
	static uint8_t btn_was_pressed = 0;
	static uint32_t btn_press_start = 0;
	ButtonEvent_t action = BTN_NO_EVENT;
	uint8_t btn_state = !(button.port->IDR & (1 << button.number));
	if(btn_state && !btn_was_pressed){
		btn_press_start = millis();
		btn_was_pressed = 1;
	}
	else if(!btn_state && btn_was_pressed){
		uint32_t press_duration = millis() - btn_press_start;
		if(press_duration > MODE_BUTTON_DEBOUNCE_MS){
			if(press_duration < LONG_PRESS_MS){
				action = BTN_SHORT_CLICK;
			}
			else{
				action = BTN_LONG_CLICK;
			}
		}
		btn_was_pressed = 0;
	}
	return action;
}
