module;
#include <optional>
#include <span>
#include <vector>

export module cab:deflater;
import :pods;
import bitstream;
import deflate;
import yoyo;

namespace cab {
export class folder_deflater {
  zipline::deflater m_deflater{};
  unsigned m_cur_data{};
  const folder &m_folder;
  yoyo::span_reader m_reader;
  zipline::bitstream m_bits;

public:
  explicit folder_deflater(const folder &fld)
      : m_folder{fld}, m_reader{fld.datas.at(0).compressed}, m_bits{&m_reader} {
    if (m_reader.read_u16() != std::optional<uint16_t>{0x4B43})
      throw std::runtime_error("Invalid MSZIP signature");

    m_deflater.set_next_block(&m_bits);
  }

  std::optional<uint8_t> next() {
    auto res = m_deflater.next();
    if (res)
      return res;

    if (!m_deflater.last_block()) {
      m_deflater.set_next_block(&m_bits);
      return m_deflater.next();
    }

    m_cur_data++;
    if (m_cur_data == m_folder.datas.size()) {
      return std::nullopt;
    }

    m_reader = yoyo::span_reader{m_folder.datas.at(m_cur_data).compressed};
    if (m_reader.read_u16() != std::optional<uint16_t>{0x4B43})
      throw std::runtime_error("Invalid MSZIP signature");

    m_bits = zipline::bitstream{&m_reader};
    m_deflater.set_next_block(&m_bits);
    return m_deflater.next();
  }
};
} // namespace cab
