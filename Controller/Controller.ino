#include "Arduino.h"
#include <Wire.h>
#include <Blinker.h>

/**
 * States
 */
#define STANDBY   0
#define STARTING  1
#define ALERTING  2
#define RUNNING   3
#define DAMAGE   4

/**
 * input/out pin definitions
 */
#define SENSOR_PIN 10
#define ON_OFF_PIN 11
#define LED_PIN 13

/**
 * The internal state
 */
int state = STANDBY;

unsigned int STARTING_DELAY = 1000;

unsigned long int alertingMillis;
unsigned long startingMillis;
unsigned long runningMillis;
unsigned int alertCount = 0;
unsigned int MAX_ALERT = 3;
boolean onOffPinOld = false;

Blinker blinker(LED_PIN,1000,1000);

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  blinker.begin();
  pinMode(LED_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  pinMode(ON_OFF_PIN, INPUT_PULLUP);

  Serial.println("System booting on STANDBY");
}

void loop()
{
  blinker.update();
  switch (state)
  {
  case STANDBY:
    handleStandByState();
    break;
  case STARTING:
    handleStartingState();
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
  boolean onOffPin = digitalRead (ON_OFF_PIN);
  if (!onOffPinOld && onOffPin)
  {
    Serial.println("DAMAGE -> STANDBY");
    state = STANDBY;
  }
  onOffPinOld = onOffPin;

  blinker.newDuration(5000, 500);
}

void handleRunningState()
{
  Wire.requestFrom(1,4);
  if(Wire.available())
  {
      Serial.println(Wire.readString());
  }
  boolean onOffPin = digitalRead (ON_OFF_PIN);
  if (!onOffPinOld && onOffPin)
  {
    Serial.println("RUNNING -> STANDBY");
    state = STANDBY;
  }
  else if (digitalRead(SENSOR_PIN) == HIGH)
  {
    if (alertCount >= MAX_ALERT)
    {
      Serial.println("RUNNING -> DEMAGE");
      state = DAMAGE;
    }
    else
    {
      Serial.println("RUNNING -> ALERTING");
      state = ALERTING;
      alertCount++;
      alertingMillis = millis();
    }

  }
  onOffPinOld = onOffPin;
  blinker.newDuration(2000, 500);
}

void handleStartingState()
{
  if (millis() - startingMillis >= STARTING_DELAY)
  {
    if (digitalRead(SENSOR_PIN) == HIGH)
    {
      Serial.println("SENSOR HIGH while system is STARTING");
      Serial.println("STARTING -> Damage");
      state = DAMAGE;
    }
    else
    {
      alertCount = 0;
      Serial.println("STARTING -> RUNNING");
      runningMillis = millis();
      state = RUNNING;
    }
  }
  blinker.newDuration(1000, 1000);
}

void handleStandByState()
{
  boolean onOffPin = digitalRead (ON_OFF_PIN);
  if (!onOffPinOld && onOffPin)
  {
    Serial.println("STANDBY -> STARTING");
    startingMillis = millis();
    state = STARTING;
  }
  onOffPinOld = onOffPin;
  blinker.newDuration(500, 2000);
}

void handleAlertingState()
{
   boolean onOffPin = digitalRead (ON_OFF_PIN);
  if (!onOffPinOld && onOffPin)
  {
    Serial.println("ALERTING -> STANDBY");
    state = STANDBY;
  }
  else if ((millis() - alertingMillis) >= 3000)
  {
    Serial.println("ALERTING -> RUNNING");
    state = RUNNING;
  }
  onOffPinOld = onOffPin;
  blinker.newDuration(500, 500);
}
