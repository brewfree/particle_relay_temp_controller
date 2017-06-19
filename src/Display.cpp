#include <Particle.h>
#include "Display.h"

Display::Display() : _oled(-1) {
  _oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  _oled.clearDisplay();
}

void Display::update(Controller controller) {
  _oled.clearDisplay();
  _oled.setTextColor(WHITE);
  _oled.setCursor(0, 0);

  String name = controller.getName();
  name = name.substring(0, 9);
  String temp = controller.getCurrentTempFormatted();
  String status = controller.getStateFormatted();
  String target = controller.getTargetTempFormatted();
  String nextState = controller.getNextStateFormatted();

  _oled.setTextSize(2);
  _oled.println(name);

  _oled.setTextSize(1);
  _oled.println(status);
  _oled.print("CURRENT: ");
  _oled.println(temp);
  _oled.print("TARGET: ");
  _oled.println(target);
  _oled.println(nextState);

  _oled.display();
}
