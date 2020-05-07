#include <Arduino.h>
#include "PWM.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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
const int PROGMEM pin_PWM = 10;
const int PROGMEM pin_PWM2 = 9;
const int PROGMEM defaultDuty = 50;
const int32_t PROGMEM defaultFreq = 40000; //frequency (in Hz)

const uint8_t PROGMEM BTN_UP = 8;
const uint8_t PROGMEM BTN_SELECT = 11;
const uint8_t PROGMEM BTN_DOWN = 12;
// Note D13 is not ideal as it has a res the needs to be desoldered on the nano to work
const uint8_t PROGMEM BTN_BACK =13;
uint8_t BTNS[]={BTN_UP, BTN_SELECT,
                BTN_DOWN, BTN_BACK};

const uint8_t PROGMEM MAX_MAINSTATE = 2;
uint8_t MAX_SUBSTATE[MAX_MAINSTATE]={2,2};


// Globals
int DUTY = defaultDuty;
int32_t FREQ = defaultFreq;
String Command;


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

void UI_mainMenuDisplay(const String &buf);

void drawCentreString(const String &buf, int x, int y);

void UI_btnUpdate(uint16union_t *displayState);

void UI_updateValue(uint8_t btn, uint16union_t *displayState);

void UI_updateValue(uint8_t btn, uint16union_t *displayState)
{
  uint8_t *mainState = &displayState->s.Hi;
  uint8_t *subState = &displayState->s.Lo;
  int increment = 0;

  if (*subState == 1)
  {
    switch (btn)
    {
    case BTN_UP:
      // not sure why *mainState--; didnt update val
      *mainState = *mainState-1;
      if (*mainState  == 0)
      {
        *mainState = MAX_MAINSTATE;
      }
      break;
    case BTN_DOWN:
      *mainState = *mainState+1;
      if (*mainState  > MAX_MAINSTATE)
      {
        *mainState = 1;
      }
      break;
    case BTN_SELECT:
      *subState = *subState+1;
      if (*subState > MAX_SUBSTATE[*mainState-1])
      {
        *subState = MAX_SUBSTATE[*mainState-1];
      }
      break;
    default:
      break;
    }
    return;
  }

  
  switch (btn)
  {
    case BTN_BACK:
      *subState = *subState-1;
      if (*subState == 0)
      {
        *subState = 1;
      }
      return;
      break;
    case BTN_UP:
      increment = 1;
      break;
    case BTN_DOWN:
      increment = -1;
      break;
    default:
      return;
      break;
  }
  switch (*mainState)
  {
    // Frequency
    case 1:
      switch (*subState)
      {
        // Adjust Frequency
        case 2:
          if (increment > 0)
          {
            setFreq(FREQ + 1000);
          }
          else if (increment < 0)
          {
            setFreq(FREQ - 1000);
          }
          break;
        default:
          break;
      }
      break;
    case 2:
      switch (*subState)
      {
        case 2:
          if (increment > 0)
          {
            setDutyCycle(DUTY + 5);
          }
          else if (increment < 0)
          {
            setDutyCycle(DUTY - 5);
          }
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void UI_btnUpdate(uint16union_t *displayState)
{
  static unsigned long period = 250;
  static unsigned long waitTime = 0;
  
  for (int i = 0; i < 4; i++)
  {
    int btnState = digitalRead(BTNS[i]);
    if (btnState == LOW && (millis() > waitTime) )
    {
      UI_updateValue(BTNS[i], displayState);
      waitTime = millis() + period;
    }  
  }
}

void UI_updateDisplay(uint16union_t displayState)
{
  uint8_t mainState = displayState.s.Hi;
  uint8_t subState = displayState.s.Lo;
  // 0 is an invalid mainState
  String value;

  switch (mainState)
  {
    // Frequency
    case 1:
      switch (subState)
      {
        case 1:
          UI_mainMenuDisplay(F("Frequency"));
          break;
        // Adjust Frequency
        case 2:
          value = String(FREQ);
          UI_mainMenuDisplay(value+"Hz");
          break;
        default:
          break;
      }
      break;
    case 2:
      switch (subState)
      {
        case 1:
          UI_mainMenuDisplay(F("Duty%"));
          break;
        // Adjust Duty Cycle
        case 2:
          value = String(DUTY);
          UI_mainMenuDisplay(value+"%");
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void drawCentreString(const String &buf, int x, int y)
{
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h); //calc width of new string
    display.setCursor(x - (w / 2), y - (h / 2));
    display.print(buf);
}

void UI_mainMenuDisplay(const String &buf)
{
  display.clearDisplay();

  uint8_t dispQuartH = display.height()/4;
  uint8_t dispHalfW = display.width()/2;
  uint8_t dispHalfH = display.height()/2;

  // Top Triangle
  display.fillTriangle(
    dispHalfW  , dispQuartH-10,
    dispHalfW-5, dispQuartH,
    dispHalfW+5, dispQuartH, SSD1306_WHITE);

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  drawCentreString(buf,dispHalfW,dispHalfH);

  //Bottom Triangle
  display.fillTriangle(
    dispHalfW  , (dispQuartH*3)+10,
    dispHalfW-5, (dispQuartH*3),
    dispHalfW+5, (dispQuartH*3), SSD1306_WHITE);

  display.display();

}

int getAWrite(int32_t freq,int duty)
{
  int32_t x = clkFreq/(2*freq);

  return (x*duty)/100;
}

void setDutyCycle(int duty)
{
  if (duty < 10)
  {
    duty = 10;
  }
  else if (duty > 90)
  {
    duty = 90;
  }
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

void UI_init(void)
{
  //Config Buttons
  pinMode(BTN_UP,INPUT);
  pinMode(BTN_SELECT,INPUT);
  pinMode(BTN_DOWN,INPUT);
  pinMode(BTN_BACK,INPUT);
  
  // Config Display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Display Current Firmware Version
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  drawCentreString(F("0.0.1"), display.width()/2, display.height()/2);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);
}

void setup()
{
  Serial.begin(9600);

  PWMInit();

  UI_init();

  DisplayState.s.Hi = 1;
  DisplayState.s.Lo = 1;
}



void loop()
{
  UI_updateDisplay(DisplayState);

  UI_btnUpdate(&DisplayState);
}
