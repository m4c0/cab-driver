module;
#include <strstream>
#include <vector>

export module msi:tables;
import :pods;
import :reader;

namespace msi {
export class tables {
  unsigned m_min_stream_size;
  reader m_reader;
  std::vector<secid_t> m_sat;
  std::vector<secid_t> m_ssat;
  std::vector<uint8_t> m_sstr;

public:
  tables(std::streambuf *f, const header &h)
      : m_reader(f, 1U << h.pot_sec_size), m_min_stream_size(h.min_stream_size),
        m_sat(m_reader.read_sat(h)),
        m_ssat(m_reader.read_chain<secid_t>(m_sat, h.secid_short_sat)),
        m_sstr(m_reader.read_chain<uint8_t>(m_ssat, 0 /*wrong*/)) {}

  template <typename T> inline auto read(secid_t secid) {
    return m_reader.read_chain<T>(m_sat, secid);
  }

  inline auto read_stream(secid_t secid, unsigned len) {
    if (len < m_min_stream_size) {
      // return m_reader.read_chain<uint8_t>(m_ssat, secid);
      //  use ssat to read into a "short-stream container stream"
    } else {
      return m_reader.read_chain<uint8_t>(m_sat, secid);
    }
  }
};
} // namespace msi
