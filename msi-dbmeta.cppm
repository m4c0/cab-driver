module;
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

export module msi:dbmeta;

import :tables;
import :strpool;

namespace msi {
struct dbcolumn {
  std::string table;
  uint16_t index;
  std::string name;
  union {
    struct {
      unsigned len : 8;
      unsigned type : 4;
      bool nullable : 1;
      bool key : 1;
    } s;
    uint16_t u16;
  } meta;
};

export class dbmeta {
  msi::strpool m_pool;
  msi::tables &m_t;
  std::vector<std::string_view> m_tables;
  std::vector<dbcolumn> m_columns;

public:
  explicit dbmeta(tables &t) : m_pool{t}, m_t{t} {
    t.visit_tree([&](auto e) {
      if (e->name() != "__Tables")
        return true;

      auto raw = t.read_stream(e->entry());
      std::span<uint16_t> list{(uint16_t *)raw.data(),
                               raw.size() / sizeof(uint16_t)};
      for (const auto &idx : list) {
        auto ostr = m_pool[idx];
        if (!ostr)
          continue;

        m_tables.push_back(*ostr);
      }
      return false;
    });

    t.visit_tree([&](auto e) {
      if (e->name() != "__Columns")
        return true;

      constexpr const auto row_size = sizeof(uint16_t) * 4;

      auto raw = t.read_stream(e->entry());
      const auto row_count = raw.size() / row_size;

      m_columns.resize(row_count);

      std::span<uint16_t> tables{(uint16_t *)raw.data(), row_count};
      for (auto i = 0; i < row_count; i++)
        m_columns[i].table = *(m_pool[tables[i]]);

      std::span<uint16_t> indices{tables.end(), row_count};
      for (auto i = 0; i < row_count; i++)
        m_columns[i].index = indices[i] & 0x7FFFU;

      std::span<uint16_t> names{indices.end(), row_count};
      for (auto i = 0; i < row_count; i++)
        m_columns[i].name = *(m_pool[names[i]]);

      std::span<uint16_t> metas{names.end(), row_count};
      for (auto i = 0; i < row_count; i++)
        m_columns[i].meta.u16 = metas[i] & 0x7FFFU;

      return false;
    });
  }

  [[nodiscard]] constexpr const auto &tables() const noexcept {
    return m_tables;
  }
  [[nodiscard]] auto columns(std::string_view tbl) const {
    std::vector<dbcolumn> res;
    std::copy_if(m_columns.begin(), m_columns.end(), std::back_inserter(res),
                 [tbl](const auto &c) { return c.table == tbl; });
    return res;
  }

  [[nodiscard]] auto string(unsigned idx) const { return m_pool[idx]; }
};

} // namespace msi
