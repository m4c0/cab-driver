#include "ecow.hpp"

using namespace ecow;

int main(int argc, char **argv) {
  seq all{"all"};

  auto m = all.add_unit<mod>("msi");
  m->add_part("pods");
  m->add_part("reader");
  m->add_part("treenode");

  auto e = all.add_unit<exe>("msi-dump");
  e->add_ref(m);
  e->add_unit<>("msi-dump");

  return run_main(all, argc, argv);
}
