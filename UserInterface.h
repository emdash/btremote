/* UserInterface.h
 *
 * An Arduino-compatible UI library for use with
 * Adafruit_GFX-compatible screens.
 * 
 * This is the core of a simple UI library that aims to be reasonably
 * complete and compatible with Adafruit_GFX displays. Aside from
 * being flexible with the choice of display, the library is also
 * designed to accomodate a range of programming styles and UI design
 * patterns, encouraging modeless UI where possible.
 *
 * A secondary, but no-less important design goal is efficiency. The
 * library allows the UI components to be declared statically, which
 * allows for greater compiler optimization and code size
 * reduction. At the same time, it is relatively easy to change the
 * arrangement of components
 *
 * Finally, this library aims to be as simple as possible, but no
 * simpler.
 *
 * The Event Queue
 *
 *  The Library is organized around EventQueue, which is a ring-buffer
 *  of events. Events are inserted into the EventQueue by
 *  InputSources, and are dispatched to one of an arbitrary number of
 *  screens. The InputSources and Screens are declared statically in
 *  the user's sketch.
 *
 * InputSources
 *
 *  Input sources can be either polling or interrupt-driven. A few
 *  basic input sources are provided, but it is easy to create your
 *  own if you have special needs not covered by the base library.
 *
 *  For polling InputSources, you must poll them in loop()
 *  before calling UI::loop(). For Interrupt-driven sources, the
 *  events are inserted into the queue automatically.
 *
 * Screens
 *
 *  Screens represent a particular mode of interaction. For example,
 *  on an audio player, you might have one screen for playback and
 *  another screen for scrolling through playlists. A couple of
 *  standard screens are provided, including TestScreen.
 *
 * Controllers
 *
 *  Controllers can be used to handle user input. They abstract away
 *  the details of handling certain patterns of events.
 */

#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <Adafruit_GFX.h>

const int SIZE = 8;

/*
 * Forward declare UI class. We need it in a few places.
 */
class UI;

/*
 * A class which represents an input event. Not much here to try and
 * keep things light-weight. Also trying not to make assumptions about
 * what type of input devices may be present.
 */
struct Event
{
  unsigned long time;
  unsigned char source;
  unsigned char data;
};

/*
 * Used to initialize Event references.
 */
Event null_event = {0, 0, 0};

/*
 * A place to buffer input events for further processing 
 */
class EventQueue {
 public:
  EventQueue() {
    m_front = 0;
    m_back = 0;
    m_count = 0;
  };
    
  void put(unsigned char source, unsigned char data) {
    if (m_count < SIZE) {
      m_queue[m_back].time = millis();
      m_queue[m_back].source = source;
      m_queue[m_back].data = data;
      m_back = (m_back + 1) % SIZE;
      m_count++;
    }
  };
    
  Event& get() {
    if (m_count > 0) {
      unsigned char idx = m_front;
      m_front = (m_front + 1) % SIZE;
      m_count--;
      
      return m_queue[idx];
    }
    
    return (Event &) null_event;
  };
  
  unsigned char count() {
    return m_count;
  };
  
 private:
  unsigned char m_front;
  unsigned char m_back;
  unsigned char m_count;

  Event m_queue[SIZE];
};


/*
 * Like a Window in a desktop system, but devoted to the entire
 * screen. A window receives a stream of events, and knows how to draw
 * itself to the screen.
 */
class Screen {
 public:
  Screen() {};
  virtual void draw (Adafruit_GFX &display) = 0;
  virtual void handle_event(UI& ui, Event &) = 0;
};


/*
 * A simple way to test whether the event system is working and
 * receiving events. It always displays the last received event.
 */
class TestScreen : public Screen {
 public:
  
 TestScreen(): 
  last_event(null_event)
  {
    /* nothing to be done for now */
  };
      
  void draw(Adafruit_GFX &display) {
    display.println(String("Time:") + last_event.time);
    display.println(String("Src: ") + last_event.source);
    display.println(String("Data: ") + last_event.data);
  }
  
  void handle_event(UI& ui, Event &event) {
    last_event = event;
  }
  
 private:  
  Event & last_event;
};


/*
 * Context class which manages all the screens and the event queue in
 * your sketch. You shoud declare a global instance of this class in
 * the top level of your sketch. Then you should call its loop method
 * from loop().
 *
 * For now, we need to manually inject events into the event loop with
 * put(). This will change when InputSources are implemented.
 */
class UI {
 public:
 UI(Adafruit_GFX& display, Screen &home):
  m_cur_screen(&home),
    m_home(home),
    m_display(display) {
  };
  
  void show(Screen &screen) {
    this->m_cur_screen = &screen;
  };
    
  void loop() {
    if (m_queue.count()) {
      m_cur_screen->handle_event(*this, m_queue.get());
    }
    Serial.println(String("Show:") + ((unsigned long) m_cur_screen));    
    m_cur_screen->draw(m_display);
  };
    
  void put(unsigned char source, unsigned char data) {
    m_queue.put(source, data);
  };
    
 private:
  Screen *m_cur_screen;
  Screen &m_home;
  Adafruit_GFX& m_display;
  EventQueue m_queue;
};


/*
 * A screen screen which displays static text. Optionally, a screen to
 * return to after an input event.
 */
class TextScreen : public Screen {

 public:
 TextScreen(const char *text, Screen *screen = 0)
   : m_text(text),
     m_screen(screen) 
     {
     };

  void draw(Adafruit_GFX &display) {
    display.println(m_text);
  };

  void handle_event(UI& ui, Event &event) {
    switch (event.source) {
    case 2:
      ui.show(*m_screen);
      break;
    };
  };

 private:
  const char *m_text;
  Screen *m_screen;
};


#endif
