/*! @file
 *
 *  @brief Functions for interacting with the User interface
 *
 *  @author Robert Carey
 *  @date 2020-05-15
 */

#ifndef _UI_H_
#define _UI_H_

// Using Adafruit libraries to do most of the heavy lifting for 0.96" OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Software version, used to display at startup
const String SW_VER = "0.1.1";

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)

// UI buttons pin definitions
const uint8_t PROGMEM BTN_UP = 8;
const uint8_t PROGMEM BTN_SELECT = 11;
const uint8_t PROGMEM BTN_DOWN = 12;
// Note D13 is not ideal as it has a res the needs to be desoldered on the nano to work
const uint8_t PROGMEM BTN_BACK = 13;

// Parameter to indicate if emergency stop has been enabled or not
extern bool Emerg_Stop;

// Union mainly used for the display state storage
typedef union {
  uint16_t l;
  struct
  {
    uint8_t Lo;
    uint8_t Hi;
  } s;
} uint16union_t;

/*! @brief Initilises User Interface
 *
 *  Sets up btn pins and initilises the display 
 * 
 *  @param display pointer to the display handle
 *
 *  @return  void
 */
void UI_init(Adafruit_SSD1306 *display);

/*! @brief Updates display with current status
 * 
 *  @param display      pointer to the display handle
 *  @param targetSpeed  value of the current target speed setting
 *
 *  @return  void
 * 
 *  @note This should be called in a constant loop
 *        i.e. execute every 10ms, the faster the better
 */
void UI_updateDisplay(Adafruit_SSD1306 *display, int targetSpeed);

/*! @brief Checks the current btn state and updates the internal variables
 * 
 *  @param targetSpeed  pointer to the value of the current target speed setting
 *
 *  @return  void
 * 
 *  @note This should be called in a constant loop
 *        i.e. execute every 10ms, the faster the better
 */
void UI_btnUpdate(int *targetSpeed);

#endif //_UI_H_