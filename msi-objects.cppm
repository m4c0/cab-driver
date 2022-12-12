module;
#include <filesystem>
#include <map>
#include <string>

export module msi:objects;
import :dbmeta;

namespace msi {
struct raw_dir {
  unsigned parent;
  unsigned name;
};

auto strip_long_path(std::string_view v) {
  const auto p = v.find('|');
  return (p == v.npos) ? v : v.substr(p + 1);
}

auto build_path(msi::dbmeta &m, const auto &cache, unsigned key) {
  auto v = cache.find(key);
  if (v == cache.end()) {
    return std::filesystem::path{"."};
  }
  auto name = strip_long_path(*m.string(v->second.name));
  auto parent_id = v->second.parent;
  if (parent_id == 0) {
    return std::filesystem::path{name};
  }

  auto parent = build_path(m, cache, parent_id);
  return parent / name;
};

export [[nodiscard]] auto read_directories(msi::dbmeta &m) {
  auto tbl = m.table("Directory");

  std::map<unsigned, raw_dir> cache;
  for (const auto &d : tbl) {
    const auto id = d.at("Directory").data;
    const auto parent = d.at("Directory_Parent").data;
    const auto name = d.at("DefaultDir").data;

    cache.emplace(id, raw_dir{parent, name});
  }

  std::map<unsigned, std::filesystem::path> res;
  for (auto &[k, v] : cache) {
    auto p = std::filesystem::relative(build_path(m, cache, k));
    res.emplace(k, p);
  }
  return res;
}
} // namespace msi
