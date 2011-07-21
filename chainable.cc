

#include <chainable.h>
#include <chainbase.h>


Chainable::~Chainable() {
  if( chain ) chain->remove(this);
}

