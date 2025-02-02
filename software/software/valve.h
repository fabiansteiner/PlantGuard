/*
 * valve.h
 *
 * Created: 02.11.2022 16:19:30
 *  Author: SteinerF
 *
 *	Interface to the motor for opening and closing the valve + Button for valve state feedback.
 *  States: Open, Closed, CurrentSensorError
 *  Peripherals needed: Timer Interrupt, Button Interrupt, ADC
 */ 


#ifndef VALVE_H_
#define VALVE_H_

#define OPEN_CURRENT 0.40
#define CLOSE_CURRENT 0.79
//#define SAMPLE_ACCUM ADC_SAMPNUM_ACC16_gc

#define SAMPLE_ACCUM_OPEN ADC_SAMPNUM_ACC8_gc
#define SAMPLE_ACCUM_CLOSE ADC_SAMPNUM_ACC32_gc

//#define MAX_MOTOR_CLOSE_DELAY 150					//when Motor Parts are printed in PC Blend
#define MAX_MOTOR_CLOSE_DELAY 50					//when Motor Parts are printed in ASA

#define OPEN_TIMEOUT 6000
#define CLOSE_TIMEOUT 6000

#define PIN_MOTORPLUS 1		//PORTB
#define PIN_MOTORMINUS 6	//PORTA
#define PIN_MOTORSTOP 5		//PORTA

#define MAX_VOL 9.9
#define MIN_VOL 4.1
#define MAX_CUR 1.0			//Bei 0.2 Ohm Shunt

#define OPEN_CURRENT_LIMIT_ADC (uint16_t)(RES_10BIT / MAX_CUR * OPEN_CURRENT)
#define CLOSE_CURRENT_LIMIT_ADC (uint16_t)(RES_10BIT / MAX_CUR * CLOSE_CURRENT)

typedef enum {UNDEFINED=1, OPEN=2, CLOSED=5, OPENING = 20, CLOSING = 40}  valveState;
typedef enum {LOW_VOLTAGE=1 , HIGH_CURRENT=2, VALVE_TIMEOUT=3, NO_ERROR}  valveError;

/*
Errors Meanings
	VOLTAGELOW:		When Voltage got too low (<3,9V) when the valve closed last time, But Valve is Closed and in Safe Position
	HIGH_CURRENT:	When Current got too high when closing (the valve could potentially damage itself without this), But valve is PROBABLY closed and in safe position, Causes: Button is damaged or out of Range (Housing Deformation), or threshold is set too low
	VALVE_TIMEOUT:	When the Motor was driving for more than 3 seconds and the valve is not closed AND did not get shut off because of high current, valve is PROBABLY open and in an unsafe position, Causes: Damaged Motor or motor mechanics, Also occurs in the INIT Phase where user wants to bring motor shaft in suitable position
*/

//Figure out state of valve, if not closed, close it
void initializeValve();

void openValve();

void closeValve();

void changeMotorState();

valveState getValveState();

valveError getValveError();



#endif /* VALVE_H_ */