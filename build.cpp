#include "../yoyo/build.hpp"
#include "../zipline/build.hpp"
#include "ecow.hpp"

using namespace ecow;

int main(int argc, char **argv) {
  seq all{"all"};

  auto cdf = all.add_unit<mod>("cdf");
  cdf->add_part("pods");
  cdf->add_part("reader");
  cdf->add_part("treenode");
  cdf->add_part("tables");

  auto cab = all.add_unit<mod>("cab");
  cab->add_part("pods");
  cab->add_part("reader");
  cab->add_part("deflater");
  cab->add_wsdep("zipline", zipline());
  cab->add_wsdep("yoyo", yoyo());

  auto m = all.add_unit<mod>("msi");
  m->add_part("name");
  m->add_part("strpool");
  m->add_part("cell");
  m->add_part("dbmeta");
  m->add_part("objects");
  m->add_ref(cdf);

  auto e = all.add_unit<exe>("msi-dump");
  e->add_ref(m);
  e->add_unit<>("msi-dump");

  auto repl = all.add_unit<exe>("msi-repl");
  repl->add_ref(m);
  repl->add_unit<>("msi-repl");

  auto cabl = all.add_unit<exe>("cab-list");
  cabl->add_unit<>("cab-list");
  cabl->add_ref(cab);

  auto cabx = all.add_unit<exe>("cab-xt");
  cabx->add_unit<>("cab-xt");
  cabx->add_ref(cab);

  return run_main(all, argc, argv);
}
