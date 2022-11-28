#include <fstream>
#include <iostream>
#include <optional>
#include <string>

import msi;

using namespace msi;

std::optional<std::string> read_cmd() {
  std::cout << "> ";
  std::string res;
  std::cin >> res;
  return std::cin ? std::optional{res} : std::nullopt;
}

std::string eval_cmd(const std::string &cmd) { return cmd; }

void print_result(const std::string &res) { std::cout << res << std::endl; }

void try_main(int argc, char **argv) {
  if (argc != 2)
    throw std::runtime_error("Missing filename");

  std::ifstream f{argv[1]};
  if (!f)
    throw std::runtime_error("Could not open input file");

  while (auto cmd = read_cmd()) {
    print_result(eval_cmd(*cmd));
  }
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
