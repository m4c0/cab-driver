module;
#include <optional>
#include <ostream>
#include <string>

export module cab;
export import :pods;
export import :reader;
export import :deflater;

namespace cab {
export const auto &find_file(const cab &c, const std::string &fn) {
  for (auto i = 0U; i < c.hdr.num_files; i++) {
    const auto &fl = c.files[i];
    if (fl.name != fn)
      continue;

    return fl;
  }

  throw std::runtime_error("Could not find target file");
}
export void extract(std::ostream &out, const cab &c, const std::string &fn) {
  const auto &file = find_file(c, fn);
  const auto &fld = c.folders[file.folder];
  if (fld.compress != 1)
    throw std::runtime_error("Only MSZIP is supported");

  folder_deflater fdef{fld};
  for (auto i = 0U; i < file.unc_offset; i++) {
    if (!fdef.next())
      throw std::runtime_error("Folder data is smaller than file offset");
  }

  for (int i = 0; i < file.unc_size; i++) {
    auto n = fdef.next();
    if (!n)
      throw std::runtime_error("Failed to read stuff");
    out.put((char)*n);
  }
}
export void extract(std::ostream &out, std::streambuf *rdbuf,
                    const std::string &fn) {
  extract(out, read(rdbuf), fn);
}

} // namespace cab
