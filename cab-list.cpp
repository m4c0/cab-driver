#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

import cab;

void dump(const cab::cab &c) {
  const auto &hdr = c.hdr;
  std::cout << "Cabinet size: " << hdr.cab_size << "\n";

  for (auto i = 0U; i < hdr.num_folders; i++) {
    const auto &fld = c.folders[i];
    std::cout << "Folder " << (i + 1) << ":\n";
    std::cout << "  Compress type: " << (fld.compress == 1 ? "MSZIP" : "NONE")
              << "\n";

    for (auto j = 0U; j < fld.num_data; j++) {
      const auto &dt = fld.datas[j];
      std::cout << "  Data " << (j + 1) << ":\n";
      std::cout << "    Compressed size: " << dt.comp_size << "\n";
      std::cout << "    Uncompressed size: " << dt.uncomp_size << "\n";
    }
  }

  for (auto i = 0U; i < hdr.num_files; i++) {
    const auto &fl = c.files[i];
    std::cout << "File " << (i + 1) << ":\n";
    std::cout << "  Name: " << fl.name << "\n";
    std::cout << "  Size: " << fl.unc_size << "\n";
    std::cout << "  Date/Time: " << std::hex << fl.date << fl.time << std::dec
              << "\n";
  }
}

void try_main(int argc, char **argv) {
  if (argc != 2)
    throw std::runtime_error("Missing filename");

  std::ifstream f{argv[1]};
  if (!f)
    throw std::runtime_error("Could not open input file");

  dump(cab::read(f.rdbuf()));
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
