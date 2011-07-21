#ifndef _CHAINBASE_H
#define _CHAINBASE_H


class Chainable;


class ChainBase
{
public:
  ChainBase();
  ~ChainBase();
  
  Chainable *first() const;
  Chainable *last() const;
  
  void prepend( Chainable* );
  void append( Chainable* );
  void insert( Chainable *new_link, Chainable *before );     
  void remove( Chainable* );
  
private:
  Chainable *_first, *_last;
};


inline
ChainBase::ChainBase() : _first(0), _last(0) {
}

inline
Chainable *ChainBase::first() const {
  return _first;
}

inline
Chainable *ChainBase::last() const {
  return _last;
}




#endif // _CHAINBASE_H
