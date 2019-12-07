#include "my-jsl-parser.h"

my_jsl_parser::my_jsl_parser(jsl_parser::src_t &_src) : jsl_parser(_src) {}

jsl_data_dict *my_jsl_parser::parse_no_seek() { return eat_dict(); }
