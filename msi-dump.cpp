#include <array>
#include <fstream>
#include <iostream>
#include <locale>
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
  int32_t dirid_left;
  int32_t dirid_right;
  int32_t dirid_root;
  std::array<uint8_t, 16> uid;
  uint32_t user_flags;
  std::array<uint8_t, 8> ts_create; // u64 can't be aligned here
  std::array<uint8_t, 8> ts_update;
  secid_t secid_first;
  uint32_t stream_size;
  uint32_t unused;
};
static_assert(sizeof(dir_entry) == 128);

inline unsigned align_to(unsigned v, unsigned sec_size) {
  unsigned ceil = (v / sec_size) + ((v % sec_size == 0) ? 0 : 1);
  return ceil * sec_size;
}

template <typename T>
inline std::vector<T> read(std::ifstream &f, int32_t secid, uint32_t sec_size) {
  const auto hdr_size = align_to(sizeof(header), sec_size);

  auto offs = secid * sec_size + hdr_size;
  if (offs != f.rdbuf()->pubseekpos(offs))
    throw std::runtime_error("Failed to seek to sector");

  std::vector<T> buffer;
  buffer.resize(sec_size / sizeof(T));
  if (!f.rdbuf()->sgetn((char *)buffer.data(), sec_size))
    throw std::runtime_error("Failed to read sector");

  return std::move(buffer);
}

inline std::string to_str(const char16_t *chars, unsigned char_count) {
  std::string res;
  res.resize(char_count);
  for (unsigned i = 0; i < char_count; i++) {
    auto c = chars[i];
    if ((unsigned)c > 0xFFU)
      throw std::runtime_error("Non-ASCII in U16 string");
    res[i] = (int)c;
  }
  return res;
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

  const auto sec_size = 1 << h.pot_sec_size;
  const auto mini_sec_size = 1 << h.pot_minisec_size;

  if (h.secid_msat != -2)
    throw std::runtime_error("Extended MSAT not supported");

  // Build SAT from MSAT

  const auto sat = read<int32_t>(f, h.first_sect_fats[0], sec_size);
  if (sat[0] != -3)
    throw std::runtime_error("Invalid SAT sector");

  // Read dir from SAT

  const auto root_dir = read<dir_entry>(f, sat[h.secid_dir], sec_size);

  const auto &b = root_dir[0];
  const auto name_len = (b.name_size - 1) / 2;
  std::cout << "[" << to_str(b.name.data(), name_len) << "] " << (int)b.type
            << "\n";
}

int main(int argc, char **argv) { try_main(argc, argv); }
