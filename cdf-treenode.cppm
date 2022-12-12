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

  bool visit(auto fn) const {
    if (!*this)
      return true;

    return left().visit(fn) && fn(this) && right().visit(fn) &&
           root().visit(fn);
  }
};
} // namespace cdf
