#include <Arduino.h>
#include "PWM.h"
#include "UI.h"

// Processor Frequency
int32_t clkFreq = 16000000;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int SPEED_VAL[] =
    {
        220, 280, 340, 415, 475, 506, 506,
        506, 530, 600, 680, 740, 820};

const int SPEED_DUTY[] =
    {
        80, 75, 70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20};

// Set to 10 as the is the zero point in the array
int Target_Speed = 6;

// Constants
const int PROGMEM pin_PWM = 10;
const int PROGMEM pin_PWM2 = 9;
const int PROGMEM defaultDuty = 50;
const int32_t PROGMEM defaultFreq = 40000; //frequency (in Hz)

const int PROGMEM GEN_PIN = A0;

// Globals
int DUTY = defaultDuty;
int32_t FREQ = defaultFreq;
String Command;

// Gets value to set analogWrite function
int getAWrite(int32_t freq, int duty);
// Updates the duty cycle for pin_PWM
void setDutyCycle(int duty);
// Updates the frequency for pin_PWM
void setFreq(int32_t freq);

void PWMInit(void);

void maintainSpeed(void);

void maintainSpeed(void)
{
  static unsigned long period = 300;
  static unsigned long waitTime = 0;

  static int avgSpeed;
  static int avgSamples = 10;
  int p_o = 35;
  int hyst = 20;

  int currentSpeed = analogRead(GEN_PIN);

  // Calculate the moving average of the current speed
  avgSpeed -= avgSpeed / avgSamples;
  avgSpeed += currentSpeed / avgSamples;

  int speedDiff = avgSpeed - SPEED_VAL[Target_Speed];

  Serial.print("Speed Diff: ");
  Serial.println(speedDiff);

  // Closed loop control
  if (Target_Speed == 6 || Emerg_Stop) //positive dir
  {
    setDutyCycle(50);
  }
  // Check if within hysteresis
  else if (abs(speedDiff) > hyst)
  {
    if (millis() > waitTime)
    {
      // check if within peturb and observe
      if (abs(speedDiff) < p_o)
      {
        if (speedDiff < 0)
        {
          setDutyCycle(DUTY + 1);
        }
        else
        {
          setDutyCycle(DUTY - 1);
        }
      }
      else
      {
        if (speedDiff < 0)
        {
          setDutyCycle(DUTY + 3);
        }
        else
        {
          setDutyCycle(DUTY - 3);
        }
      }
      waitTime = millis() + period;
    }
  }
}

int getAWrite(int32_t freq, int duty)
{
  int32_t x = clkFreq / (2 * freq);

  return (x * duty) / 100;
}

void setDutyCycle(int duty)
{
  if ((duty < 10) && (duty != 0))
  {
    duty = 20;
  }
  else if (duty > 90)
  {
    duty = 80;
  }
  DUTY = duty;
  analogWrite(pin_PWM, getAWrite(FREQ, duty));
  analogWrite(pin_PWM2, getAWrite(FREQ, duty));
}

void setFreq(int32_t freq)
{
  if (freq < 5000)
  {
    freq = 5000;
  }
  else if (freq > 100000)
  {
    freq = 1000000;
  }
  FREQ = freq;
  SetPinFrequencySafe(pin_PWM, freq);
  analogWrite(pin_PWM, getAWrite(freq, DUTY));
  analogWrite(pin_PWM2, getAWrite(freq, DUTY));
}

void PWMInit(void)
{
  //initialize all timers except for 0, to save time keeping functions
  InitTimersSafe();
  //sets the frequency for the specified pin
  bool success = SetPinFrequencySafe(pin_PWM, defaultFreq);
  if (success)
  {
    pinMode(pin_PWM, OUTPUT);
    pinMode(pin_PWM2, OUTPUT);
  }
  TCCR1A |= _BV(COM1A0);
  analogWrite(pin_PWM, getAWrite(defaultFreq, defaultDuty));
  analogWrite(pin_PWM2, getAWrite(defaultFreq, defaultDuty));
}

void setup()
{
  Serial.begin(115200);

  PWMInit();

  UI_init(&display);
}

void loop()
{
  UI_updateDisplay(&display, Target_Speed);

  UI_btnUpdate(&Target_Speed);

  maintainSpeed();
}
