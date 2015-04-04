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
#include "MVC.h"


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
} EventType;

#define CLICK_THRESHOLD 1000

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
	    put(ui, INVERTED ? !state : state);
	    m_state = state;
	    m_debounce = millis() + 100;
	 }
      };

      const uint8_t id;

   private:
      void put(UI &ui, boolean state) {
	 if (state) {
	    ui.put(BUTTON_PRESS, ID);
	    m_pressed = millis();
	 } else {
	    ui.put(BUTTON_RELEASE, ID);
	    if (millis() > m_pressed + CLICK_THRESHOLD) {
	       ui.put(HOLD, ID);
	    } else {
	       ui.put(CLICK, ID);
	    }
	 }
      };

      unsigned long m_pressed;
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



/*
 * Displays a line of text larger than the screen by scrolling it
 * horizontally.
 */
template <
   uint8_t Y,
   uint8_t CHARS_PER_LINE
>
class ScrolledText : public Screen {
   public:
      ScrolledText(const char *text) : m_text(text) {};

      void draw(Adafruit_GFX &display) {
	 if (strlen(m_text) > CHARS_PER_LINE) {
	    // we must ignore the dirty bit in this case, since we are
	    // contantly updating the screen.
	    uint8_t w = display.width() - 1;
	    uint8_t m = w / 5;
	    display.setCursor(w - ((millis() / 1000) % m) * 20, Y);
	    display.print(m_text);
	 } else {
	    display.setCursor(0, Y);
	    display.print(m_text);
	 }
      };

   private:
      const char *m_text;
};


/*
 * Switches between two views based on the state of a boolean.
 */
class ToggleView : public Screen {
   public:
      ToggleView(Model<boolean> &model,
		 Screen &if_true,
		 Screen &if_false) :
	 m_model(model),
	 m_true(if_true),
	 m_false(if_false) {
      };

      void draw(Adafruit_GFX &display) {
	 if (m_model.value()) {
	    m_true.draw(display);
	 } else {
	    m_false.draw(display);
	 }
      };
   private:
      Model<boolean> &m_model;
      Screen &m_true;
      Screen &m_false;
};


/*
 * Draws an icon at the specified coordinates and dimensions.
 */
template <
   uint8_t W,
   uint8_t H
>
class IconView : public Screen {
   public:
      IconView(uint8_t x,
	       uint8_t y,
	       const uint8_t *data) : 
	 m_data(data),
	 m_x(x),
	 m_y(y) {
      };

      void draw(Adafruit_GFX &display) {
	 display.drawBitmap(
	    m_x, m_y,
	    m_data,
	    W, H,
	    BLACK);
      };
   private:
      const uint8_t *m_data;
      const uint8_t m_x;
      const uint8_t m_y;
};


/*
 * Controller which uses a button to toggle a boolean value.
 */
class Toggle : public Controller {
   public:
      Toggle(Model<boolean> &model,
	     uint8_t button_id,
	     EventType type = CLICK) : 
	 m_model(model),
	 m_id(button_id),
	 m_type(type) {
      };

      void handle_event(UI &ui, Event &event) {
	 if (event.source == m_type) {
	    if (event.data == m_id) {
	       m_model.update(!m_model.value());
	    };
	 }
      };

   private:
      Model<boolean> &m_model;
      const uint8_t m_id;
      EventType m_type;
};


/*
 * Controller which uses the encoder wheel to adjust a scalar value.
 */
template <typename T>
class Knob : public Controller {
   public:
      Knob(Model<T> &model, T coefficient, T min, T max) :
	 m_model(model),
	 m_coefficient(coefficient),
	 m_min(min),
	 m_max(max)
      {
      };

      void handle_event(UI &ui, Event &event) {
	 if (event.source == WHEEL) {
	    m_model.update(
	       constrain(
		  m_model.value() + m_coefficient * char(event.data),
		  m_min,
		  m_max));
	 };
      };

   private:
      Model<T> &m_model;
      T m_coefficient;
      T m_min;
      T m_max;
};

#endif
