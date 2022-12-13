module;
#include <string>
#include <vector>
export module cab:pods;

export namespace cab {
struct header {
  uint32_t signature;
  uint32_t reserved_1;
  uint32_t cab_size;
  uint32_t reserved_2;
  uint32_t ofs_first_file;
  uint32_t reserved_3;
  uint16_t version;
  uint16_t num_folders;
  uint16_t num_files;
  uint16_t flags;
  uint16_t set_id;
  uint16_t id_cab_in_set;
  uint16_t header_extra;
  uint8_t folder_extra;
  uint8_t data_extra;
};
struct data_meta {
  uint32_t checksum;
  uint16_t comp_size;
  uint16_t uncomp_size;
};
struct data : data_meta {
  std::vector<uint8_t> compressed{};
};
struct folder_data {
  uint32_t ofs_first_data;
  uint16_t num_data;
  uint16_t compress;
};
struct folder : folder_data {
  std::vector<data> datas{};
};
struct file_data {
  uint32_t unc_size;
  uint32_t unc_offset;
  uint16_t folder;
  uint16_t date;
  uint16_t time;
  uint16_t attribs;
};
struct file : file_data {
  std::string name;
};

struct cab {
  header hdr;
  std::vector<folder> folders;
  std::vector<file> files;
};
} // namespace cab
