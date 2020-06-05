#include <Arduino.h>
#include "PWM.h"
#include "UI.h"

// Processor Frequency
int32_t clkFreq = 16000000;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Stores the GEN_PIN value for corresponding speedsetting
const int SPEED_VAL[] =
    {
        220, 280, 340, 415, 475, 506,
        506,
        506, 530, 600, 680, 740, 820};

// Set to 6 as this is Speed zero in the arrays
int Target_Speed = 6;

// Pin Definitions
const int PROGMEM pin_PWM = 10;
const int PROGMEM pin_PWM2 = 9;
const int PROGMEM GEN_PIN = A0;

// Defaults
const int PROGMEM defaultDuty = 50;
const int32_t PROGMEM defaultFreq = 40000; //frequency (in Hz)

// Globals
int DUTY = defaultDuty;
int32_t FREQ = defaultFreq;

// Gets value to set analogWrite function
int getAWrite(int32_t freq, int duty);
// Updates the duty cycle for pin_PWM
void setDutyCycle(int duty);

void PWMInit(void);

/*! @brief Gets value to set analogWrite function for request duty%
 *
 *  @param freq Frequency of the PWM signal
 *  @param duty Duty% of the PWM signal
 *
 *  @return required analogwrite int
 */
int getAWrite(int32_t freq, int duty)
{
  int32_t x = clkFreq / (2 * freq);

  return (x * duty) / 100;
}

/*! @brief Updates the Global Duty cycle and sets the PWM pins duty cycle
 *
 *  @param duty Duty% of the PWM signal
 *
 *  @return void
 */
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

/*! @brief Control scheme to keep the motor rotating at the desired speed setting
 *
 *  @param void
 *
 *  @return void
 */
void maintainSpeed(void)
{
  static unsigned long period = 300; // Used to increment on current time for waits
  static unsigned long waitTime = 0;

  static int avgSpeed;
  static int avgSamples = 10;
  int p_o = 35;  // Petrub and observe window
  int hyst = 20; // hysteresis window

  int currentSpeed = analogRead(GEN_PIN);

  // Calculate the moving average of the current speed
  avgSpeed -= avgSpeed / avgSamples;
  avgSpeed += currentSpeed / avgSamples;

  // Difference between current speed at corresponding speed setting val
  int speedDiff = avgSpeed - SPEED_VAL[Target_Speed];

  // Closed loop control
  if (Target_Speed == 6 || Emerg_Stop) //positive dir
  {
    setDutyCycle(50);
  }
  // Check if within hysteresis
  else if (abs(speedDiff) > hyst)
  {
    // Only update the duty cycle every "period" ms
    if (millis() > waitTime)
    {
      int dutyIncr = 0;
      // check if within peturb and observe
      if (abs(speedDiff) < p_o)
      {
        // Small steps
        dutyIncr = 1;
      }
      else
      {
        // Large steps
        dutyIncr = 3;
      }

      if (speedDiff < 0)
      {
        setDutyCycle(DUTY + dutyIncr);
      }
      else
      {
        setDutyCycle(DUTY - dutyIncr);
      }

      waitTime = millis() + period;
    }
  }
}

void PWMInit(void)
{
  //initialize all timers except for 0, to save time keeping functions
  InitTimersSafe();
  //sets the frequency for the specified pin
  bool success = SetPinFrequencySafe(pin_PWM, defaultFreq);
  success |= SetPinFrequencySafe(pin_PWM2, defaultFreq);
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
