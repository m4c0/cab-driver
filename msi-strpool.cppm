module;
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

export module msi:strpool;

import :name;
import cdf;

namespace msi {
struct strpool_entry {
  uint16_t size;
  uint16_t ref_count;
};

export class strpool {
  std::vector<std::string> m_strs;

public:
  explicit strpool(cdf::tables &t) {
    std::vector<uint8_t> data;
    t.visit_tree([&](auto e) {
      if (msi::decode_name(e->entry()) != "__StringData")
        return true;
      data = t.read_stream(e->entry());
      return false;
    });
    t.visit_tree([&](auto e) {
      if (msi::decode_name(e->entry()) != "__StringPool")
        return true;

      auto raw = t.read_stream(e->entry());
      std::span<uint32_t> pool{(uint32_t *)raw.data(),
                               raw.size() / sizeof(uint32_t)};
      if (pool[0] != 0xfde9) {
        throw std::runtime_error("String pool is not UTF-8");
      }

      std::string_view str{(char *)data.data(), data.size()};
      for (auto dt : pool.subspan(1)) {
        auto sz = dt & 0xFFFF;
        if (sz > str.size()) {
          throw std::runtime_error("String pool is shorter than needed");
        }
        m_strs.emplace_back(str.substr(0, sz));
        str = str.substr(sz);
      }

      return false;
    });
  }

  [[nodiscard]] const auto &operator*() const noexcept { return m_strs; }

  [[nodiscard]] std::optional<std::string_view> operator[](unsigned idx) const {
    if (idx == 0)
      return std::nullopt;
    if (idx > m_strs.size())
      throw std::runtime_error("Invalid access to string pool");
    return {m_strs[idx - 1]};
  }
};
} // namespace msi
