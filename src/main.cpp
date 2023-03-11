#include <Arduino.h>
#include <NewPing.h>
#include <L298N.h>
#include <Servo.h>

#define LIGHT_FRONT A7
#define LIGHT_RIGHT A6
#define LIGHT_LEFT A5
#define LIGHT_TRESHOLD 130
#define LIGHT_PERCENT_DIFF 1.1

enum State
{
  SearchingDirection,
  Moving,
  Shooting,
  EmergencyStop
};

State state = SearchingDirection;

Servo seringe;

NewPing sonar(11, 12, 400);
L298N motorLeft(4, 5);
L298N motorRight(3, 2);

void stop()
{
  motorLeft.stop();
  motorRight.stop();
}

void forward()
{
  motorLeft.forward();
  motorRight.forward();
}

void turnRight()
{
  motorLeft.forward();
  motorRight.backward();
}

void turnLeft()
{
  motorLeft.backward();
  motorRight.forward();
}

void shootWater()
{
  seringe.write(-180);
}

void stopWater()
{
  seringe.write(0);
}

float getDistance()
{
  float duration = sonar.ping_median(10);
  float distance = (duration / 2) * 0.0343;
  if (distance <= 0.5)
    distance = 400;
  return distance;
}

unsigned int getLightLevel(int pin)
{
  return analogRead(pin);
}

void setup()
{
  Serial.begin(9600);
  pinMode(A7, INPUT);
  pinMode(A6, INPUT);
  pinMode(A5, INPUT);
}

void loop()
{
  switch (state)
  {
  case SearchingDirection:
    stop();
    if (getLightLevel(LIGHT_FRONT) >= getLightLevel(LIGHT_LEFT) && getLightLevel(LIGHT_FRONT) >= getLightLevel(LIGHT_RIGHT))
    {
      state = Moving;
    }
    else if (getLightLevel(LIGHT_LEFT) >= LIGHT_PERCENT_DIFF * getLightLevel(LIGHT_RIGHT))
    {
      turnLeft();
      delay(50);
    }
    else if (getLightLevel(LIGHT_RIGHT) >= LIGHT_PERCENT_DIFF * getLightLevel(LIGHT_LEFT))
    {
      turnRight();
      delay(50);
    }
    break;
  case Moving:
    forward();
    if (getDistance() <= 55 && (getLightLevel(LIGHT_FRONT) >= LIGHT_TRESHOLD || getLightLevel(LIGHT_RIGHT) >= LIGHT_TRESHOLD || getLightLevel(LIGHT_LEFT) >= LIGHT_TRESHOLD))
    {
      state = Shooting;
    }
    else if (getDistance() <= 15)
    {
      state = SearchingDirection;
    }
    delay(50);
    break;
  case EmergencyStop:
    stop();
    delay(5000);
    break;
  case Shooting:
    stop();
    while (getLightLevel(LIGHT_LEFT) >= LIGHT_PERCENT_DIFF * getLightLevel(LIGHT_RIGHT))
    {
      turnLeft();
    }
    while (getLightLevel(LIGHT_RIGHT) > LIGHT_PERCENT_DIFF * getLightLevel(LIGHT_LEFT))
    {
      turnRight();
    }
    shootWater();
    delay(5000);
    stopWater();
    state == EmergencyStop;
    break;
  default:
    break;
  }

  // Serial.print("FrontL ");
  // Serial.println(getLightLevel(LIGHT_FRONT));
  // Serial.print("LeftL ");
  // Serial.println(getLightLevel(LIGHT_LEFT));
  // Serial.print("RightL ");
  // Serial.println(getLightLevel(LIGHT_RIGHT));
  // Serial.print("Dist ");
  // Serial.println(getDistance());
  // Serial.println("======");
  // delay(3000);
}