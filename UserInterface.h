#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <Adafruit_GFX.h>

const int SIZE = 8;


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
  virtual void handle_event(Event &) = 0;
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
  
  void handle_event(Event &event) {
    last_event = event;
  }
  
 private:  
  Event & last_event;
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
  
  void handle_event(Event &event) {
    switch (event.source) {
    case 1:
      m_value += event.data;
      break;
    case 2:
      m_value = 0;
      break; 
    };
  }
  
 private:
  int m_value;
}


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
  m_cur_screen(home),
    m_home(home),
    m_display(display) {
  };
  
  void show(Screen &screen) {
    m_cur_screen = screen;
  };
    
  void loop() {
    if (m_queue.count()) {
      m_cur_screen.handle_event(m_queue.get());
    }
    m_cur_screen.draw(m_display);
  };
    
  void put(unsigned char source, unsigned char data) {
    m_queue.put(source, data);
  };
    
 private:
  Screen &m_cur_screen;
  Screen &m_home;
  Adafruit_GFX& m_display;
  EventQueue m_queue;
  
  void draw() {
    m_cur_screen.draw(m_display);
  };
    
  void handle_event(Event &event) {
    m_cur_screen.handle_event(event);
  };
};

#endif
