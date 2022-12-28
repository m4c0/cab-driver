module;
#include <cassert>
#include <cstdint>
#include <optional>

export module msi:cell;

namespace msi {
[[nodiscard]] static constexpr std::optional<signed>
u16_cell(uint16_t v) noexcept {
  if (v == 0)
    return {};
  assert((v & 0x8000U) != 0);
  return v & 0x7FFFU;
}
static_assert(!u16_cell(0));
static_assert(*u16_cell(0x8000) == 0);
static_assert(*u16_cell(0x8005) == 5);
static_assert(*u16_cell(0x8FFF) == 0xFFF);
// TODO: find what should be done for the interval 0x0001~0x7FFF

[[nodiscard]] static constexpr std::optional<signed>
string_cell(uint16_t v) noexcept {
  if (v == 0)
    return {};
  assert((v & 0x8000U) == 0);
  return v;
}
static_assert(!string_cell(0));
static_assert(*string_cell(0x0001) == 1);
static_assert(*string_cell(0x0005) == 5);
static_assert(*string_cell(0x0FFF) == 0xFFF);
// TODO: find what should be done for the interval 0x8001~0x8FFF

class cell {
  // TODO: optional
  signed m_data{};
  bool m_string{};

public:
  constexpr cell() = default;
  cell(unsigned type, uint8_t *ptr) {
    switch (type) {
    case 1:
      // TODO: find what's the rule here
      //  Media/File/MsiFileHash uses DoubleInteger, but their numbers are not
      //  making sense
      m_data = *(int32_t *)ptr;
      break;
    case 5:
      m_data = *u16_cell(*(int16_t *)ptr);
      break;
    case 13:
    case 15:
      m_data = string_cell(*(uint16_t *)ptr).value_or(0);
      m_string = true;
      break;
    default:
      break;
    }
  }

  [[nodiscard]] constexpr auto operator*() const noexcept { return m_data; }
  [[nodiscard]] constexpr bool is_string() const noexcept { return m_string; }
};
} // namespace msi
