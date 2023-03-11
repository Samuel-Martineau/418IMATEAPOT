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

int currentStateStart = 0;

enum CheckDirectionStatus
{
  Right,
  Left,
  Ok
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

CheckDirectionStatus checkDirection()
{
  float minDistLeft = 0;
  float minDistRight = 0;
  int start = millis();
  turnLeft();
  while (millis() - start <= 1500)
  {
    minDistLeft = min(getDistance(), minDistLeft);
  }
  start = millis();
  turnRight();
  while (millis() - start <= 1500)
  {
    minDistLeft = min(getDistance(), minDistLeft);
  }
  start = millis();
  while (millis() - start <= 1500)
  {
    minDistRight = min(getDistance(), minDistRight);
  }
  turnLeft();
  start = millis();
  while (millis() - start <= 1500)
  {
    minDistRight = min(getDistance(), minDistRight);
  }
  stop();

  if (minDistLeft >= 20 && minDistRight >= 20)
  {
    return Ok;
  }
  else if (minDistRight <= minDistLeft)
  {
    return Left;
  }
  else
  {
    return Right;
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(A7, INPUT);
  pinMode(A6, INPUT);
  pinMode(A5, INPUT);

  delay(3000);
}

void loop()
{
  switch (state)
  {
  case SearchingDirection:
    stop();
    if (getLightLevel(LIGHT_FRONT) >= getLightLevel(LIGHT_LEFT) && getLightLevel(LIGHT_FRONT) >= getLightLevel(LIGHT_RIGHT))
    {

      CheckDirectionStatus directionStatus = checkDirection();
      while (directionStatus != Ok)
      {
        if (directionStatus == Right)
        {
          turnRight();
          delay(2000);
          stop();
        }
        else if (directionStatus == Left)
        {
          turnLeft();
          delay(2000);
          stop();
        }

        directionStatus = checkDirection();
      }

      state = Moving;
      currentStateStart = millis();
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
      state = EmergencyStop;
      currentStateStart = millis();
    }
    else if (getDistance() <= 15)
    {
      state = EmergencyStop;
      currentStateStart = millis();
    }
    else if (millis() - currentStateStart >= 3000)
    {
      state = SearchingDirection;
      currentStateStart = millis();
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
    state = EmergencyStop;
    currentStateStart = millis();
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