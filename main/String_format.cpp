#include <cstdarg>
#include <string>
#include <vector>

#include <cinttypes>

#include "String_format.h"

#define INITIAL_BUF_SIZE 128

std::string vtformat_impl(const std::string &fmt,
                          const std::vector<std::string> &strs) {
  static const char FORMAT_SYMBOL = '%';
  std::string res;
  std::string buf;
  bool arg = false;
  for (int i = 0; i <= static_cast<int>(fmt.size()); ++i) {
    bool last = i == static_cast<int>(fmt.size());
    char ch = fmt[i];
    if (arg) {
      if (ch >= '0' && ch <= '9') {
        buf += ch;
      } else {
        int num = 0;
        if (!buf.empty() && buf.length() < 10)
          num = atoi(buf.c_str());
        if (num >= 1 && num <= static_cast<int>(strs.size()))
          res += strs[num - 1];
        else
          res += FORMAT_SYMBOL + buf;
        buf.clear();
        if (ch != FORMAT_SYMBOL) {
          if (!last)
            res += ch;
          arg = false;
        }
      }
    } else {
      if (ch == FORMAT_SYMBOL) {
        arg = true;
      } else {
        if (!last)
          res += ch;
      }
    }
  }
  return res;
}

std::string to_string(int32_t x) {
  // -2147483647
  char buf[16];
  snprintf(buf, sizeof(buf) - 2, "%d", x);
  return std::string(buf);
}

std::string to_string(uint32_t x) {
  // 4294967295
  char buf[16];
  snprintf(buf, sizeof(buf) - 2, "%u", x);
  return std::string(buf);
}

std::string to_string(int64_t x) {
  // 18446744073709551616
  char buf[24];
  snprintf(buf, sizeof(buf) - 2, "%" PRId64, x);
  return std::string(buf);
}

std::string to_string(uint64_t x) {
  // 18446744073709551616
  char buf[24];
  snprintf(buf, sizeof(buf) - 2, "%" PRIu64, x);
  return std::string(buf);
}

std::string to_string(float x) {
  char buf[24];
  snprintf(buf, sizeof(buf) - 2, "%f", x);
  return std::to_string(x);
}

std::string to_string(double x) {
  char buf[24];
  snprintf(buf, sizeof(buf) - 2, "%f", x);
  return std::to_string(x);
}

std::string to_string(const char *x) { return std::string(x); }

std::string to_string(const std::string &x) { return x; }

std::string format(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::vector<char> v(INITIAL_BUF_SIZE);
  while (true) {
    va_list args2;
    va_copy(args2, args);
    int res = vsnprintf(v.data(), v.size(), fmt, args2);
    if ((res >= 0) && (res < static_cast<int>(v.size()))) {
      va_end(args);
      va_end(args2);
      return std::string(v.data());
    }
    size_t size;
    if (res < 0)
      size = v.size() * 2;
    else
      size = static_cast<size_t>(res) + 1;
    v.clear();
    v.resize(size);
    va_end(args2);
  }
}
