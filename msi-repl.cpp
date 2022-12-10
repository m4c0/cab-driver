#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <string_view>

import msi;

using namespace msi;

std::optional<std::string> read_cmd() {
  std::cout << "> ";
  std::string res;
  return std::getline(std::cin, res) ? std::optional{res} : std::nullopt;
}

std::string eval_cmd(auto &t, const std::string &cmd) {
  std::stringstream res;
  if (cmd == "list") {
    t.visit_tree([&](auto e) {
      const auto &b = e->entry();
      res << e->name() << " -- size:" << b.stream_size << "\n";
      return true;
    });
  } else if (cmd == "strings") {
    std::vector<uint8_t> data;
    t.visit_tree([&](auto e) {
      if (e->name() != "__StringData")
        return true;
      data = t.read_stream(e->entry());
      return false;
    });
    t.visit_tree([&](auto e) {
      if (e->name() != "__StringPool")
        return true;

      auto raw = t.read_stream(e->entry());
      std::span<uint32_t> pool{(uint32_t *)raw.data() + 1, raw.size() / 2};
      std::string_view str{(char *)data.data(), data.size()};
      for (auto dt : pool) {
        auto sz = dt & 0xFFFF;
        if (sz > str.size()) {
          res << "Failure!!!!\n";
          break;
        }
        res << str.substr(0, sz) << "\n";
        str = str.substr(sz);
      }

      return false;
    });
  } else {
    res << std::setfill('0') << std::hex;
    t.visit_tree([&](auto e) {
      if (e->name() != cmd)
        return true;

      unsigned i = 0;
      for (auto c : t.read_stream(e->entry())) {
        res << std::setw(2) << (unsigned)c;
        switch (++i % 16) {
        case 0:
          res << '\n';
          break;
        case 8:
          res << "  ";
          break;
        default:
          res << ' ';
          break;
        }
      }
      return false;
    });
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
