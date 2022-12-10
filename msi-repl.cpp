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
    msi::dbmeta m{t};
    for (const auto &tbl : m.tables()) {
      res << tbl << "\n";
    }
  } else if (cmd.substr(0, 2) == "c ") {
    msi::dbmeta m{t};
    for (const auto &d : m.columns(cmd.substr(2))) {
      res << (d.meta.s.key ? "K" : ".");
      res << (d.meta.s.nullable ? "N" : ".");
      res << "\t";
      switch (d.meta.s.type) {
      case 1:
        res << "Int32";
        break;
      case 5:
        res << "Int16";
        break;
      case 9:
        res << "Blob";
        break;
      case 13:
        res << "Str(" << d.meta.s.len << ")";
        break;
      default:
        res << d.meta.s.type << "-" << std::hex << d.meta.s.len << std::dec;
        break;
      }
      res << "\t" << d.name << "\n";
    }
  } else if (cmd.substr(0, 2) == "d ") {
    msi::dbmeta m{t};

    const auto fn = cmd.substr(2);
    const auto kfn = "_" + fn;
    const auto cols = m.columns(fn);

    struct colpair {
      unsigned offset;
      decltype(cols[0]) col;
    };
    std::vector<colpair> cp;
    auto row_size = 0U;
    auto row_incr = 0U;
    for (const auto &c : cols) {
      cp.push_back({row_size, c});
      switch (c.meta.s.type) {
      case 1:
        row_size += 4;
        break;
      case 5:
        row_size += 2;
        break;
      case 13:
      case 15:
        row_size += 2;
        break;
      default:
        res << "Unsupported column type: " << c.meta.s.type << "\n";
        return res.str();
      }
      if (row_incr == 0)
        row_incr = row_size;
    }

    t.visit_tree([&](auto e) {
      if (e->name() != kfn)
        return true;

      auto raw = t.read_stream(e->entry());
      const auto row_count = raw.size() / row_size;

      for (const auto &c : cp) {
        res << c.col.name << "\t";
      }
      res << "\n";

      for (auto i = 0U; i < row_count; i++) {
        const auto ri = row_incr * i;
        for (const auto &c : cp) {
          auto *ptr = raw.data() + c.offset * row_count + ri;
          switch (c.col.meta.s.type) {
          case 1:
            res << (*(uint32_t *)ptr & 0x7FFFFFFFU);
            break;
          case 5:
            res << (*(uint16_t *)ptr & 0x7FFFU);
            break;
          case 13:
          case 15: {
            const auto v = *(uint16_t *)ptr;
            if (v)
              res << *(m.string(v));
            break;
          }
          default:
            break;
          }
          res << "\t";
        }
        res << "\n";
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
