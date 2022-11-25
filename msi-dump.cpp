import msi;

#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <span>
#include <sstream>
#include <string_view>
#include <vector>

using namespace msi;

void try_main(int argc, char **argv) {
  if (argc != 2)
    throw std::runtime_error("Missing filename");

  std::ifstream f{argv[1]};
  if (!f)
    throw std::runtime_error("Could not open input file");

  header h;

  if (!f.rdbuf()->sgetn((char *)&h, sizeof(h)))
    throw std::runtime_error("Failed to read header");

  constexpr const auto signature =
      std::array<uint8_t, 8>{0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1};

  if (h.doc_file_id != signature)
    throw std::runtime_error("Invalid signature");

  if (h.byte_order != 0xFFFE)
    throw std::runtime_error("Invalid byte order");

  const auto sec_size = 1U << h.pot_sec_size;
  const auto mini_sec_size = 1U << h.pot_minisec_size;

  if (h.secid_msat != -2)
    throw std::runtime_error("Extended MSAT not supported");

  reader r{f.rdbuf(), sec_size};

  // Build SAT from MSAT

  std::vector<secid_t> sat;
  for (auto s : h.first_sect_fats) {
    r.read(s, sat);
  }
  if (sat[0] != -3)
    throw std::runtime_error("Invalid SAT sector");

  // Read dir from SAT

  std::vector<dir_entry> root_dir;
  auto secid = h.secid_dir;
  while (secid >= 0) {
    r.read(secid, root_dir);
    secid = sat[secid];
  }

  // Read SSAT from SAT

  treenode tree{root_dir};

  std::vector<secid_t> ssat;
  secid = h.secid_short_sat;
  while (secid >= 0) {
    r.read(secid, ssat);
    secid = sat[secid];
  }

  tree.visit([&](auto e) {
    const auto &b = e->entry();
    std::cout << e->name() << ":" << e->name().size()
              << " secid:" << b.secid_first << " sz:" << b.stream_size << "\n";
    return true;
  });

  tree.visit([&](auto e) {
    if (e->name() == "5 SummaryInformation") {
      const auto &b = e->entry();
      auto secid = b.secid_first;
      auto sz = b.stream_size;

      std::vector<uint8_t> buf;
      buf.reserve(sz);
      if (sz < h.min_stream_size) {
        // r.read(buf, ssat);
      } else {
        // r.read(buf, sat);
      }
      std::cout << secid << " " << sz << "\n";
      return false;
    }

    return true;
  });
}

int main(int argc, char **argv) { try_main(argc, argv); }
