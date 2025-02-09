#pragma leco tool
#include <array>
#include <fstream>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string_view>

import cdf;
import msi;

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
struct pset {
  uint32_t size;
  uint32_t num_props;
};
struct pid_ofs {
  uint32_t pid;
  uint32_t offs;
};
struct typed_prop_val {
  uint16_t type;
  uint16_t padding;
};

void try_main(int argc, char **argv) {
  if (argc != 2)
    throw std::runtime_error("Missing filename");

  std::ifstream f{argv[1]};
  if (!f)
    throw std::runtime_error("Could not open input file");

  auto t = cdf::read(f.rdbuf());
  t.visit_tree([&](auto e) {
    const auto &b = e->entry();
    std::cout << msi::decode_name(b) << " secid:" << b.secid_first
              << " sz:" << b.stream_size << "\n";
    return true;
  });

  t.visit_tree([&](auto e) {
    if (e->entry().name == cdf::dir_name_t{u"\5SummaryInformation"}) {
      auto buf = t.read_stream(e->entry());

      constexpr const guid fmtid_summary_information = {
          0xE0, 0x85, 0x9F, 0xF2, 0xF9, 0x4F, 0x68, 0x10,
          0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9};

      auto pss = (pset_str *)buf.data();
      if (pss->byte_order != 0xFFFE)
        throw std::runtime_error("Invalid signature in summary information");
      if (pss->version != 0)
        throw std::runtime_error("Expection version 0 property set");
      if (pss->num_psets != 1)
        throw std::runtime_error("Only one property set is supported");
      if (pss->fmtid0 != fmtid_summary_information)
        throw std::runtime_error("Invalid format");

      auto ps_start = buf.data() + pss->offset0;
      auto ps = (pset *)ps_start;
      auto pifs = (pid_ofs *)(ps + 1);
      for (auto i = 0; i < ps->num_props; i++) {
        auto prop = (typed_prop_val *)(ps_start + pifs[i].offs);
        if (prop->padding != 0)
          throw std::runtime_error("Invalid padding");

        std::cout << pifs[i].pid << " ";

        switch (prop->type) {
        case 2:
          std::cout << *(uint16_t *)(prop + 1);
          break;
        case 3:
          std::cout << *(uint32_t *)(prop + 1);
          break;
        case 30:
          std::cout << std::string_view{(char *)(prop + 2),
                                        *(uint32_t *)(prop + 1)};
          break;
        default:
          std::cout << "(type = " << prop->type << ")";
        }
        std::cout << "\n";
      }

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
