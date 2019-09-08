#ifndef _DI_ENCODER_
#define _DI_ENCODER_

#include <map>
#include <string>
#include <vector>

namespace DynamicIndication {
namespace EncodePolicy {

template <typename Tout> struct ic74141Encoder {
  using Output_t = Tout;
  using Encode_map_t = std::map<typename std::string::value_type, Output_t>;

  ic74141Encoder(const std::string::value_type unknown_char_value,
                 const Encode_map_t &encodeMap)
      : encodemap(encodeMap), unknown_char_value(unknown_char_value) {}
  ic74141Encoder(const std::string::value_type unknown_char_value,
                 Encode_map_t &&encodeMap)
      : encodemap(std::move(encodeMap)),
        unknown_char_value(unknown_char_value) {}

  ic74141Encoder(const ic74141Encoder &) = default;
  ic74141Encoder(ic74141Encoder &&) = default;

  std::vector<Tout> encode(const std::string &input) {
    std::vector<Tout> result(input.size());

    auto src_it = input.cbegin();
    auto dest_it = result.begin();
    auto end = result.end();
    for (; dest_it != end; ++dest_it, ++src_it) {
      *dest_it = encode_symbol(*src_it);
    }

    return result;
  }

private:
  Encode_map_t encodemap;
  const std::string::value_type unknown_char_value;

  Output_t encode_symbol(const std::string::value_type symbol) {
    auto it = encodemap.find(symbol);
    return it != encodemap.end() ? it->second : unknown_char_value;
  }
};

} // namespace EncodePolicy

} // namespace DynamicIndication

#endif /* _DI_ENCODER_ */
