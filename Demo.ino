// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
	Name:       DemoOfXiangXun_DeFront103Project.ino
	Created:	2018/8/31
	Author:     DeFront\Zhengliang
*/

#include <Wire.h>
#include <math.h>

// MLX90393 I2C Address is 0x0Ch
// MLX90393-ABA-011-RE
#define Addr 0x0C

#define fanPositive  4
#define fanNegative  3
#define fanPWM       2
#define bengPWM      6
#define bengPositive 7
#define bengNegative 8

const int speedMax = 14;
const int speedMin = 2;
const int speedScope = (speedMax - speedMin);

float angles[4];
int   mag[3];
int   speed = (speedMax + speedMin) / 2;
int   magGlobal[] = { 0, 0 };

// The setup() function runs once each time the micro-controller starts
void setup()
{
	Wire.begin();
	Serial.begin(9600);
	pinMode(fanPositive, OUTPUT);
	pinMode(fanNegative, OUTPUT);
	pinMode(fanPWM, OUTPUT);
	pinMode(bengPositive, OUTPUT);
	pinMode(bengNegative, OUTPUT);
	pinMode(bengPWM, OUTPUT);

	digitalWrite(fanPositive, 0);
	digitalWrite(fanNegative, 0);
	digitalWrite(bengPositive, 0);
	digitalWrite(bengNegative, 0);
	analogWrite(bengPWM, 110);

	// Start I2C Transmisson
	Wire.beginTransmission(Addr);
	Wire.write(0x60);
	Wire.write(0x00);
	Wire.write(0x5C);
	Wire.write(0x00);
	Wire.endTransmission();

	Wire.requestFrom(Addr, 1);

	if (Wire.available() == 1)
	{
		unsigned int c = Wire.read();
	}

	// Start I2C Transmisson
	Wire.beginTransmission(Addr);
	Wire.write(0x60);
	Wire.write(0x02);
	Wire.write(0xB4);
	Wire.write(0x08);
	Wire.endTransmission();

	Wire.requestFrom(Addr, 1);

	if (Wire.available() == 1)
	{
		unsigned int c = Wire.read();
	}
	delay(300);

	// global magnetic effects
	for (int i = 1; i < 11; i++)
	{
		getMag();
		magGlobal[0] = mag[0] + magGlobal[0];
		magGlobal[1] = mag[1] + magGlobal[1];
	}
	magGlobal[0] = magGlobal[0] / 10;
	magGlobal[1] = magGlobal[1] / 10;
}

// get magnetic values
void getMag()
{
	unsigned int data[7];
	float angle;

	// Start I2C Transmission
	Wire.beginTransmission(Addr);
	Wire.write(0x3E);
	Wire.endTransmission();

	Wire.requestFrom(Addr, 1);

	if (Wire.available() == 1)
	{
		unsigned int c = Wire.read();
	}
	delay(100);

	Wire.beginTransmission(Addr);
	Wire.write(0x4E);
	Wire.endTransmission();

	Wire.requestFrom(Addr, 7);

	if (Wire.available() == 7)
	{
		data[0] = Wire.read();
		data[1] = Wire.read();
		data[2] = Wire.read();
		data[3] = Wire.read();
		data[4] = Wire.read();
		data[5] = Wire.read();
		data[6] = Wire.read();
	}

	// Convert the data
	int xMag = data[1] * 256 + data[2];
	int yMag = data[3] * 256 + data[4];
	int zMag = data[5] * 256 + data[6];

	mag[0] = xMag;
	mag[1] = yMag;
}

// get data from I2C
float getData()
{
	unsigned int data[7];
	float angle;

	// Start I2C Transmission
	Wire.beginTransmission(Addr);
	Wire.write(0x3E);
	Wire.endTransmission();

	Wire.requestFrom(Addr, 1);

	if (Wire.available() == 1)
	{
		unsigned int c = Wire.read();
	}
	delay(100);

	Wire.beginTransmission(Addr);
	Wire.write(0x4E);
	Wire.endTransmission();

	Wire.requestFrom(Addr, 7);

	if (Wire.available() == 7)
	{
		data[0] = Wire.read();
		data[1] = Wire.read();
		data[2] = Wire.read();
		data[3] = Wire.read();
		data[4] = Wire.read();
		data[5] = Wire.read();
		data[6] = Wire.read();
	}

	// Convert the data
	int xMag = data[1] * 256 + data[2] - magGlobal[0];
	int yMag = data[3] * 256 + data[4] - magGlobal[1];
	int zMag = data[5] * 256 + data[6];

	// get rad from (x,y)
	if (xMag >= 0)
	{
		angle = atan(yMag * 1.0000 / xMag);
	}
	else
	{
		if (yMag >= 0)
		{
			angle = PI - atan(yMag * 1.0000 / -xMag);
		}
		else
		{
			angle = -PI - atan(yMag * 1.0000 / -xMag);
		}
	}

	// reserve the x,y,z magnetic data
	mag[0] = xMag;
	mag[1] = yMag;
	mag[2] = abs(zMag);

	// at this time ,value angle¡Ê[-¦Ð, ¦Ð]
	return angle;
}

// Add the main program code into the continuous loop() function
void loop()
{
	analogWrite(fanPWM, speed);

	angles[1] = getData();

	// mark i as condition of air-pump need work
	int i = true;

	// control program of speed
	while ((abs(mag[0]) > 1000) || (abs(mag[1]) > 1000) || (mag[2] > 1000))
	{
		// turn on the air-pump before trun on the fan
		if (i == true)
		{
			digitalWrite(bengPositive, 1);
			digitalWrite(bengNegative, 0);
			delay(1000);
			digitalWrite(bengPositive, 0);
			digitalWrite(bengNegative, 0);
		}

		// orientation magnetic field and control the speed of fan
		digitalWrite(fanPositive, 1);
		digitalWrite(fanNegative, 0);
		delay(10);
		angles[0] = angles[1];
		angles[1] = getData();
		angles[2] = angles[1] - angles[0];
		angles[3] = abs(angles[2]);

		// Normalization and Change the angle difference to speed difference
		if (angles[3] < PI)
		{
			speed = speed - int(angles[2] / PI * speedScope);
		}
		else
		{
			int f;
			(angles[0] >= 0) ? (f = -1) : (f = 1);
			speed = speed + int((TWO_PI - angles[3]) / PI * f * speedScope);
		}

		// Control the scope of speed
		if (speed >= speedMax)
		{
			speed = speedMax;
		}
		else if (speed <= speedMin)
		{
			speed = speedMin;
		}

		analogWrite(fanPWM, speed);

		// avoid air-pump work again
		i = false;
	}

	// turn off the fan
	digitalWrite(fanPositive, 0);
	digitalWrite(fanNegative, 0);
}
