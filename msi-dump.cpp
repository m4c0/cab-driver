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

  auto t = msi::read(f.rdbuf());
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

      auto buf = t.read_stream(secid, sz);
      std::cout << buf.size() << "\n";
      for (int i = 0; i < buf.size(); i++) {
        auto c = (int)buf[i];
        if (c < 32 || c > 127) {
          std::cout << (int)buf[i] << " ";
        } else {
          std::cout << buf[i];
        }
      }
      std::cout << "\n";
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
