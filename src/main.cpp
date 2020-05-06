#include <Arduino.h>
#include "PWM.h"

typedef union
{
  uint16_t l;
  struct
  {
    uint8_t Lo;
    uint8_t Hi;
  } s;
} uint16union_t;

// Processor Frequency
int32_t clkFreq = 16000000;

// Constants
const int pin_PWM = 10;
const int pin_PWM2 = 9;
const int defaultDuty = 50;
const int32_t defaultFreq = 35714; //frequency (in Hz)

// Globals
int Duty = defaultDuty;
int32_t Freq = defaultFreq;
String Command;



// Gets value to set analogWrite function
int getAWrite(int32_t freq,int duty);
// Updates the duty cycle for pin_PWM
void setDutyCycle(int duty);
// Updates the frequency for pin_PWM
void setFreq(int32_t freq);
// Parses any serial commands recieved
bool parseCommand(String com);

void PWMInit(void);

void UI_init(void);

uint16union_t DisplayState;
void UI_updateDisplay(uint16union_t displayState);

void UI_mainMenu(void);

void UI_updateDisplay(uint16union_t displayState)
{
  uint8_t mainState = displayState.s.Hi;
  uint8_t subState = displayState.s.Lo;
  switch (mainState)
  {
    // Frequency
    case 1:
      switch (subState)
      {
        case 0:
          
          break;
        // Adjust Frequency
        case 1:
          break;
        default:
          break;
      }
      break;
    case 2:
      break;
    default:
      break;
  }
}

void UI_mainMenu(void)
{

}

int getAWrite(int32_t freq,int duty)
{
  int32_t x = clkFreq/(2*freq);

  return (x*duty)/100;
}

void setDutyCycle(int duty)
{
  Duty = duty;
  analogWrite(pin_PWM,getAWrite(Freq, duty));
  analogWrite(pin_PWM2,getAWrite(Freq, duty));
}

void setFreq(int32_t freq)
{
  Freq = freq;
  SetPinFrequencySafe(pin_PWM, freq);
  analogWrite(pin_PWM,getAWrite(freq, Duty));
  analogWrite(pin_PWM2,getAWrite(freq, Duty));
}

bool parseCommand(String com)
{
  String part1,part2;
  int32_t val;

  part1 = com.substring(0, com.indexOf(' '));
  part2 = com.substring(com.indexOf(' ')+1);

  // Duty Cycle
  if (part1.equalsIgnoreCase("D"))
  {
    val = part2.toInt();
    if (val > 100 || val < 0)
    {
      return false;
    }
    else
    {
      setDutyCycle(val);
      Serial.print("Duty Cycle: ");
      Serial.print(Duty);
      Serial.println("%");
      return true;
    }
  }
  // Frequency
  else if (part1.equalsIgnoreCase("F"))
  {
    val = part2.toInt();
    if (val < 0)
    {
      return false;
    }
    else
    {
      setFreq(val);
      Serial.print("Frequency: ");
      Serial.print(Freq);
      Serial.println("Hz");
      return true;
    }
  }

  return false;
}

void PWMInit(void)
{
  //initialize all timers except for 0, to save time keeping functions
  InitTimersSafe();
  //sets the frequency for the specified pin
  bool success = SetPinFrequencySafe(pin_PWM, defaultFreq);
   if(success) {
      pinMode(13, OUTPUT);
      digitalWrite(13, HIGH);
      pinMode(pin_PWM,OUTPUT);
      pinMode(pin_PWM2,OUTPUT);
   }
   TCCR1A |= _BV(COM1A0);
  analogWrite(pin_PWM,getAWrite(defaultFreq,defaultDuty));
  analogWrite(pin_PWM2,getAWrite(defaultFreq,defaultDuty));
}

void UI_init(void)
{

}

void setup()
{
  Serial.begin(9600);

  PWMInit();
}

void loop()
{
  if (Serial.available())
  {  
   char c = Serial.read();
   if (c == '\n')
   {
     if (!parseCommand(Command))
     {
       Serial.println("Invalid Input");
       Serial.println("");
       Serial.println("Valid Inputs:");
       Serial.println("'D 50', Duty Cycle Update");
       Serial.println("'F 10000', Frequency Update");
     }
     Command = "";
   }
   else
   {
     Command += c;
   }
  }
}
