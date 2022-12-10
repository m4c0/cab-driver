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
    msi::strpool pool{t};
    for (const auto &s : *pool) {
      res << s << "\n";
    }
    res << "Total: " << (*pool).size() << "\n";
  } else if (cmd == "tables") {
    msi::strpool pool{t};
    t.visit_tree([&](auto e) {
      if (e->name() != "__Tables")
        return true;

      auto raw = t.read_stream(e->entry());
      std::span<uint16_t> list{(uint16_t *)raw.data(),
                               raw.size() / sizeof(uint16_t)};
      unsigned i = 0;
      for (const auto &idx : list) {
        auto ostr = pool[idx];
        if (!ostr)
          continue;

        res << *ostr << "\n";
        i++;
      }
      res << "Total: " << i << "\n";
      return false;
    });
  } else if (cmd == "test") {
    msi::strpool pool{t};
    t.visit_tree([&](auto e) {
      if (e->name() != "__Columns")
        return true;

      constexpr const auto row_size = sizeof(uint16_t) * 4;

      auto raw = t.read_stream(e->entry());
      const auto row_count = raw.size() / row_size;

      struct col {
        std::string table;
        uint16_t index;
        std::string name;
        uint16_t meta;
      };
      std::vector<col> data{};
      data.resize(row_count);

      std::span<uint16_t> tables{(uint16_t *)raw.data(), row_count};
      for (auto i = 0; i < row_count; i++)
        data[i].table = *(pool[tables[i]]);

      std::span<uint16_t> indices{tables.end(), row_count};
      for (auto i = 0; i < row_count; i++)
        data[i].index = indices[i] & 0x7FFFU;

      std::span<uint16_t> names{indices.end(), row_count};
      for (auto i = 0; i < row_count; i++)
        data[i].name = *(pool[names[i]]);

      std::span<uint16_t> metas{names.end(), row_count};
      for (auto i = 0; i < row_count; i++)
        data[i].meta = metas[i] & 0x7FFFU;

      for (const auto &d : data) {
        res << d.table << "\t" << d.index << "\t";
        res << (d.meta & 0x2000 ? "K" : ".");
        res << (d.meta & 0x1000 ? "N" : ".");
        res << "\t" << d.name << "\n";
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
