/*
 * LEDs.h
 *
 * Created: 02.11.2022 16:17:20
 *  Author: SteinerF
 */ 


#ifndef LEDS_H_
#define LEDS_H_

#include "UserInterface.h"

#define PIN_GREENLED 3		//PORTB
#define PIN_REDLED 7		//PORTA
#define BLUE_LED 2		//PORTB

#define LED_CONFIRM 1
#define LED_TIMEOUT 0
#define LED_SHUTDOWN 2
#define LED_STARTUP 3

#define VERSION_ANIMATION_COUNT1 (6+VERSION_MAJOR*2)
#define VERSION_ANIMATION_COUNT2 (6+VERSION_MAJOR*2+VERSION_MINOR*2)
#define VERSION_ANIMATION_COUNT3 (6+VERSION_MAJOR*2+VERSION_MINOR*2+4)

typedef enum {NO_ANIMATION, A_BATTERY, A_SELECTTHRESHOLD , A_SELECTINTERVAL,A_CHANGETHRESHOLD,A_CHANGEINTERVAL, A_TRANSITIONING_CONF, A_TRANSITIONING_TIMEOUT,A_TRANSITIONING_SHUTDOWN, A_TRANSITIONING_STARTUP, A_MANUALIRRIGATION, A_SHOWERRORS} currentLEDAnimation;

void initLEDs();
void changeLEDAnimation(state_change change);
void cycleLEDAnimation();
currentLEDAnimation getLEDAnimation();


#endif /* LEDS_H_ */