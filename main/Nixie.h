#ifndef _NIXIE_H_
#define _NIXIE_H_

#include <string>

struct Nixie {
  inline static const std::string clear_indicators = "      ";

  static Nixie *instance();

  void setValue(const std::string &v);

private:
  static Nixie *inst;

  Nixie();
};

#endif /*_NIXIE_H_*/
