#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

import cab;

const auto &find_file(const cab::cab &c, const std::string &fn) {
  for (auto i = 0U; i < c.hdr.num_files; i++) {
    const auto &fl = c.files[i];
    if (fl.name != fn)
      continue;

    return fl;
  }

  throw std::runtime_error("Could not find target file");
}
void xt(const cab::cab &c, const std::string &fn) {
  const auto &file = find_file(c, fn);
  const auto &fld = c.folders[file.folder];
  if (fld.compress != 1)
    throw std::runtime_error("Only MSZIP is supported");

  cab::folder_deflater fdef{fld};
  for (auto i = 0U; i < file.unc_offset; i++) {
    if (!fdef.next())
      throw std::runtime_error("Folder data is smaller than file offset");
  }

  for (int i = 0; i < file.unc_size; i++) {
    auto n = fdef.next();
    if (!n)
      throw std::runtime_error("Failed to read stuff");
    std::cout << (char)*n;
  }
}

void try_main(int argc, char **argv) {
  if (argc != 3)
    throw std::runtime_error("Usage: cab-xt <cab> <file-to-extract>");

  std::ifstream f{argv[1]};
  if (!f)
    throw std::runtime_error("Could not open input file");

  xt(cab::read(f.rdbuf()), argv[2]);
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
