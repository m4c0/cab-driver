#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

import msi;

using namespace msi;

std::optional<std::string> read_cmd() {
  std::cout << "> ";
  std::string res;
  std::cin >> res;
  return std::cin ? std::optional{res} : std::nullopt;
}

std::string eval_cmd(const auto &t, const std::string &cmd) {
  std::stringstream res;
  if (cmd == "list") {
    t.visit_tree([&](auto e) {
      const auto &b = e->entry();
      res << e->name() << " size:" << b.stream_size << "\n";
      return true;
    });
  } else {
    res << "Unknown command: " << cmd;
  }
  return res.str();
}

void print_result(const std::string &res) { std::cout << res << std::endl; }

void try_main(int argc, char **argv) {
  if (argc != 2)
    throw std::runtime_error("Missing filename");

  std::ifstream f{argv[1]};
  if (!f)
    throw std::runtime_error("Could not open input file");

  auto t = msi::read(f.rdbuf());

  while (auto cmd = read_cmd()) {
    print_result(eval_cmd(t, *cmd));
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
