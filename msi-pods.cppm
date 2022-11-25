module;
#include <array>

export module msi:pods;

export namespace msi {
using dirid_t = int32_t;
using secid_t = int32_t;

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

} // namespace msi

static_assert(sizeof(msi::secid_t) == 4);
static_assert(sizeof(msi::dir_entry) == 128);
static_assert(sizeof(msi::header) == 512);
