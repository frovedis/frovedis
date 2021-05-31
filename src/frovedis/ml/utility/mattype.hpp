#ifndef MATTYPE_HPP
#define MATTYPE_HPP

namespace frovedis {

enum MatType {
  CRS = 0,
  JDS,
  ELL,
  HYBRID
};

enum { NONE = 0xDEAD };

}
#endif
