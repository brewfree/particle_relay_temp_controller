#ifndef Display_h
#define Display_h

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Controller.h"

  class Display {
      public:
          Display();
          void update(Controller controller);
      private:
        Adafruit_SSD1306 _oled;
  };

#endif
