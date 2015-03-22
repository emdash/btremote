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

#include <AdaEncoder.h>
#include "UserInterface.h"


/*
 * An input source that wraps and AdaEncoder encoder.
 */
template <
  char CHAR,
  uint8_t PIN_A,
  uint8_t PIN_B,
  uint8_t ID
>
class EncoderSrc : PollingInputSource {
  public:
    EncoderSrc() : 
      m_encoder(CHAR, PIN_A, PIN_B),
      id(ID) {};

    void init()
    {
      
    };

    void poll(UI &ui) {
      if (m_encoder.getClicks()) {
	ui.put(ID, m_encoder.query());
      }
    };

    const uint8_t id;

  private:
    AdaEncoder m_encoder;
};


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
    
    void handle_event(UI& ui, Event &event) {
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

template <int N> 
class Menu : public Screen 
{
  public:
    Menu(MenuItem (&items)[N]): m_items(items) {};
  
    void draw(Adafruit_GFX& display) {
      display.setTextSize(1);
      unsigned int i = 0;
    
      for (i = 0; i < N; i++) {
	if (i == m_pos) {
	  display.setTextColor(WHITE, BLACK);
	} else {
	  display.setTextColor(BLACK, WHITE);
	}

	display.println(m_items[i].name);
      }
    };

    void handle_event(UI& ui, Event &event) {
      switch (event.source) {
	case 1:
	  m_pos = min(N - 1,
		      max(0, 
			  m_pos + (signed char) event.data));
	  break;
	case 2:
	  ui.show(m_items[m_pos].screen);
	  Serial.print("got here");
	  break;
      };
    }

  private:
    MenuItem (&m_items)[N];
    unsigned char m_pos;
};


#endif
