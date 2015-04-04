#ifndef MVC_H
#define MVC_H

#include "UserInterface.h"

/*
 * Generic model class. 
 * 
 * Rather than implement the observer pattern, views poll their models
 * for dirty state. The assumption here is that we will do screen
 * updates at a fixed rate, and only redraw views that proxy dirty
 * values. All visible views that proxy dirty values would be redrawn
 * in the same tick of the event loop, and generally only one view for
 * a model would be visible at any given time. The dirty state will
 * then be automatically cleared at the end of the event loop tick.
 *
 * Model becomes dirty when its value is updates. It is considered
 * dirty until it is explicitly reset.
 */
template <typename T> class Model {
  public:
   virtual void update(T value);
   virtual T value();
   
   boolean dirty() {
      return m_dirty;
   };

   void reset() {
      m_dirty = false;
   };

  protected:
    boolean m_dirty;
};


/*
 * There are two types of Model: Direct models, and Proxy
 * models. Direct models simply wrap an internal copy of a value and
 * synchronously update.
 */
template <typename T> class DirectModel : public Model<T> {
  public:
   DirectModel(T val) :
      m_val(val) {
   };

   void update(T value) {
      m_val = value;
      Model<T>::m_dirty = true;
   };

   T value() {
      return m_val;
   };
   
  private:
   T m_val;
};


/*
 * Proxy models are intended for proxying remote data. You should
 * subclass ProxyModel and define update() as appropriate. update()
 * may or may not synchronously update the model value. Call
 * proxy_set() either internally or externally to commit the value
 * change.
 */
template <typename T> class ProxyModel : public Model<T> {
  public:
    ProxyModel(T initial) :
     m_cache(initial) {
   };

   void proxy_set(T value) {
      m_cache = value;
      Model<T>::m_dirty = true;
   };

   T value() {
      return m_cache;
   };

  private:
    T m_cache;
};


/*
 * Special case model for fixed-length character arrays. Update copies
 * the string into the internal buffer.
 */
template<uint8_t SIZE>
class DirectStringModel : public Model<const char *> {
  public:
    DirectStringModel(const char *initial) {
      strncpy(m_buffer, initial, SIZE);
    };

    void update(const char *value) {
      strncpy(m_buffer, value, SIZE);
      Model<const char *>::m_dirty = true;
    };

    const char *value () {
      return m_buffer;
    };

  private:
    char m_buffer[SIZE];
};


/*
 * Controller Base class
 */
class Controller {
  public:
    virtual void handle_event(UI &ui, Event &event);
};


#endif
