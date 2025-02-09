module;
#include <cassert>
#include <optional>
#include <span>
#include <vector>

export module cab:deflater;
import :pods;
import yoyo;
import zipline;

namespace cab {
export class folder_deflater {
  zipline::deflater m_deflater{};
  unsigned m_cur_data{};
  unsigned m_total_read{};
  const folder &m_folder;
  yoyo::span_reader m_reader;
  zipline::bitstream m_bits;

  [[nodiscard]] constexpr const auto data() const {
    return m_folder.datas.at(m_cur_data);
  }

public:
  explicit folder_deflater(const folder &fld)
      : m_folder{fld}, m_reader{data().compressed}, m_bits{&m_reader} {
    if (m_reader.read_u16() != std::optional<uint16_t>{0x4B43})
      throw std::runtime_error("Invalid MSZIP signature");

    m_deflater.set_next_block(&m_bits);
  }

  std::optional<uint8_t> next() {
    auto res = m_deflater.next();
    if (res) {
      m_total_read++;
      return res;
    }

    if (!m_deflater.last_block()) {
      m_deflater.set_next_block(&m_bits);
      m_total_read++;
      return m_deflater.next();
    }

    assert(m_total_read == data().uncomp_size);
    m_total_read = 0;

    m_cur_data++;
    if (m_cur_data == m_folder.datas.size()) {
      return std::nullopt;
    }

    m_reader = yoyo::span_reader{data().compressed};
    if (m_reader.read_u16() != std::optional<uint16_t>{0x4B43})
      throw std::runtime_error("Invalid MSZIP signature");

    m_total_read = 1;
    m_bits = zipline::bitstream{&m_reader};
    m_deflater.set_next_block(&m_bits);
    return m_deflater.next();
  }
};
} // namespace cab
