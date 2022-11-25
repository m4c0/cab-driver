module;
#include <array>
#include <streambuf>

export module msi:reader;
import :pods;

namespace msi {
export class reader {
  std::streambuf *m_f;
  unsigned m_sec_size;
  unsigned m_hdr_size;

  static inline unsigned align_to(unsigned sec_size) {
    unsigned v = sizeof(header);
    unsigned ceil = (v / sec_size) + ((v % sec_size == 0) ? 0 : 1);
    return ceil * sec_size;
  }

  template <typename T> inline void read(int32_t secid, std::vector<T> &out) {
    if (secid < 0)
      return;

    auto offs = secid * m_sec_size + m_hdr_size;
    if (offs != m_f->pubseekpos(offs))
      throw std::runtime_error("Failed to seek to sector");

    auto bsz = out.size();
    out.resize(bsz + m_sec_size / sizeof(T));
    if (!m_f->sgetn((char *)(out.data() + bsz), m_sec_size))
      throw std::runtime_error("Failed to read sector");
  }

public:
  reader(std::streambuf *f, unsigned sec_size)
      : m_f{f}, m_sec_size{sec_size}, m_hdr_size{align_to(sec_size)} {}

  template <typename T>
  inline std::vector<T> read_chain(std::vector<int32_t> sat, int32_t secid) {
    std::vector<T> res;
    while (secid >= 0) {
      read(secid, res);
      secid = sat[secid];
    }
    return res;
  }

  inline std::vector<secid_t> read_sat(const header &h) {
    std::vector<secid_t> sat;
    for (auto s : h.first_sect_fats) {
      read(s, sat);
    }
    if (sat[0] != -3)
      throw std::runtime_error("Invalid SAT sector");
    return sat;
  }
};
} // namespace msi
