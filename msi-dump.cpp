#include <array>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>

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
  int32_t secid_dir;
  uint32_t unused_2;
  uint32_t min_stream_size;
  int32_t secid_short_sat;
  uint32_t numsec_short_sat;
  int32_t secid_msat;
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
  std::array<char, 64> name;
  uint16_t name_len;
  dir_entry_type type;
  std::array<uint8_t, 61> rest;
};
static_assert(sizeof(dir_entry) == 128);

inline unsigned align_to(unsigned v, unsigned sec_size) {
  unsigned ceil = (v / sec_size) + ((v % sec_size == 0) ? 0 : 1);
  return ceil * sec_size;
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
    throw std::runtime_error("MSAT not supported");

  const auto hdr_size = align_to(sizeof(header), sec_size);

  // Build SAT from MSAT

  auto offs = h.first_sect_fats[0] * sec_size + hdr_size;
  if (offs != f.rdbuf()->pubseekpos(offs))
    throw std::runtime_error("Failed to seek to sector");

  std::vector<int32_t> buffer;
  buffer.resize(sec_size / sizeof(int32_t));
  if (!f.rdbuf()->sgetn((char *)buffer.data(), sec_size))
    throw std::runtime_error("Failed to read sector");

  if (buffer[0] != -3)
    throw std::runtime_error("Invalid SAT sector");

  std::cout << h.secid_dir << " " << buffer[h.secid_dir] << " "
            << buffer[buffer[h.secid_dir]] << "\n";
  std::cout << h.secid_short_sat << " " << buffer[h.secid_short_sat] << "\n";

  // Read dir from SAT

  /*
  auto dir_offs = h.secid_dir * sec_size + sizeof(header);
  if (dir_offs != f.rdbuf()->pubseekpos(dir_offs))
    throw std::runtime_error("Failed to seek to root dir entry");

  std::vector<dir_entry> buffer;
  buffer.resize(sec_size / sizeof(dir_entry));
  if (!f.rdbuf()->sgetn((char *)buffer.data(), sec_size))
    throw std::runtime_error("Failed to read sector");

  auto &b = buffer[0];
  std::cout << b.name_len << " " << (int)b.type << "\n";
  // std::cout << "[" << std::string_view{b.name.data(), b.name_len} << "] " <<
  // (int)b.type << "\n";
  */
}

int main(int argc, char **argv) { try_main(argc, argv); }
