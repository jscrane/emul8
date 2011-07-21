#ifndef _CHAIN_H
#define _CHAIN_H


#include <chainbase.h>


template <class T>
class Chain : private ChainBase
{
public:
  /*  Iterators - the methods are defined here, rather than in the ih file,
   *              because of bugs in the g++ compiler when compiling templates
   *              with nested classes
   */
  class Iter;
  class ConstIter;
  
  class Iter
  {
  friend class Chain;
  friend class ConstIter;
  public:
    Iter() : 
      _current(0) {}
    Iter( const Iter &i ) : 
      _current(i._current) {}
    const Iter &operator = ( const Iter &i )
      { _current = i._current; return i; }
    
    operator bool () const 
      { return _current != 0; }
    
    bool operator == ( const Iter &i ) const
      { return _current == i._current; }
    bool operator != ( const Iter &i ) const
      { return _current != i._current; }
    
    Iter &operator ++ ()    // pre-increment
      { _current = (T*)(_current->next); return *this; }
    Iter operator ++ (int)  // post-increment
      { Iter temp = *this; _current = (T*)(_current->next); return temp; }
    Iter &operator -- ()    // pre-decrement
      { _current = (T*)(_current->prev); return *this; }
    Iter operator -- (int)  // post-decrement
      { Iter temp = *this; _current = (T*)(_current->prev); return temp; }
    
    operator T* ()
      { return _current; }
    T *operator -> ()
      { return _current; }
    T &operator * ()
      { return *_current; }
    
  private:
    Iter( T *link ) :
      _current(link) {}
    
    T *_current;
  };
  
  class ConstIter
  {
  friend class Chain;
  public:
    ConstIter() : 
      _current(0) {}
    ConstIter( const ConstIter &i ) : 
      _current(i._current) {}
    ConstIter( const Iter &i ) : 
      _current(i._current) {}
    const ConstIter &operator = ( const ConstIter &i )
      { _current = i._current; return *this; }
    const ConstIter &operator = ( const Iter &i )
      { _current = i._current; return *this; }
    
    operator bool () const 
      { return _current != 0; }
    
    bool operator == ( const ConstIter &i ) const
      { return _current == i._current; }
    bool operator != ( const ConstIter &i ) const
      { return _current != i._current; }
    
    ConstIter &operator ++ () {		// pre-increment
      _current = (const T*)(_current->next); 
      return *this;
    }
    ConstIter operator ++ (int) {	// post-increment
      ConstIter temp = *this; 
      _current = (const T*)(_current->next); 
      return temp; 
    }
    ConstIter &operator -- () {		// pre-decrement
      _current = (const T*)(_current->prev); 
      return *this; 
    }
    ConstIter operator -- (int) {	// post-decrement
      ConstIter temp = *this; 
      _current = (const T*)(_current->prev); 
      return temp; 
    }
    
    operator const T* () 
      { return _current; }
    const T *operator -> ()
      { return _current; }
    const T &operator * ()
      { return *_current; }
    
  private:
    ConstIter( const T *link ) :
      _current(link) {}
    
    const T *_current;
  };
  
  /*  Accessors
   */
  T *first() const { return (T*)(ChainBase::first()); }
  T *last() const { return (T*)(ChainBase::last()); }
  
  Iter begin() { return Iter(first()); }
  Iter end() { return Iter(0); }
  Iter forward() { return Iter(first()); }
  Iter reverse() { return Iter(last()); }

  ConstIter begin() const { return ConstIter(first()); }
  ConstIter end() const { return ConstIter(0); }
  ConstIter forward() const { return ConstIter(first()); }
  ConstIter reverse() const { return ConstIter(last()); }
  
  /*  Modifiers
   */
  void prepend( T* );            // Add to the front of the chain
  void append( T* );             // Add to the end of the chain
  void insert( T *t, Iter i )    // Add before item referenced by the Iter
    { ChainBase::insert(t,i._current); }
  void insert( T *t, T *before_this )
    { ChainBase::insert(t,before_this); }
  T *remove( Iter i )            // remove the item referenced by i
    { ChainBase::remove(i._current); return i._current; }
  T *remove( T* );               // remove an item
};


/*----------------------------------------------------------------------------
 *  Chain<T> methods
 */

template <class T> inline
void Chain<T>::prepend( T *t ) {
  ChainBase::prepend(t);
}

template <class T> inline
void Chain<T>::append( T *t ) {
  ChainBase::append(t);
}

template <class T> inline
T *Chain<T>::remove( T *t ) {
  ChainBase::remove(t);
  return t;
}


#if 0 // Included in the class definition (yuk!) because of problems with
      // g++ 2.7.2 and below

template <class T> inline
T *Chain<T>::remove( Iter i ) {
  ChainBase::remove( i._current );
  return _current;
}

template <class T> inline
Chain<T>::Iter Chain<T>::begin() {
  return Iter(_first);
}

template <class T> inline
Chain<T>::Iter Chain<T>::end() {
  return Iter(0);
}

/*----------------------------------------------------------------------------
 *  Chain<T>::Iter methods
 */

template <class T> inline
Chain<T>::Iter::Iter( T *link ) : _current(link) {
}

template <class T> inline
Chain<T>::Iter::Iter( const Iter &i ) : _current(i._current) {
}

template <class T> inline
Chain<T>::Iter::~Iter() {
}

template <class T> inline
const Chain<T>::Iter &Chain<T>::Iter::operator = ( const Iter &i ) {
  _current = i._current;
  return *this;
}

template <class T> inline
Chain<T>::Iter &Chain<T>::Iter::operator ++ () {
  _current = _current->next;
  return *this;
}

template <class T> inline
Chain<T>::Iter Chain<T>::Iter::operator ++ (int) {
  Iter temp = *this;
  _current = _current->next;
  return temp;
}

template <class T> inline
T * Chain<T>::Iter::operator -> () {
  return _current;
}

template <class T> inline
T &Chain<T>::Iter::operator * () {
  return *_current;
}

#endif // g++ bug workaround!

/*----------------------------------------------------------------------------
 */


#endif //_CHAIN_H
