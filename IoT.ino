#include <Arduino.h>
#include <Arduino_LSM9DS1.h>
#include <Arduino_HTS221.h>
#include <Arduino_LPS22HB.h>
#include <Arduino_APDS9960.h>

#define RED 22
#define BLUE 24
#define GREEN 23

#define READ_RATE 10000
#define RBG_RATE 1000

#define LONG_RATE 500
#define MEDIUM_RATE 250
#define SHORT_RATE 125

unsigned long _lastRead;
unsigned long _lastUpdate;
unsigned long _lastProx;

bool _ledState;

uint8_t _rbgState;

void setup() {
  Serial.begin(9600);
  while (!Serial)
  {
    delay(1000);
  }
  _lastRead = 0;
  _lastUpdate = 0;
  _lastProx = 0;
  _rbgState = 0;
  _ledState = false;

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);
  digitalWrite(LED_BUILTIN, LOW);

  if (!HTS.begin())
    Serial.println("Failed initializing HTS");
  if (!BARO.begin())
    Serial.println("Failed initializing BARO");
  if (!APDS.begin())
    Serial.println("Failed initializing APDS");
  if (!IMU.begin())
    Serial.println("Failed initializing IMU");
}

void loop()
{
  updateRBG();
  checkProximity();

  unsigned long currentTime = millis();
  if (currentTime - _lastRead >= READ_RATE)
  {
    float tempF = HTS.readTemperature(FAHRENHEIT);
    float rh = HTS.readHumidity();
    float pressure = BARO.readPressure(PSI);
    Serial.print(tempF);
    Serial.print("F, ");
    Serial.print(rh);
    Serial.println("% RH");
    Serial.print(pressure);
    Serial.println(" PSI");

    Serial.print("Prox: ");
    if (APDS.readProximity())
    {
      int prox = APDS.readProximity();
      Serial.print(prox);
      Serial.println();
    }
    else
    {
      Serial.println("No data");
    }

    Serial.print("Mag: ");
    if (IMU.magneticFieldAvailable())
    {
      float x, y, z;
      IMU.readMagneticField(x, y, z);
      Serial.print(x);
      Serial.print('\t');
      Serial.print(y);
      Serial.print('\t');
      Serial.println(z);
    }
    else
    {
      Serial.println("No data");
    }

    _lastRead = currentTime;
  }
}

void updateRBG()
{
  unsigned long currentTime = millis();
  if (currentTime - _lastUpdate >= RBG_RATE) {
    switch (_rbgState % 6) {
    case 0:
      digitalWrite(RED, HIGH);
      break;
    case 1:
      digitalWrite(GREEN, LOW);
      break;
    case 2:
      digitalWrite(BLUE, HIGH);
      break;
    case 3:
      digitalWrite(RED, LOW);
      break;
    case 4:
      digitalWrite(GREEN, HIGH);
      break;
    case 5:
      digitalWrite(BLUE, LOW);
      break;
    }

    _rbgState++;

    _lastUpdate = currentTime;
  }
}

void checkProximity()
{
  if (APDS.readProximity())
  {
    unsigned long currentTime = millis();

    int prox = APDS.readProximity();
    if (prox == -1)
    {
      _ledState = true;
    }
    else if (prox > 100)
    {
      if (currentTime - _lastProx >= LONG_RATE) {
        _ledState = !_ledState;
        _lastProx = currentTime;
      }
    }
    else if (prox > 50)
    {
      if (currentTime - _lastProx >= MEDIUM_RATE) {
        _ledState = !_ledState;
        _lastProx = currentTime;
      }
    }
    else
    {
      if (currentTime - _lastProx >= SHORT_RATE) {
        _ledState = !_ledState;
        _lastProx = currentTime;
      }
    }

    digitalWrite(LED_BUILTIN, _ledState);
  }
}