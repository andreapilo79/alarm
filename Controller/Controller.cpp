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
#include <Blinker.h>
#include <LiquidCrystal_I2C.h>
#include <Alarm.h>

/**
 * input pins
 */
#define SENSOR_PIN_1 10
#define SENSOR_PIN_2 11
#define SENSOR_PIN_3 12
#define SENSOR_PIN_4 14

#define ON_PIN 8
#define OFF_PIN 8

/**
 * out pins
 */
#define LED_PIN 13

/**
 * Starting delay: the system will be running after that
 */
unsigned int STARTING_DELAY = 1000;

/**
 * Internal status.
 */

LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long int alertingMillis;
unsigned long runningMillis;
unsigned int alertCount = 0;
unsigned int MAX_ALERT = 3;

Blinker blinker(LED_PIN, 1000, 1000);
Status status = new Status(STANDBY);
Button onButton = new Button(ON_PIN);
Button offButton = new Button(OFF_PIN);

void setup() {
	Serial.begin(9600);

	blinker.begin();

	pinMode(LED_PIN, OUTPUT);
	pinMode(SENSOR_PIN_1, INPUT_PULLUP);
	pinMode(SENSOR_PIN_2, INPUT_PULLUP);
	pinMode(SENSOR_PIN_3, INPUT_PULLUP);
	pinMode(SENSOR_PIN_4, INPUT_PULLUP);

	info("System booting ...");
}

void loop() {
	blinker.update();
	switch (status) {
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
		info("Unknown Status: " + status);
		status = DAMAGE;
		break;
	}
}

void info(char * msg) {
	Serial.println("status " + status + " - " + msg);
}

/**
 * Should be read the onoff switch:
 * if it is ON the next status will be STANDBY
 */
void damage() {
	if (offButton.read()) {
		status.setNewStatus(STANDBY);
	} else {
		info("Error!");
	}
	blinker.newDuration(5000, 500);
}

/*
 * Should be read the onoff switch:
 * if it is OFF the next status will be STANDBY
 * Should be read the onoff switch:
 *  if it is ON the next status will be ALERTING
 */
void running() {

	if (offButton.read()) {
		status.setNewStatus(STANDBY);
	} else if ((millis() - status.getMillis()) < 10000) {
		info("Running since less than 10s.");
	} else if (digitalRead(SENSOR_PIN_1) == LOW) {
		status.setNewStatus(ALERTING);
	}
	blinker.newDuration(2000, 500);
}

/*
 * Should be read the on-off switch:
 * if it is ON the next status will be RUNNING
 */
void standby() {
	if (onButton.read()) {
		status.setNewStatus(RUNNING);
	}
	blinker.newDuration(500, 2000);
}

/*
 * Should be read the on-off switch:
 * if it is OFF the next status will be STANDBY
 */
void alerting() {
	if (offButton.read()) {
		status.setNewStatus(STANDBY);
	} else if ((millis() - status.getMillis()) > 60000) {
		status.setNewStatus(RUNNING);
		runningMillis = millis();
	}
	blinker.newDuration(500, 500);
}
