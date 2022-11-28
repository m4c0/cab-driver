#include <array>
#include <fstream>
#include <iostream>
#include <span>
#include <stdexcept>

import msi;

using namespace msi;

using guid = std::array<uint8_t, 16>;
struct pset_str {
  uint16_t byte_order;
  uint16_t version;
  uint32_t sys_id;
  guid clsid;
  uint32_t num_psets;
  guid fmtid0;
  uint32_t offset0;
};

void try_main(int argc, char **argv) {
  if (argc != 2)
    throw std::runtime_error("Missing filename");

  std::ifstream f{argv[1]};
  if (!f)
    throw std::runtime_error("Could not open input file");

  auto t = msi::read(f.rdbuf());
  t.visit_tree([&](auto e) {
    const auto &b = e->entry();
    std::cout << e->name() << ":" << e->name().size()
              << " secid:" << b.secid_first << " sz:" << b.stream_size << "\n";
    return true;
  });

  t.visit_tree([&](auto e) {
    if (e->name() == "<05>SummaryInformation") {
      auto buf = t.read_stream(e->entry());
      std::span<pset_str> spn{(pset_str *)buf.data(), buf.size()};

      constexpr const guid fmtid_summary_information = {
          0xE0, 0x85, 0x9F, 0xF2, 0xF9, 0x4F, 0x68, 0x10,
          0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9};

      auto pss = spn[0];
      if (pss.byte_order != 0xFFFE)
        throw std::runtime_error("Invalid signature in summary information");
      if (pss.version != 0)
        throw std::runtime_error("Expection version 0 property set");
      if (pss.num_psets != 1)
        throw std::runtime_error("Only one property set is supported");
      if (pss.fmtid0 != fmtid_summary_information)
        throw std::runtime_error("Invalid format");

      std::cout << "o0" << pss.offset0 << "\n";

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
    std::cerr << "Failure: " << e.what() << std::endl;
    return 1;
  }
}
