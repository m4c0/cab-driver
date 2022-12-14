module;
#include <array>
#include <streambuf>

export module cdf;

import :pods;
import :reader;
import :tables;
import :treenode;

namespace cdf {
export auto read(std::streambuf *sb) {
  header h;

  if (!sb->sgetn((char *)&h, sizeof(h)))
    throw std::runtime_error("Failed to read header");

  constexpr const auto signature =
      std::array<uint8_t, 8>{0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1};

  if (h.doc_file_id != signature)
    throw std::runtime_error("Invalid signature");

  if (h.byte_order != 0xFFFE)
    throw std::runtime_error("Invalid byte order");

  if (h.secid_msat != -2)
    throw std::runtime_error("Extended MSAT not supported");

  return tables{sb, h};
}
export class tables;
export class dir_entry;
export using dir_name_t = decltype(dir_entry{}.name);
} // namespace cdf
