/* WheelUI.h
 * 
 * A UI library for encoder-driven menu-based UIs.
 *
 * Provides some features on top of UserInterface.h based on the
 * assumption that the device has at least one encoder wheel
 * and a hand-full of buttons.
 */

#ifndef WHEELUI_H
#define WHEELUI_H

#include "UserInterface.h"


/*
 * This is a temporary class that will probaly go away soon.
 */
class EncoderValueScreen : public Screen {
 public:
  
  EncoderValueScreen() {
    m_value = 0;
  };
  
  void draw(Adafruit_GFX &display) {
    display.println(String("Value: ") + m_value);
  }
  
  void handle_event(Event &event) {
    switch (event.source) {
    case 1:
      m_value += (signed char) event.data;
      break;
    case 2:
      m_value = 0;
      break; 
    };
  }
  
 private:
  int m_value;
};


struct MenuItem {
  const char *name;
  Screen &screen;
};

class Menu : public Screen {

 public:
  Menu(MenuItem items[]): m_items(items) {
    MenuItem *item = items;

    while (item->name) {
      m_n_items++;
      item++;
    }
  };
  
  void draw(Adafruit_GFX& display) {
    display.setTextSize(1);
    
    for (MenuItem *(item = m_pos= m_items; item->name; item++) {
      display.println(item);
    }
  };

  void handle_event(Event &event) {
    switch (event.source) {
    case 1:
      m_pos += min(m_n_items, max(0, (unsigned char) event.data));
      break;
    case 2:
      if (event.data = 1) {
	//
      }
    };
  }

 private:
  MenuItem *m_items;
  unsigned char m_pos;
  unsigned char m_n_items;
};


#endif
