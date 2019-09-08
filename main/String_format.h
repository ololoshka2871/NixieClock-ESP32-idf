#ifndef _STRING_FORMAT_H_
#define _STRING_FORMAT_H_

#include <string>
#include <vector>

std::string to_string(int x);
std::string to_string(unsigned int x);
std::string to_string(long x);
std::string to_string(unsigned long x);
std::string to_string(long long x);
std::string to_string(unsigned long long x);
std::string to_string(float x);
std::string to_string(double x);
std::string to_string(long double x);
std::string to_string(const char *x);
std::string to_string(const std::string &x);

std::string vtformat_impl(const std::string &fmt,
                          const std::vector<std::string> &strs);

template <typename Arg, typename... Args>
inline std::string vtformat_impl(const std::string &fmt,
                                 std::vector<std::string> &strs, Arg &&arg,
                                 Args &&... args) {
  strs.push_back(to_string(std::forward<Arg>(arg)));
  return vtformat_impl(fmt, strs, std::forward<Args>(args)...);
}

inline std::string vtformat(const std::string &fmt) { return fmt; }

template <typename Arg, typename... Args>
inline std::string vtformat(const std::string &fmt, Arg &&arg,
                            Args &&... args) {
  std::vector<std::string> strs;
  return vtformat_impl(fmt, strs, std::forward<Arg>(arg),
                       std::forward<Args>(args)...);
}

std::string format(const char *fmt, ...);

#endif /*_STRING_FORMAT_H_*/
