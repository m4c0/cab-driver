module;
#include <array>
#include <iomanip>
#include <span>
#include <sstream>
#include <string>

export module msi:name;

import cdf;

namespace msi {
export [[nodiscard]] std::string decode_name(const cdf::dir_entry &entry) {
  std::stringstream res;
  res << std::setfill('0') << std::hex;
  const auto hex = [&res](unsigned c) {
    res << "<" << std::setw(2) << (unsigned)c << std::setw(1) << ">";
  };
  const auto b64 = [&res](uint8_t c) {
    if (c < 10) {
      res << (char)('0' + c);
    } else if (c < 36) {
      res << (char)('A' + c - 10);
    } else if (c < 62) {
      res << (char)('a' + c - 36);
    } else if (c < 63) {
      res << '.';
    } else {
      res << '_';
    }
  };

  const size_t name_len = (entry.name_size - 1) / 2;
  for (auto c : std::span{entry.name.data(), name_len}) {
    if (c >= 0x4800) {
      b64(c - 0x4800);
    } else if (c >= 0x3800) {
      c -= 0x3800;
      b64(c & 0x3f);
      b64(c >> 6);
    } else if (c >= 32 && c < 127) {
      res << (char)c;
    } else {
      hex(c);
    }
  }
  return res.str();
}
} // namespace msi
