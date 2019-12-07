#ifndef _MY_JSL_PARCER_H_
#define _MY_JSL_PARCER_H_

#include <json/jsl-parser.h>

struct my_jsl_parser : public jsl_parser {
  my_jsl_parser(src_t &_src);

  jsl_data_dict *parse_no_seek();
};

#endif /* _MY_JSL_PARCER_H_ */
