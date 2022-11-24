#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <span>
#include <string_view>
#include <vector>

using secid_t = int32_t;
static_assert(sizeof(secid_t) == 4);

struct header {
  std::array<uint8_t, 8> doc_file_id;
  std::array<uint8_t, 16> uid;
  uint16_t rev_number;
  uint16_t ver_humber;
  uint16_t byte_order;
  uint16_t pot_sec_size;
  uint16_t pot_minisec_size;
  std::array<uint8_t, 10> unused_1;
  uint32_t numsec_sat;
  secid_t secid_dir;
  uint32_t unused_2;
  uint32_t min_stream_size;
  secid_t secid_short_sat;
  uint32_t numsec_short_sat;
  secid_t secid_msat;
  uint32_t numsec_msat;
  std::array<int32_t, 109> first_sect_fats;
};
static_assert(sizeof(header) == 512);

using dirid_t = int32_t;
enum dir_entry_type : uint8_t {
  empty = 0,
  user_storage = 1,
  user_stream = 2,
  lock_bytes = 3,
  property = 4,
  root_storage = 5,
};
struct dir_entry {
  std::array<char16_t, 32> name;
  uint16_t name_size; // in bytes + NIL (1b)
  dir_entry_type type;
  enum : uint8_t { red, black } color;
  dirid_t dirid_left;
  dirid_t dirid_right;
  dirid_t dirid_root;
  std::array<uint8_t, 16> uid;
  uint32_t user_flags;
  std::array<uint8_t, 8> ts_create; // u64 can't be aligned here
  std::array<uint8_t, 8> ts_update;
  secid_t secid_first;
  uint32_t stream_size;
  uint32_t unused;
};
static_assert(sizeof(dir_entry) == 128);

class reader {
  std::streambuf *m_f;
  unsigned m_sec_size;
  unsigned m_hdr_size;

  static inline unsigned align_to(unsigned sec_size) {
    unsigned v = sizeof(header);
    unsigned ceil = (v / sec_size) + ((v % sec_size == 0) ? 0 : 1);
    return ceil * sec_size;
  }

public:
  reader(std::ifstream &f, unsigned sec_size)
      : m_f{f.rdbuf()}, m_sec_size{sec_size}, m_hdr_size{align_to(sec_size)} {}

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
};

inline void dump_str(const char16_t *chars, unsigned char_count) {
  for (auto c : std::span{chars, char_count}) {
    uint8_t uc = (uint8_t)c;
    if (uc == c) {
      std::cout << uc;
    } else {
      uint16_t sc = (uint16_t)((int)c & 0xFFFFU);
      std::cout << std::setw(4) << std::hex << sc << " " << std::dec;
    }
  }
  std::cout << ":" << char_count;
}

int dump_tree(auto &dir_entries, dirid_t dirid, unsigned depth = 10,
              const std::string &ind = "") {
  if (dirid < 0 || depth <= 0)
    return 0;

  const auto &b = dir_entries[dirid];
  const auto name_len = (b.name_size - 1) / 2;
  std::cout << ind << "[";
  dump_str(b.name.data(), name_len);
  std::cout << "] t:" << (int)b.type << " " << b.dirid_left << " "
            << b.dirid_right << " " << b.dirid_root
            << " secid:" << b.secid_first << " sz:" << b.stream_size << "\n";
  return b.stream_size +
         dump_tree(dir_entries, b.dirid_left, depth - 1, ind + " ") +
         dump_tree(dir_entries, b.dirid_right, depth - 1, ind + " ") +
         dump_tree(dir_entries, b.dirid_root, depth - 1, ind + " ");
}

void try_main(int argc, char **argv) {
  if (argc != 2)
    throw std::runtime_error("Missing filename");

  std::ifstream f{argv[1]};
  if (!f)
    throw std::runtime_error("Could not open input file");

  header h;

  if (!f.rdbuf()->sgetn((char *)&h, sizeof(h)))
    throw std::runtime_error("Failed to read header");

  constexpr const auto signature =
      std::array<uint8_t, 8>{0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1};

  if (h.doc_file_id != signature)
    throw std::runtime_error("Invalid signature");

  if (h.byte_order != 0xFFFE)
    throw std::runtime_error("Invalid byte order");

  const auto sec_size = 1U << h.pot_sec_size;
  const auto mini_sec_size = 1U << h.pot_minisec_size;
  std::cerr << "Sector size: " << sec_size << "\n";

  if (h.secid_msat != -2)
    throw std::runtime_error("Extended MSAT not supported");

  reader r{f, sec_size};

  // Build SAT from MSAT

  std::vector<secid_t> sat;
  for (auto s : h.first_sect_fats) {
    r.read(s, sat);
  }
  if (sat[0] != -3)
    throw std::runtime_error("Invalid SAT sector");

  // Read dir from SAT

  std::vector<dir_entry> root_dir;
  auto secid = h.secid_dir;
  while (secid >= 0) {
    r.read(secid, root_dir);
    secid = sat[secid];
  }

  int sz = dump_tree(root_dir, 0);
  std::cout << "Total stream size: " << sz << "\n";
}

int main(int argc, char **argv) { try_main(argc, argv); }
