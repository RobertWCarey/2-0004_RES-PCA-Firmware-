#include <Arduino.h>
#include "PWM.h"
#include "UI.h"

// Processor Frequency
int32_t clkFreq = 16000000;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// const int SPEED_VAL[] = 
// {
//   0,51,102,153,205,
//   256,307,358,409,460,
//   512,
//   563,614,665,716,767,
//   818,870,921,972,1023
// };

const int SPEED_DUTY[] = 
{
  80,75,70,65,60,55,50,45,40,35,30,25,20,
};

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
int getAWrite(int32_t freq,int duty);
// Updates the duty cycle for pin_PWM
void setDutyCycle(int duty);
// Updates the frequency for pin_PWM
void setFreq(int32_t freq);

void PWMInit(void);

// void maintainSpeed(void);



void maintainSpeed(void)
{
  int currentSpeed = analogRead(GEN_PIN);
  // int speedDiff = currentSpeed - SPEED_VAL[Target_Speed];
    // Closed loop control
  // if (Target_Speed == 10) //positive dir
  // {
  //   setDutyCycle(0);
  // }
  // else if ((speedDiff > 100) || (speedDiff < -100))
  // {
  //   if (speedDiff < 0)
  //   {
  //     setDutyCycle(DUTY+1);
  //   }
  //   else
  //   {
  //     setDutyCycle(DUTY-1);
  //   }
    
  // }

  // Open Loop Control
  setDutyCycle(SPEED_DUTY[Target_Speed]);
  // delay(1000);
}

int getAWrite(int32_t freq,int duty)
{
  int32_t x = clkFreq/(2*freq);

  return (x*duty)/100;
}

void setDutyCycle(int duty)
{
  // if ((duty < 10) && (duty != 0))
  // {
  //   duty = 10;
  // }
  // else if (duty > 90)
  // {
  //   duty = 90;
  // }
  DUTY = duty;
  analogWrite(pin_PWM,getAWrite(FREQ, duty));
  analogWrite(pin_PWM2,getAWrite(FREQ, duty));
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
  analogWrite(pin_PWM,getAWrite(freq, DUTY));
  analogWrite(pin_PWM2,getAWrite(freq, DUTY));
}

void PWMInit(void)
{
  //initialize all timers except for 0, to save time keeping functions
  InitTimersSafe();
  //sets the frequency for the specified pin
  bool success = SetPinFrequencySafe(pin_PWM, defaultFreq);
  if(success) {
    pinMode(pin_PWM,OUTPUT);
    pinMode(pin_PWM2,OUTPUT);
  }
  TCCR1A |= _BV(COM1A0);
  analogWrite(pin_PWM,getAWrite(defaultFreq,defaultDuty));
  analogWrite(pin_PWM2,getAWrite(defaultFreq,defaultDuty));
}

void setup()
{
  Serial.begin(115200);

  PWMInit();

  UI_init(&display);
}

unsigned long period = 500;
unsigned long waitTime = 0;

void loop()
{
  // if (DISPLAY_UPDATE)
  // {
    UI_updateDisplay(&display,Target_Speed);
  //   DISPLAY_UPDATE = false;
  // }

  UI_btnUpdate(&Target_Speed);

  // if (millis() > waitTime)
  // {
  // Serial.print("TSpeed: "); 
  // Serial.println(Target_Speed);
  // Serial.print("Speed Duty: "); 
  // Serial.println(SPEED_DUTY[Target_Speed]);


  maintainSpeed();

  
    
  //   waitTime = millis() + period;
  // }
  // delay(100);

}
