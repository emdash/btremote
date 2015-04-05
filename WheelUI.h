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

template<class T> struct SmartRef {
    T &ref;
};

struct LayoutItem {
   Rect bounds;
   Screen &ref;
};

template <
  uint8_t N_VIEWS,
  uint8_t N_CONTROLLERS
>
struct Layout {
    LayoutItem views[N_VIEWS];
    SmartRef<Controller> controllers[N_CONTROLLERS];
};


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
 * A screen screen which displays static text.
 */
class Label : public Screen {

   public:
      Label(const char *text, boolean wrap=true) : 
	 m_text(text),
	 m_wrap(wrap) {
      };

      void draw(Adafruit_GFX &display, const Rect &where) {
	 display.setTextWrap(m_wrap);
	 display.setCursor(where.x, where.y);
	 display.println(m_text);
      };

   private:
      const char *m_text;
      boolean m_wrap;
};


/*
 * A screen which displays a value range.
 */
template<typename T>
class RangeView : public Screen {
  public:
    RangeView(
      Model<T> &model,
      T min, 
      T max) :
      m_model(model),
      m_min(min),
      m_max(max) {
    };

    void draw(Adafruit_GFX &display, const Rect &where) {
      uint8_t 
	x1 = where.x,
	y1 = where.y + where.h,
	x2 = where.x + where.w,
	y2 = y1,
	x3 = x2,
	y3 = where.y,
	 tfill = (where.w * (m_model.value() - m_min)) / (m_max - m_min);

      // Draw a triangle outline that "fills up" according to the
      // volume level. I.e. at 0 volume, it's just a solid
      // outline. At full volume it's completely filled.

      // First, draw the filled triangle.
      display.fillTriangle(
	x1, y1,
	x2, y2,
	x3, y3,
	BLACK);
    
      // Now, erase the unfilled portion of the triangle.
      display.fillRect(
	where.x + tfill, where.y,
	where.w - tfill, where.h,
	WHITE);

      // now draw the outlined portion of the triangle.
      display.drawTriangle(
	x1, y1,
	x2, y2,
	x3, y3,
	BLACK);
    };

  private:
    Model<T> &m_model;
    T m_min;
    T m_max;
};


/*
 * A Screen which is composed of a set of screens and controllers. It
 * is defined by a Layout. A CompositeScreen allways draws all the
 * views in the layout and always passes each event off to all the
 * controllers in the layout.
 */
template
<
  uint8_t N_VIEWS,
  uint8_t N_CONTROLLERS
>
class CompositeScreen : public Screen {
  public:
    CompositeScreen(const Layout<N_VIEWS, N_CONTROLLERS> &layout) :
      m_layout(layout) {
    };

    void draw(Adafruit_GFX &display, const Rect &where) {
      for (uint8_t i = 0; i < N_VIEWS; i++) {
	m_layout.views[i].ref.draw(display,
				   m_layout.views[i].bounds);
      }
    };

    void handle_event(UI& ui, Event &event) {
      for (uint8_t i = 0; i < N_CONTROLLERS; i++) {
	m_layout.controllers[i].ref.handle_event(ui, event);
      }
    };

  private:
    const Layout<N_VIEWS, N_CONTROLLERS> &m_layout;
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
 * Displays a line of text larger than the screen by scrolling it
 * horizontally.
 */

class ScrolledText : public Screen {
  public:
    ScrolledText(const char *text) : m_text(text) {};
    
    void draw(Adafruit_GFX &display, const Rect &where) {
      display.setTextWrap(false);
      display.setTextSize(1);

      if (strlen(m_text) > (where.w) / 6) {
	uint8_t w = where.w;
	uint8_t m = w / 5;
	display.setCursor(w - ((millis() / 1000) % m) * 20, where.y);
	display.print(m_text);
      } else {
	display.setCursor(0, where.y);
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

      void draw(Adafruit_GFX &display, const Rect &where) {
	 if (m_model.value()) {
	    m_true.draw(display, where);
	 } else {
	    m_false.draw(display, where);
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
class IconView : public Screen {
   public:
      IconView(uint8_t w, uint8_t h, const uint8_t *data) : 
	 m_data(data),
	 m_w(w),
	 m_h(h) {
      };

      void draw(Adafruit_GFX &display, const Rect &where) {
	 // TODO: implement clipping according to bounds rect
	 display.drawBitmap(
	    where.x, where.y,
	    m_data,
	    m_w, m_h,
	    BLACK);
      };
   private:
      const uint8_t *m_data;
      const uint8_t m_w;
      const uint8_t m_h;
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


/*
 * Controller which pushes and pops screens.
 */
class NavController : public Controller {
  public:
    NavController(
      EventType push_source,
      uint8_t push_id) :
      m_push_source(push_source),
      m_push_id(push_id) {
    };

    virtual void action(UI &ui) {};

    void handle_event(UI &ui, Event &event) {
      if (event.source == m_push_source) {
	if (event.data == m_push_id) {
	  action(ui);
	}
      }
    };

  private:
    EventType m_push_source;
    const uint8_t m_push_id;
};

class PushController : public NavController {
  public:
    PushController(
      Screen &s,
      EventType src,
      uint8_t id) :
      NavController::NavController(src, id),
      m_screen(s) {
    };

    void action(UI &ui) {
      ui.push(m_screen);
    };

  private:
    Screen &m_screen;
};

class PopController : public NavController {
  public:
    PopController(EventType src, uint8_t id) :
      NavController::NavController(src, id) {
    };

    void action(UI &ui) {
      ui.pop();
    };
};

#endif
