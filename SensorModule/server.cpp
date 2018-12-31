/*
 Copyright (c) 2017 Andrea Pilo.  All right reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "Arduino.h"

//#include <Blinker.h>

enum State {
	DISARMED, ALERTING, ARMED, DAMAGE
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
unsigned int MAX_ALERTING_TIME = 1000;
unsigned int MAX_ALERTING_COUNT = 3;
unsigned int DELAY_TIME = 1000;

/**
 * Internal state.
 */
State state = DISARMED;

unsigned long int alertingMillis;
unsigned int alertCount = 0;
boolean onOffPinOld = true;

boolean isSignaling = false;

//Blinker blinker(LED_PIN, 1000, 1000);

void setup() {
	Serial.begin(9600);

	//blinker.begin();

	pinMode(LED_PIN, OUTPUT);
	pinMode(ON_OFF_PIN, INPUT_PULLUP);
	pinMode(SENSOR_PIN, INPUT_PULLUP);

	Serial.println("System booting on state: DISARMED");
}

void changeState(State s) {
	Serial.println(state + " -> " + s);
	state = s;
}

/**
 * This is the system status in case of internal errors
 *
 * Should be read the ON_OFF_PIN switch:
 * if ON the next state will be DISARMED
 */
void doDamage() {

	boolean onOffPin = digitalRead(ON_OFF_PIN);
	if (onOffPinOld && !onOffPin) {
		changeState(DISARMED);
	}
	onOffPinOld = onOffPin;
	//blinker.newDuration(5000, 500);
}

/*
 * This is the state in which the system must detect intrusions
 *
 * Should be read the ON_OFF_PIN:
 * if OFF the next state is DISARMED
 *
 * Should be read the SENSOR_PIN:
 * if ON the next state is ALERTING
 */
void doArmed() {

	boolean onOffPin = digitalRead(ON_OFF_PIN);
	if (onOffPinOld && !onOffPin) {
		changeState(DISARMED);
	} else if (digitalRead(SENSOR_PIN) == LOW) {
		changeState(ALERTING);
		alertingMillis = millis();
	}
	onOffPinOld = onOffPin;
	//blinker.newDuration(2000, 500);
}

/*
 * This is the state in which the system is ready to start
 *
 * Should be read the ON_OFF_PIN:
 * if ON the next state is ARMED
 *
 * Does not matter the state of SENSOR_PIN
 */
void doDisarmed() {
	boolean onOffPin = digitalRead(ON_OFF_PIN);
	if (onOffPinOld && !onOffPin) {
		changeState(ARMED);
		alertCount = 0;
	}
	onOffPinOld = onOffPin;
	//blinker.newDuration(500, 2000);
}

/**
 * This is the state in which the system is when happens an intrusion
 *
 * Should be read the ON_OFF_PIN:
 * if OFF the next state is ARMED
 */
void doAlerting() {
	boolean onOffPin = digitalRead(ON_OFF_PIN);
	if (onOffPinOld && !onOffPin) {
		changeState(DISARMED);
	}
	else if (millis() - alertingMillis > MAX_ALERTING_TIME) {
		changeState(ARMED);
	} else if (alertCount > MAX_ALERTING_COUNT) {
		changeState(DISARMED);
	} else if (millis() - alertingMillis > DELAY_TIME) {
		if (!isSignaling) {
			isSignaling = true;
			alertCount++;
		}
	}
	onOffPinOld = onOffPin;
	//blinker.newDuration(500, 500);
}

void loop() {
	//blinker.update();

	switch (state) {
	case DISARMED:
		doDisarmed();
		break;
	case ARMED:
		doArmed();
		break;
	case ALERTING:
		doAlerting();
		break;
	case DAMAGE:
		doDamage();
		break;
	default:
		Serial.println("Unknown State: " + state);
		changeState(DAMAGE);
		break;
	}
}
