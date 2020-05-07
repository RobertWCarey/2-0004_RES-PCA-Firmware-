#include <Arduino.h>
#include "PWM.h"
#include <SPI.h>
#include <Wire.h>
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
const int pin_PWM = 10;
const int pin_PWM2 = 9;
const int defaultDuty = 50;
const int32_t defaultFreq = 35714; //frequency (in Hz)

// Globals
int DUTY = defaultDuty;
int32_t FREQ = defaultFreq;
String Command;


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

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
        case 0:
          UI_mainMenuDisplay(F("Frequency"));
          break;
        // Adjust Frequency
        case 1:
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
        case 0:
          UI_mainMenuDisplay(F("Duty%"));
          break;
        // Adjust Duty Cycle
        case 1:
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
  DUTY = duty;
  analogWrite(pin_PWM,getAWrite(FREQ, duty));
  analogWrite(pin_PWM2,getAWrite(FREQ, duty));
}

void setFreq(int32_t freq)
{
  FREQ = freq;
  SetPinFrequencySafe(pin_PWM, freq);
  analogWrite(pin_PWM,getAWrite(freq, DUTY));
  analogWrite(pin_PWM2,getAWrite(freq, DUTY));
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
      Serial.print(DUTY);
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
      Serial.print(FREQ);
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
  drawCentreString(F("0.0.0"), display.width()/2, display.height()/2);

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
}

void loop()
{
  DisplayState.s.Hi = 1;
  DisplayState.s.Lo = 1;
  static uint16_t prevDispState = 0;
  if (prevDispState != DisplayState.l)
  {
    UI_updateDisplay(DisplayState);
    prevDispState = DisplayState.l;
  }
  

  // if (Serial.available())
  // {  
  //  char c = Serial.read();
  //  if (c == '\n')
  //  {
  //    if (!parseCommand(Command))
  //    {
  //      Serial.println("Invalid Input");
  //      Serial.println("");
  //      Serial.println("Valid Inputs:");
  //      Serial.println("'D 50', Duty Cycle Update");
  //      Serial.println("'F 10000', Frequency Update");
  //    }
  //    Command = "";
  //  }
  //  else
  //  {
  //    Command += c;
  //  }
  // }
}
