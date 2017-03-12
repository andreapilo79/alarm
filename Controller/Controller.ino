#include "Arduino.h"
#include <Wire.h>
#include <Blinker.h>

enum State {
	STANDBY, ALERTING, RUNNING, DAMAGE
};

/**
 * input pins
 */
#define SENSOR_PIN 10
#define ON_OFF_PIN 11

/**
 * out pins
 */
#define LED_PIN 13

/**
 * Starting delay: the system will be running after that
 */
unsigned int STARTING_DELAY = 1000;

/**
 * Internal state.
 */
State state = STANDBY;

unsigned long int alertingMillis;
unsigned long startingMillis;
unsigned long runningMillis;
unsigned int alertCount = 0;
unsigned int MAX_ALERT = 3;
boolean onOffPinOld = true;

Blinker blinker(LED_PIN, 1000, 1000);

/*
 * Send the sensor's state to the bus
 */
void onRequest() {
	String sensorState;
	if (state == ALERTING) {
		sensorState = "X11111111X";
	} else {
		sensorState = "X00000000X";
	}
	Wire.print(sensorState);
}

void setup() {
	Serial.begin(9600);

	blinker.begin();

	Wire.begin(1);
	Wire.onRequest(onRequest);
	Wire.onReceive(onReceive);

	pinMode(LED_PIN, OUTPUT);
	pinMode(ON_OFF_PIN, INPUT_PULLUP);
	pinMode(SENSOR_PIN, INPUT_PULLUP);

	Serial.println("System booting on state: STANDBY");
}

/**
 * function that executes whenever data is received from master
 * this function is registered as an event, see setup()
 */
void onReceive(int howMany) {
	if (Wire.available()) {
		//state = Wire.readString().toInt();
	}
}

void loop() {
	blinker.update();

	switch (state) {
	case STANDBY:
		standby();
		break;
	case RUNNING:
		running();
		break;
	case ALERTING:
		alerting();
		break;
	case DAMAGE:
		damage();
		break;
	default:
		Serial.println("Unknown State: " + state);
		state = DAMAGE;
		break;
	}
}

void changeState(State s) {
	Serial.println(state + " -> " + s);
	state = s;
}

void damage() {
//
// Should be read the onoff switch:
// if it is ON the next state will be STANDBY
//
	boolean onOffPin = digitalRead(ON_OFF_PIN);
	if (onOffPinOld && !onOffPin) {
		changeState(STANDBY);
	}
	onOffPinOld = onOffPin;
	blinker.newDuration(5000, 500);
}

void running() {
//
// Should be read the onoff switch:
// if it is OFF the next state will be STANDBY
//

	boolean onOffPin = digitalRead(ON_OFF_PIN);
	if (onOffPinOld && !onOffPin) {
		changeState(STANDBY);
	}

//
// Should be read the onoff switch:
// if it is ON the next state will be ALERTING
//
	else if (digitalRead(SENSOR_PIN) == LOW) {
		changeState(ALERTING);
		alertingMillis = millis();
	}
	onOffPinOld = onOffPin;
	blinker.newDuration(2000, 500);
}

void standby() {
//
// Should be read the on-off switch:
// if it is ON the next state will be RUNNING
//
	boolean onOffPin = digitalRead(ON_OFF_PIN);
	if (onOffPinOld && !onOffPin) {
		changeState(RUNNING);
	}
	onOffPinOld = onOffPin;
	blinker.newDuration(500, 2000);
}

void alerting() {
//
// Should be read the on-off switch:
// if it is OFF the next state will be STANDBY
//

	boolean onOffPin = digitalRead(ON_OFF_PIN);
	if (onOffPinOld && !onOffPin) {
		changeState(STANDBY);
	}
	onOffPinOld = onOffPin;
	blinker.newDuration(500, 500);
}
