#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

import cab;

void try_main(int argc, char **argv) {
  if (argc != 3)
    throw std::runtime_error("Usage: cab-xt <cab> <file-to-extract>");

  std::ifstream f{argv[1]};
  if (!f)
    throw std::runtime_error("Could not open input file");

  cab::extract(std::cout, f.rdbuf(), argv[2]);
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
