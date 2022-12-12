module;
#include <array>
#include <iomanip>
#include <span>
#include <sstream>
#include <vector>

export module cdf:treenode;
import :pods;

namespace cdf {
export class treenode {
  const std::vector<dir_entry> &m_entries;
  dirid_t m_id;
  unsigned m_depth;

public:
  explicit treenode(const std::vector<dir_entry> &e) : treenode{e, 0, 0} {}
  treenode(const std::vector<dir_entry> &e, dirid_t id, unsigned depth)
      : m_entries{e}, m_id{id}, m_depth{depth} {}

  [[nodiscard]] explicit operator bool() const { return m_id >= 0; }

  [[nodiscard]] auto depth() const { return m_depth; }
  [[nodiscard]] const auto &entry() const { return m_entries[m_id]; }

  [[nodiscard]] treenode left() const {
    return {m_entries, entry().dirid_left, m_depth + 1};
  };
  [[nodiscard]] treenode right() const {
    return {m_entries, entry().dirid_right, m_depth + 1};
  };
  [[nodiscard]] treenode root() const {
    return {m_entries, entry().dirid_root, m_depth + 1};
  };

  [[nodiscard]] std::string name() const {
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

    const size_t name_len = (entry().name_size - 1) / 2;
    for (auto c : std::span{entry().name.data(), name_len}) {
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

  bool visit(auto fn) const {
    if (!*this)
      return true;

    return left().visit(fn) && fn(this) && right().visit(fn) &&
           root().visit(fn);
  }
};
} // namespace cdf
