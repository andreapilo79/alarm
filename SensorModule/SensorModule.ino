#include "Arduino.h"
#include <Wire.h>
#include <Blinker.h>

/**
 * system states
 */
#define STANDBY   0
#define ALERTING  1
#define RUNNING   2
#define DAMAGE    3

/**
 * input/out pins
 */
#define SENSOR_PIN 10
#define ON_OFF_PIN 11
#define LED_PIN 13

/**
 * Starting delay: the system will be running after that
 */
unsigned int STARTING_DELAY = 1000;

/**
 * Internal state.
 */
int state = STANDBY;

unsigned long int alertingMillis;
unsigned long startingMillis;
unsigned long runningMillis;
unsigned int alertCount = 0;
unsigned int MAX_ALERT = 3;
boolean onOffPinOld = true;

Blinker blinker(LED_PIN, 1000, 1000);


void onRequest()
{
	String sensorState;
	if (state == ALERTING)
	{
		sensorState = "X11111111X";
	}
	else
	{
		sensorState = "X00000000X";
	}
	Wire.print(sensorState);
}

void setup()
{
	Serial.begin(9600);

	blinker.begin();

	Wire.begin(1);
	Wire.onRequest(onRequest);
	Wire.onReceive(onReceive);

	pinMode(LED_PIN, OUTPUT);
	pinMode(ON_OFF_PIN, INPUT_PULLUP);  //Dichiaro il pin 2 come input
	pinMode(SENSOR_PIN, INPUT_PULLUP);  //Dichiaro il pin 2 come input

	Serial.println("System booting on STANDBY");
}

// TODO make more robust
boolean isValid(String state)
{
	if (state.length() > 0)
	{
		return true;
	}
	return false;
}

/**
 * function that executes whenever data is received from master
 * this function is registered as an event, see setup()
 */
void onReceive(int howMany)
{
	if (Wire.available())
	{
		state = Wire.readString().toInt();
	}
}

void loop()
{
	blinker.update();
	switch (state)
	{
	case STANDBY:
		handleStandByState();
		break;
	case RUNNING:
		handleRunningState();
		break;
	case ALERTING:
		handleAlertingState();
		break;
	case DAMAGE:
		handleDamageState();
		break;
	default:
		Serial.println("Unknown State: " + state);
		state = DAMAGE;
		break;
	}
}

void handleDamageState()
{
	boolean onOffPin = digitalRead(ON_OFF_PIN);
	if (onOffPinOld && !onOffPin)
	{
		Serial.println("DAMAGE -> STANDBY");
		state = STANDBY;
	}
	onOffPinOld = onOffPin;
	blinker.newDuration(5000, 500);
}

void handleRunningState()
{
	boolean onOffPin = digitalRead(ON_OFF_PIN);
	if (onOffPinOld && !onOffPin)
	{
		Serial.println("RUNNING -> STANDBY");
		state = STANDBY;

	}
	else if (digitalRead(SENSOR_PIN) == LOW)
	{
		Serial.println("RUNNING -> ALERTING");
		state = ALERTING;
		alertingMillis = millis();
	}
	onOffPinOld = onOffPin;
	blinker.newDuration(2000, 500);
}

void handleStandByState()
{
	boolean onOffPin = digitalRead(ON_OFF_PIN);
	if (onOffPinOld && !onOffPin)
	{
		Serial.println("STANDBY -> RUNNING");
		state = RUNNING;
	}
	onOffPinOld = onOffPin;
	blinker.newDuration(500, 2000);
}

void handleAlertingState()
{
	boolean onOffPin = digitalRead(ON_OFF_PIN);
	if (onOffPinOld && !onOffPin)
	{
		Serial.println("ALERTING -> STANDBY");
		state = STANDBY;
	}
	onOffPinOld = onOffPin;
	blinker.newDuration(500, 500);
}
