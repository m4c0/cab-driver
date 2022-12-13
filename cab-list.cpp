#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

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
struct folder {
  uint32_t ofs_first_data;
  uint16_t num_data;
  uint16_t compress;
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
folder read_next_folder(std::streambuf *f, const header &hdr) {
  folder fld;
  if (!f->sgetn((char *)&fld, sizeof(fld)))
    throw std::runtime_error("Failed to read folder");

  if (fld.compress > 1)
    throw std::runtime_error("Unsupported compression type");

  f->pubseekoff(hdr.folder_extra, std::ios::cur);

  std::cout << "  Block count: " << fld.num_data << "\n";
  std::cout << "  Compress type: " << (fld.compress == 1 ? "MSZIP" : "NONE")
            << "\n";

  return fld;
}
file read_next_file(std::streambuf *f, const header &hdr) {
  file fl;
  if (!f->sgetn((char *)&fl, sizeof(file_data)))
    throw std::runtime_error("Failed to read file");

  if (fl.folder >= hdr.num_folders)
    throw std::runtime_error("Invalid folder");

  char c;
  fl.name = "";
  while ((c = f->sbumpc()) != '\0')
    fl.name += c;

  std::cout << "  Name: " << fl.name << "\n";
  std::cout << "  Size: " << fl.unc_size << "\n";
  std::cout << "  Date/Time: " << std::hex << fl.date << fl.time << std::dec
            << "\n";

  return fl;
}

void read(std::streambuf *f) {
  auto hdr = read_header(f);
  for (auto i = 0U; i < hdr.num_folders; i++) {
    std::cout << "Folder " << (i + 1) << ":\n";
    auto fld = read_next_folder(f, hdr);
  }

  f->pubseekpos(hdr.ofs_first_file);
  for (auto i = 0U; i < hdr.num_files; i++) {
    std::cout << "File " << (i + 1) << ":\n";
    auto fl = read_next_file(f, hdr);
  }
}

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
