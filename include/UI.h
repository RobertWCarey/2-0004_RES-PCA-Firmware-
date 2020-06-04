

#ifndef _UI_H_
#define _UI_H_

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const String SW_VER = "0.1.1";

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)

const uint8_t PROGMEM BTN_UP = 8;
const uint8_t PROGMEM BTN_SELECT = 11;
const uint8_t PROGMEM BTN_DOWN = 12;
// Note D13 is not ideal as it has a res the needs to be desoldered on the nano to work
const uint8_t PROGMEM BTN_BACK = 13;

extern bool Emerg_Stop;

// bool DISPLAY_UPDATE = true;

typedef union {
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
void UI_updateDisplay(Adafruit_SSD1306 *display, int targetSpeed);

void UI_btnUpdate(int *targetSpeed);

// void UI_setSpeed(int *targetSpeed, int newSpeed);

#endif //_UI_H_