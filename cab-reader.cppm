module;
#include <streambuf>

export module cab:reader;
import :pods;

namespace cab {
header read_header(std::streambuf *f) {
  header hdr;
  if (!f->sgetn((char *)&hdr, sizeof(hdr)))
    throw std::runtime_error("Failed to read header");

  if (hdr.signature != 'FCSM')
    throw std::runtime_error("Signature doesn't match");

  if (hdr.version != 0x0103)
    throw std::runtime_error("Unsupported version");

  if (hdr.set_id != 0 || hdr.id_cab_in_set != 0)
    throw std::runtime_error("Sets are not supported");

  switch (hdr.flags) {
  case 0x0:
    hdr.header_extra = 0;
    hdr.folder_extra = 0;
    hdr.data_extra = 0;
    f->pubseekoff(-4, std::ios::cur);
    break;
  case 0x4:
    f->pubseekoff(hdr.header_extra, std::ios::cur);
    break;
  default:
    throw std::runtime_error("Sets are not supported");
  }

  return hdr;
}
folder read_next_folder(std::streambuf *f, const header &hdr) {
  folder fld;
  if (!f->sgetn((char *)&fld, sizeof(folder_data)))
    throw std::runtime_error("Failed to read folder");

  if (fld.compress > 1)
    throw std::runtime_error("Unsupported compression type");

  f->pubseekoff(hdr.folder_extra, std::ios::cur);

  return fld;
}
file read_next_file(std::streambuf *f, const header &hdr) {
  file fl;
  if (!f->sgetn((char *)&fl, sizeof(file_data)))
    throw std::runtime_error("Failed to read file");

  if (fl.folder >= hdr.num_folders)
    throw std::runtime_error("Invalid folder");

  char c;
  fl.name = "";
  while ((c = f->sbumpc()) != '\0')
    fl.name += c;

  return fl;
}
data read_next_data(std::streambuf *f, const header &hdr) {
  data d;
  if (!f->sgetn((char *)&d, sizeof(data_meta)))
    throw std::runtime_error("Failed to read data");

  f->pubseekoff(hdr.data_extra, std::ios::cur);

  d.compressed.resize(d.comp_size);
  if (!f->sgetn((char *)d.compressed.data(), d.compressed.size()))
    throw std::runtime_error("Failed to read compressed data");

  return d;
}

export cab read(std::streambuf *f) {
  cab res;
  auto &hdr = res.hdr = read_header(f);

  for (auto i = 0U; i < hdr.num_folders; i++) {
    auto &fld = res.folders.emplace_back(read_next_folder(f, hdr));

    f->pubseekpos(fld.ofs_first_data, std::ios::cur);
    for (auto j = 0U; j < fld.num_data; j++) {
      fld.datas.emplace_back(read_next_data(f, hdr));
    }
  }

  f->pubseekpos(hdr.ofs_first_file);
  for (auto i = 0U; i < hdr.num_files; i++) {
    res.files.emplace_back(read_next_file(f, hdr));
  }

  return res;
}

} // namespace cab
