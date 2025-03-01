#pragma leco tool
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <unistd.h>

import cab;
import cdf;
import msi;

std::optional<std::string> read_cmd() {
  if (isatty(0))
    std::cout << "> ";
  std::string res;
  return std::getline(std::cin, res) ? std::optional{res} : std::nullopt;
}

std::string eval_cmd(auto &t, std::filesystem::path in_file,
                     const std::string &cmd) {
  std::stringstream res;
  if (cmd == "list") {
    t.visit_tree([&](auto e) {
      const auto &b = e->entry();
      res << msi::decode_name(b) << " -- size:" << b.stream_size << "\n";
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
      case 15:
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
    const auto cols = m.columns(fn);

    for (const auto &c : cols) {
      res << c.name << "\t";
    }
    res << "\n" << std::hex;

    for (const auto &row : m.table(fn)) {
      for (const auto &c : cols) {
        const auto col = row.at(c.name);
        if (col.is_string()) {
          auto str = m.string(*col);
          if (str)
            res << *str;
        } else {
          res << *col;
        }
        res << "\t";
      }
      res << "\n";
    }
  } else if (cmd == "ls") {
    msi::dbmeta m{t};
    for (auto &[k, v] : msi::read_files(m)) {
      res << v.string() << "\n";
    }
  } else if (cmd == "xt") {
    msi::dbmeta m{t};
    std::map<std::string, std::filesystem::path> files;
    for (auto &[k, v] : msi::read_files(m)) {
      files.emplace(*m.string(k), v);
    }
    const auto dir = in_file.parent_path();
    for (auto md : msi::read_medias(m, dir)) {
      std::cout << md.string() << "\n";

      std::ifstream f{md};
      if (!f)
        throw std::runtime_error("Could not open cab");

      for (const auto &[fid, path] : files) {
        std::cout << "  " << path.string() << "\n";
        std::filesystem::create_directories(path.parent_path());

        std::ofstream o{path};
        if (!o)
          throw std::runtime_error("Could not open output file");

        try {
          cab::extract(o, f.rdbuf(), fid);
          files.erase(fid);
          break;
        } catch (...) {
        }
      }
    }
  } else {
    res << std::setfill('0') << std::hex;
    t.visit_tree([&](auto e) {
      if (msi::decode_name(e->entry()) != cmd)
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

  auto t = cdf::read(f.rdbuf());

  while (auto cmd = read_cmd()) {
    print_result(eval_cmd(t, argv[1], *cmd));
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
