#include "location.h"

struct Location sum(struct Location a, struct Location b)
{
  struct Location r;
  r.x = a.x + b.x;
  r.y = a.y + b.y;
  return r;
}
