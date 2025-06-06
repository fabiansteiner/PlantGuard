#define F_CPU 3333333

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdbool.h>


#include "UserInterface.h"
#include "LEDs.h"
#include "ADC.h"
#include "valve.h"
#include "SoilMoistureSensor.h"
#include "EEPROM.h"

UIstate uiState = SHOWNOTHING;


uint8_t soilLevel = 4;
uint8_t multiplicator = 1;


thresholds currentThresholds;
uint8_t secondCounter = 0;
uint16_t milliSecondCounter = 0;


void changeThresholds(){
	// Magic number 102 because ADC returns value between 0 and 1023, we have 8 configurable threshold levels and 2 two upper levels are reserved to give the sensor a chance to detect when sensor is wet at the upper levels. 1023/10 = ~102
	// Closing threshold has to be higher because if ADC delivers exactly a value on the borderline its likely that the valve cant decide if it should be open or closed.
	
	currentThresholds.tresholdOpen = getSensorTreshold(soilLevel-1);
	currentThresholds.thresholdClose = currentThresholds.tresholdOpen + 25;

	//This is the linear approach
	/*
	if(LINEAR_SENSOR){
		currentThresholds.tresholdOpen = soilLevel * ADCSTEPS;
		currentThresholds.thresholdClose = currentThresholds.tresholdOpen + ADCSTEPS/2;
	}else{
		currentThresholds.tresholdOpen = adc_trehsolds[soilLevel-1];
		currentThresholds.thresholdClose = adc_trehsolds[soilLevel-1]+25;
	}
	*/
	

	
}

void initUI(){
	
	//Check if its the very first startup
	uint8_t readByte = FLASH_0_read_eeprom_byte(10);
	if(readByte != 221){
		//If it is the very first startup, write magic number to magic position to indicate that this is not the first startup
		FLASH_0_write_eeprom_byte(10, 221);
		//Also write standard values into eeprom
		FLASH_0_write_eeprom_byte(0, soilLevel);
		FLASH_0_write_eeprom_byte(1, multiplicator);
	}else{
		//If its is not the very first startup get settings from eeprom
		readByte = FLASH_0_read_eeprom_byte(0);
		if(readByte <= 8){
			soilLevel = readByte;
		}
		readByte = FLASH_0_read_eeprom_byte(1);
		if(readByte <= 5){
			multiplicator = readByte;
		}
	}
	

	changeThresholds();
}

void increaseThreshold(){
	soilLevel++;
	if(soilLevel >8){ soilLevel = 1;}

	FLASH_0_write_eeprom_byte(0, soilLevel);

	changeThresholds();
	
}

void increaseMultiplicator(){
	multiplicator++;
	if(multiplicator > 5) multiplicator = 1;

	FLASH_0_write_eeprom_byte(1, multiplicator);
}



thresholds getCurrentThresholds(){
	return currentThresholds;
}

uint8_t getCurrentMultiplicator(){
	
	return multiplicator;
}

UIstate getUIState(){
	return uiState;
}


/*
 && getValveState()==CLOSED){
	 uiState = MANUALIRRIGATION;
	 change = FROM_SHOWBATTERY_TO_MANUALIRRIGATION;
 }
*/


state_change changeUIState(pressType button_press){
	//State machine
	state_change change = NO_CHANGE;

	if(button_press != NONE && button_press != VERYLONG){

		switch(uiState) {
			case SHOWNOTHING:
			uiState = SHOWBATTERY;
			change = FROM_SHOWNOTHING_TO_SHOWBATTERY;
			break;
			case SHOWBATTERY:
			if(button_press == LONG){
				uiState = SELECTTHRESHOLD;
				change = FROM_SHOWBATTERY_TO_SELECTTHRESHOLD;
			}else if (button_press == SHORT){
				uiState = SHOWSOILMOISTURE;
				change = FROM_SHOWBATTERY_TO_SHOWSOILMOISTURE;
			}
			break;
			case SHOWSOILMOISTURE:
			if(button_press == LONG && getValveState()==CLOSED){
				uiState = MANUALIRRIGATION;
				change = FROM_SHOWSOILMOISTURE_TO_MANUALIRRIGATION;
			}else if (button_press == SHORT){
				uiState = SHOWBATTERY;
				change = FROM_SHOWSOILMOISTURE_TO_SHOWBATTERY;
			}
			break;
			case MANUALIRRIGATION:
			if((button_press == SHORT || button_press == LONG) && getValveState()==OPEN){
				uiState = SHOWSOILMOISTURE;
				change = FROM_MANUALIRRIGATION_TO_SHOWSOILMOISTURE;
			}
			break;
			case SELECTTHRESHOLD:
			if(button_press == SHORT){
				uiState = SELECTMULTIPLICATOR;
				change = FROM_SELECTTHRESHOLD_TO_SELECTMULTIPLICATOR;
				}else if (button_press == LONG){
				uiState = CHANGETHRESHOLD;
				change = FROM_SELECTTHRESHOLD_TO_CHANGETHRESHOLD;
			}
			break;
			case SELECTMULTIPLICATOR:
			if(button_press == SHORT){
				uiState = SELECTTHRESHOLD;
				change = FROM_SELECTMULTIPLICATOR_TO_SELECTTHRESHOLD;
				}else if (button_press == LONG){
				uiState = CHANGEMULTIPLICATOR;
				change = FROM_SELECTMULTIPLICATOR_TO_CHANGEMULTIPLICATOR;
			}
			break;
			case CHANGETHRESHOLD:
			if(button_press == SHORT){
				increaseThreshold();
				change = THRESHOLD_CHANGED;
				}else if (button_press == LONG){
				uiState = SHOWNOTHING;
				change = UI_OFF;
			}
			break;
			case CHANGEMULTIPLICATOR:
			if(button_press == SHORT){
				increaseMultiplicator();
				change = MULTIPLICATOR_CHANGED;
				}else if (button_press == LONG){
				uiState = SHOWNOTHING;
				change = UI_OFF;
			}
			break;
			case ERRORSTATE:
			if(button_press == LONG && getValveError() == WRONG_SENSOR_PLACEMENT){
				//Escape Error State
				setValveError(NO_ERROR);
				uiState = SHOWBATTERY;
				change = FROM_SHOW_ERROR_TO_SHOWBATTERY;
			}
			break;
			default: break;
		}
		
		
	}else if(button_press == VERYLONG && uiState != ERRORSTATE){
		uiState = SHOWNOTHING;
		change = UI_SHUTDOWN;
	}

	return change;

	
}


pressType senseMagneticSwitch(){
	static uint16_t buttonTimeCounter = 0;
	static uint16_t buttonTimeCounterForSwitchOFF = 0;
	static uint8_t alreadyPressed = 0;
	pressType press = NONE;

	if((PORTB_IN & (1<<PIN_MAGNETSWITCH))!=0){
		//User Pressed something, reset Timeout Variables
		milliSecondCounter = 0;
		secondCounter = 0;

		buttonTimeCounter++;
		buttonTimeCounterForSwitchOFF++;
		
		if(buttonTimeCounter>=75 && !alreadyPressed){
			press = LONG;
			buttonTimeCounter = 0;
			alreadyPressed = 1;
		}
		
		if(buttonTimeCounterForSwitchOFF>=300){
			press = VERYLONG;
			buttonTimeCounterForSwitchOFF = 0;
			
		}
	}else{
		if(buttonTimeCounter>=0 && !alreadyPressed){
			if(buttonTimeCounter >= 2){
				press = SHORT;
			}
		}
		
		buttonTimeCounter=0;
		buttonTimeCounterForSwitchOFF=0;
		alreadyPressed = 0;
		
	}

	return press;
}

state_change countUITimeOut(){
	state_change change = NO_CHANGE;

	if (uiState != SHOWNOTHING){
		
		milliSecondCounter = milliSecondCounter + MAINLOOP_DELAY;
		
		if(milliSecondCounter >= 1000){
			secondCounter++;
			milliSecondCounter = 0;
		}
		
		if(secondCounter >= UI_TIMEOUT){
			
			uiState = SHOWNOTHING;
			change = UI_OFF_WITHOUT_CONFIRMING;
				
			
			
			secondCounter = 0;
			milliSecondCounter = 0;
		}
		
	}else if (milliSecondCounter >= 0 || secondCounter >= 0){
		milliSecondCounter = 0;
		secondCounter = 0;
	}

	return change;
}




