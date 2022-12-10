module;
#include <array>
#include <strstream>
#include <vector>

export module msi:tables;
import :pods;
import :reader;
import :treenode;

namespace msi {
export class tables {
  unsigned m_min_sec_size;
  unsigned m_min_stream_size;
  reader m_reader;
  std::vector<secid_t> m_sat;
  std::vector<secid_t> m_ssat;
  std::vector<dir_entry> m_dirs;
  std::vector<uint8_t> m_sstr;

public:
  tables(std::streambuf *f, const header &h)
      : m_min_sec_size(1U << h.pot_minisec_size),
        m_min_stream_size(h.min_stream_size), m_reader(f, 1U << h.pot_sec_size),
        m_sat(m_reader.read_sat(h)),
        m_ssat(m_reader.read_chain<secid_t>(m_sat, h.secid_short_sat)),
        m_dirs(m_reader.read_chain<dir_entry>(m_sat, h.secid_dir)),
        m_sstr(m_reader.read_chain<uint8_t>(
            m_sat, treenode{m_dirs}.entry().secid_first)) {
    m_sstr.resize(treenode{m_dirs}.entry().stream_size);
  }

  template <typename T> inline auto read(secid_t secid) {
    return m_reader.read_chain<T>(m_sat, secid);
  }

  inline auto read_stream(secid_t secid, unsigned len) {
    std::vector<uint8_t> res;
    if (len < m_min_stream_size) {
      while (secid >= 0) {
        auto sz = res.size();
        res.resize(sz + m_min_sec_size);

        auto begin = m_sstr.begin() + secid * m_min_sec_size;
        std::copy(begin, begin + m_min_sec_size, res.begin() + sz);

        secid = m_ssat[secid];
      }
    } else {
      res = m_reader.read_chain<uint8_t>(m_sat, secid);
    }
    res.resize(len);
    return res;
  }

  inline auto read_stream(const dir_entry &e) {
    auto secid = e.secid_first;
    auto sz = e.stream_size;

    return read_stream(secid, sz);
  }

  inline void visit_tree(auto fn) const { treenode{m_dirs}.visit(fn); }
};
} // namespace msi
