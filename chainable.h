#ifndef _CHAINABLE_H
#define _CHAINABLE_H


class ChainBase;


class Chainable
{
public:
  Chainable *prev, *next;
  ChainBase *chain;
  
  Chainable();
  virtual ~Chainable();
};


inline Chainable::Chainable() : prev(0), next(0), chain(0) {
}


#endif // _CHAINABLE_H
