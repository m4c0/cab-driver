#include <cstdint>
#include <fstream>
#include <iostream>

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

header read_header(std::streambuf *f) {
  header hdr;
  if (!f->sgetn((char *)&hdr, sizeof(hdr)))
    throw std::runtime_error("Failed to read header");

  if (hdr.signature != 'FCSM')
    throw std::runtime_error("Signature doesn't match");

  if (hdr.version != 0x0103)
    throw std::runtime_error("Unsupported version");

  if (hdr.set_id != 0 || hdr.id_cab_in_set != 0)
    throw std::runtime_error("Sets are not supported");

  switch (hdr.flags) {
  case 0x0:
    hdr.header_extra = 0;
    hdr.folder_extra = 0;
    hdr.data_extra = 0;
    f->pubseekoff(-4, std::ios::cur);
    break;
  case 0x4:
    f->pubseekoff(hdr.header_extra, std::ios::cur);
    break;
  default:
    throw std::runtime_error("Sets are not supported");
  }

  std::cout << "Cabinet size: " << hdr.cab_size << "\n";
  std::cout << "Folder count: " << hdr.num_folders << "\n";
  std::cout << "File count: " << hdr.num_files << "\n";

  return hdr;
}

void read(std::streambuf *f) { auto h = read_header(f); }

void try_main(int argc, char **argv) {
  if (argc != 2)
    throw std::runtime_error("Missing filename");

  std::ifstream f{argv[1]};
  if (!f)
    throw std::runtime_error("Could not open input file");

  read(f.rdbuf());
}

int main(int argc, char **argv) {
  try {
    try_main(argc, argv);
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Failure: " << e.what() << std::endl;
    return 1;
  }
}
