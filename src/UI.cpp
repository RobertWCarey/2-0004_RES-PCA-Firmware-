

#include "UI.h"

uint8_t BTNS[]={BTN_UP, BTN_SELECT,
                BTN_DOWN, BTN_BACK};

uint16union_t Display_State;
const uint8_t PROGMEM MAX_MAINSTATE = 1;
uint8_t MAX_SUBSTATE[MAX_MAINSTATE]={2};

const String PROGMEM SPEED_DISP[] = 
{
  "-6",
  "-5","-4","-3","-2","-1",
  "0",
  "1","2","3","4","5",
  "6"
};

bool Emerg_Stop = false;

void drawCentreString(const String &buf, int x, int y, Adafruit_SSD1306 *display)
{
    int16_t x1, y1;
    uint16_t w, h;
    display->getTextBounds(buf, 0, 0, &x1, &y1, &w, &h); //calc width of new string
    display->setCursor(x - (w / 2), y - (h / 2));
    display->print(buf);
}

void mainMenuDisplay(const String &buf, Adafruit_SSD1306 *display)
{
  display->clearDisplay();

  uint8_t dispQuartH = display->height()/4;
  uint8_t dispHalfW = display->width()/2;
  uint8_t dispHalfH = display->height()/2;

  // Top Triangle
  display->fillTriangle(
    dispHalfW  , dispQuartH-10,
    dispHalfW-5, dispQuartH,
    dispHalfW+5, dispQuartH, SSD1306_WHITE);

  display->setTextSize(2);
  display->setTextColor(SSD1306_WHITE);
  drawCentreString(buf, dispHalfW, dispHalfH, display);

  //Bottom Triangle
  display->fillTriangle(
    dispHalfW  , (dispQuartH*3)+10,
    dispHalfW-5, (dispQuartH*3),
    dispHalfW+5, (dispQuartH*3), SSD1306_WHITE);

  display->display();

}

void UI_init(Adafruit_SSD1306 *display)
{
  //Config Buttons
  pinMode(BTN_UP,INPUT);
  pinMode(BTN_SELECT,INPUT);
  pinMode(BTN_DOWN,INPUT);
  pinMode(BTN_BACK,INPUT);
  
  // Config Display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display->begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  Serial.println(F("SSD1306 allocation succes"));

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display->display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display->clearDisplay();

  // Display Current Firmware Version
  display->setTextSize(2);
  display->setTextColor(SSD1306_WHITE);
  drawCentreString(SW_VER, display->width()/2, display->height()/2, display);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display->display();
  delay(2000);

  Display_State.s.Hi = 1;
  Display_State.s.Lo = 1;
}

void UI_updateDisplay(Adafruit_SSD1306 *display,int targetSpeed)
{
  uint8_t mainState = Display_State.s.Hi;
  uint8_t subState = Display_State.s.Lo;
  // 0 is an invalid mainState
  String value;

  switch (mainState)
  {
    // Speed
    case 1:
      switch (subState)
      {
        case 1:
          mainMenuDisplay(F("Speed"), display);
          break;
        // Adjust Speed
        case 2:
          // Serial.print("Disp: "); Serial.println(SPEED_DISP[targetSpeed]);
          mainMenuDisplay(SPEED_DISP[targetSpeed], display);
          break;
        default:
          break;
      }
      break;
    // Emergency Stop
    case 2:
      switch (subState)
      {
        case 1:
          mainMenuDisplay(F("Emerg Stop"), display);
          break;
        // Enable Emergency stop
        case 2:
          if (Emerg_Stop)
          {
            mainMenuDisplay(F("ENABLED"), display);
          }
          else
          {
            mainMenuDisplay(F("DISABLED"), display);
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

void UI_setSpeed(int *targetSpeed, int newSpeed)
{
  Serial.print("newSpeed (setSpeed): "); Serial.println(newSpeed);
  if ((newSpeed >= 0) && (newSpeed <=12))
  {
    *targetSpeed = newSpeed;

    // setDutyCycle(SPEED_DUTY[*targetSpeed]);

    // DISPLAY_UPDATE = true;
  }

}

void updateValue(uint8_t btn, int *targetSpeed)
{
  uint8_t *mainState = &Display_State.s.Hi;
  uint8_t *subState = &Display_State.s.Lo;
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
    // Speed
    case 1:
      switch (*subState)
      {
        // Adjust Speed
        case 2:
          if (increment > 0)
          {
            UI_setSpeed(targetSpeed, *targetSpeed+1);
          }
          else if (increment < 0)
          {
            UI_setSpeed(targetSpeed, *targetSpeed-1);
          }
          break;
        default:
          break;
      }
      break;
    // Emergency Stop
    case 2:
      switch (*subState)
      {
        case 2:
          if (BTN_SELECT)
          {
            Emerg_Stop = !Emerg_Stop;
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

void UI_btnUpdate(int *targetSpeed)
{
  static unsigned long period = 250;
  static unsigned long waitTime = 0;
  
  for (int i = 0; i < 4; i++)
  {
    int btnState = digitalRead(BTNS[i]);
    if (btnState == LOW && (millis() > waitTime) )
    {
      updateValue(BTNS[i], targetSpeed);
      // DISPLAY_UPDATE = true;
      waitTime = millis() + period;
    }  
  }
}




