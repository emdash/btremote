#ifndef MVC_H
#define MVC_H

#include "UserInterface.h"

/*
 * Generic model class. 
 * 
 * Rather than implement the observer pattern for views, views poll
 * their models for dirty state. The assumption here is that we will
 * do screen updates at a fixed rate, and only redraw views that proxy
 * dirty values. All visible views that proxy dirty values would be
 * redrawn in the same tick of the event loop, and generally only one
 * view for a model would be visible at any given time. The dirty state
 * will then be automatically cleared at the end of the even loop tick.
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
 * There are two types of Model: Reference models, and Proxy
 * models. Reference models simply update a plain reference to a
 * value. Proxy models take a function pointer, which is invoked to
 * update the model. The function should ensure that the proxy_set()
 * method is called finalize the update. Moreover, proxy_set may be
 * called asynchronously.
 */
template <typename T> class RefModel : public Model<T> {
  public:
   RefModel(T &ref) :
      m_ref(ref) {
   };

   void update(T value) {
      m_ref = value;
      Model<T>::m_dirty = true;
   };

   T value() {
      return m_ref;
   };
   
  private:
   T &m_ref;
};


template <typename T> class ProxyModel : public Model<T> {
  public:
   typedef boolean (Callback)(ProxyModel<T> &model, T value);

   ProxyModel(Callback callback, T initial) :
     ProxyModel<T>::m_callback(callback),
     m_cache(initial),
     Model<T>::m_dirty(true) 
   {
      m_callback(this, initial);
   };

   void update(T value) {
      m_callback(*this, value);
   };

   void proxy_set(T value) {
      m_cache = value;
      Model<T>::m_dirty = true;
   };

   T value() {
      return m_cache;
   };

  private:
   Callback m_callback;
   T m_cache;
};




#endif
