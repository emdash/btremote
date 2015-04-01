/* WheelUI.h
 * 
 * A UI library for encoder-driven menu-based UIs.
 *
 * Provides some features on top of UserInterface.h based on the
 * assumption that the device has at least one encoder wheel
 * and one or more buttons.
 */

#ifndef WHEELUI_H
#define WHEELUI_H

#include <AdaEncoder.h>
#include "UserInterface.h"


/* 
 * This enum defines types of our library is concerned
 * with. Applications should define input sources which map to these
 * events.
 */
typedef enum {
   WHEEL = 1,      // raw wheel event
   BUTTON_PRESS,   // raw button-press event
   BUTTON_RELEASE, // raw button-relase event
   CLICK,          // high-level click event
   HOLD,           // high-level hold event
   NAVIGATION,     // high-level navigation event
} WheelUIEvents;

/*
 * A screen screen which displays static text. Optionally, a screen to
 * return to after an input event.
 */
class TextScreen : public Screen {

   public:
      TextScreen(const char *text, uint8_t back_button) : 
	 m_text(text),
	 m_back_button(back_button) {};

      void draw(Adafruit_GFX &display) {
	 display.println(m_text);
      };

      void handle_event(UI& ui, Event &event) {
	 if (event.data && 
	     event.source == m_back_button) {
	    ui.put(255, 0);
	 }
      };

   private:
      const char *m_text;
      const uint8_t m_back_button;
};


/*
 * An input source bound to an I/O pin. Treats the pin as a momentary
 * push-button.
 */
template
<
uint8_t PIN, 
   uint8_t MODE,
   uint8_t ID,
   uint8_t BUTTON_PRESS,
   uint8_t BUTTON_RELEASE
   boolean INVERTED
>
class ButtonSrc : PollingInputSource {
   public:
      ButtonSrc() : id(ID),
		    m_debounce(0), 
		    m_state(INVERTED) {};

      void init() {
	 pinMode(PIN, MODE);
	 m_state = digitalRead(PIN);
      };

      void poll(UI &ui) {
	 if (millis() < m_debounce) {
	    return;
	 }

	 boolean state = digitalRead(PIN);

	 if (state != m_state) {
	    if (INVERTED) {
	       ui.put(state ? BUTTON_RELEASE : BUTTON_PRESS, ID);
	    } else {
	       ui.put(state ? BUTTON_PRESS : BUTTON_RELEASE, ID);
	    }
	    m_state = state;
	    m_debounce = millis() + 100;
	 }
      };

      const uint8_t id;
   private:
      unsigned long m_debounce;
      boolean m_state;
};


/*
 * An input source that wraps and AdaEncoder encoder.
 */
template <
  char CHAR,
  uint8_t PIN_A,
  uint8_t PIN_B,
  uint8_t ID
>
class EncoderSrc : public PollingInputSource {
  public:
    EncoderSrc() : 
      id(ID),
      m_encoder(CHAR, PIN_A, PIN_B) {};

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
    };
    
    void handle_event(UI& ui, Event &event) {
      switch (event.source) {
	case 1:
	  m_value += (signed char) event.data;
	  break;
	case 2:
	  m_value = 0;
	  break; 
      };
    };
    
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
    Menu(MenuItem (&items)[N]): 
    m_items(items) {};  
  
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
	  ui.push(m_items[m_pos].screen);
	  Serial.print("got here");
	  break;
      };
    }

  private:
    MenuItem (&m_items)[N];
    unsigned char m_pos;
};


#endif
