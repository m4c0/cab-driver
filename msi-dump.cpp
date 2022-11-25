#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>

import msi;

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

  tables t{f.rdbuf(), h};

  t.visit_tree([&](auto e) {
    const auto &b = e->entry();
    std::cout << e->name() << ":" << e->name().size()
              << " secid:" << b.secid_first << " sz:" << b.stream_size << "\n";
    return true;
  });

  t.visit_tree([&](auto e) {
    if (e->name() == "5 SummaryInformation") {
      const auto &b = e->entry();
      auto secid = b.secid_first;
      auto sz = b.stream_size;

      // auto buf = t.read_stream(secid, sz);
      // for (int i = 0; i < 32; i++) std::cout << (int)buf[i] << " "; std::cout
      // << "\n";
      return false;
    }

    return true;
  });
}

int main(int argc, char **argv) {
  try {
    try_main(argc, argv);
    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}
