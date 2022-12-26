module;
#include <cstdint>

export module msi:cell;

namespace msi {
class cell {
  unsigned m_data{};
  bool m_string{};

public:
  constexpr cell() = default;
  cell(unsigned type, uint8_t *ptr) {
    switch (type) {
    case 1:
      m_data = *(uint32_t *)ptr & 0x7FFFFFFFU;
      break;
    case 5:
      m_data = *(uint16_t *)ptr & 0x7FFFU;
      break;
    case 13:
    case 15:
      m_data = *(uint16_t *)ptr;
      m_string = true;
      break;
    default:
      break;
    }
  }

  [[nodiscard]] constexpr auto operator*() const noexcept { return m_data; }
};
} // namespace msi
