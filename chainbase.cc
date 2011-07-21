#include <chainable.h>
#include <chainbase.h>


ChainBase::~ChainBase() {
  Chainable *link = _first;
  while( link ) {
    Chainable *temp = link->next;
    link->chain = 0;
    delete link;
    link = temp;
  }
}

void ChainBase::prepend( Chainable *link ) {
  if( link->chain ) link->chain->remove(link);
  
  link->chain = this;
  link->prev = 0;
  link->next = _first;
  
  if( _first ) _first->prev = link;
  if( !_last ) _last = link;
  _first = link;
}

void ChainBase::append( Chainable *link ) {
  if( link->chain ) link->chain->remove(link);
  
  link->chain = this;
  link->prev = _last;
  link->next = 0;
  
  if( _last ) _last->next = link;
  if( !_first ) _first = link;
  _last = link;
}

void ChainBase::insert( Chainable *new_link, Chainable *before ) {
  if( new_link->chain ) new_link->chain->remove(new_link);
  
  new_link->chain = this;
  new_link->next = before;
  
  if( before ) {
    new_link->prev = before->prev;
    if( before->prev ) {
      before->prev->next = new_link;
    } else {
      _first = new_link;
    }
    before->prev = new_link;
  } else {
    // Insert at end
    new_link->prev = _last;
    if( _last ) {
      _last->next = new_link;
    } else {
      _first = new_link;
    }
    _last = new_link;
  }
}

void ChainBase::remove( Chainable *link ) {
  if( link->prev ) {
    link->prev->next = link->next;
  } else {
    _first = link->next;
  }
  
  if( link->next ) {
    link->next->prev = link->prev;
  } else {
    _last = link->prev;
  }
  
  link->chain = 0;
}
