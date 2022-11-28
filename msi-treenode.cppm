module;
#include <array>
#include <iomanip>
#include <span>
#include <sstream>
#include <vector>

export module msi:treenode;
import :pods;

namespace msi {
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

    const size_t name_len = (entry().name_size - 1) / 2;
    for (auto c : std::span{entry().name.data(), name_len}) {
      uint8_t uc = (uint8_t)c;
      if (uc == c) {
        if (uc >= 32 && uc < 127) {
          res << uc;
        } else {
          hex(c);
        }
      } else {
        unsigned a = (unsigned)c & 0xFF;
        unsigned b = ((unsigned)c >> 8) & 0xFF;
        hex(a);
        hex(b);
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
} // namespace msi
