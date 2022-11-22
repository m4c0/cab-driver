#include "ecow.hpp"

using namespace ecow;

int main(int argc, char **argv) {
  seq all{"all"};

  all.add_unit<exe>("msi-dump")->add_unit<>("msi-dump");

  return run_main(all, argc, argv);
}
