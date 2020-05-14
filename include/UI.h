

#ifndef _UI_H_
#define _UI_H_

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)

const uint8_t PROGMEM BTN_UP = 8;
const uint8_t PROGMEM BTN_SELECT = 11;
const uint8_t PROGMEM BTN_DOWN = 12;
// Note D13 is not ideal as it has a res the needs to be desoldered on the nano to work
const uint8_t PROGMEM BTN_BACK =13;

// bool DISPLAY_UPDATE = true;


// const uint8_t PROGMEM MAX_MAINSTATE = 2;
// uint8_t MAX_SUBSTATE[MAX_MAINSTATE]={2,2};

// const String PROGMEM SPEED_DISP[] = 
// {
//   "-10","-9","-8","-7","-6",
//   "-5","-4","-3","-2","-1",
//   "0",
//   "1","2","3","4","5",
//   "6","7","8","9","10"
// };

typedef union
{
  uint16_t l;
  struct
  {
    uint8_t Lo;
    uint8_t Hi;
  } s;
} uint16union_t;

// Initilise User interface
void UI_init(Adafruit_SSD1306 *display);

// Update OLED Display
// void UI_updateDisplay(Adafruit_SSD1306 *display, int targetSpeed);



// void UI_btnUpdate(uint16union_t *displayState);

// void UI_updateValue(uint8_t btn, uint16union_t *displayState);

// void UI_setSpeed(int speed);

#endif //_UI_H_