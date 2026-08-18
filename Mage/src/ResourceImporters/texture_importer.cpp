#include "texture_importer.hpp"

#include <fstream>

#include "io_helpers.hpp"
#include "Util.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::mage::TextureImporter>("Texture Importer")
    .REFLECT_REGISTER_RESOURCE_IMPORTER_CTOR
    .property("Import Settings", &sorcery::mage::TextureImporter::settings_);
}


namespace sorcery::mage {
auto TextureImporter::GetSupportedFileExtensions(
  std::pmr::vector<std::string>& out
) -> void {
  out.emplace_back(kDdsFileExt);
  out.emplace_back(kHdrFileExt);
  out.emplace_back(kTgaFileExt);
  std::ranges::transform(kWicFileExts, std::back_inserter(out), [](std::string_view const sv) {
    return std::string{sv};
  });
}


auto TextureImporter::Import(
  std::filesystem::path const& src,
  std::vector<ResourceImportResult>& results
) -> bool {
  std::vector<std::byte> file_bytes;

  if (!ReadBinaryFile(src, file_bytes)) {
    return false;
  }

  auto result{
    ImportTexture(TextureImportSource{.file_bytes = file_bytes, .ext = src.extension().u8string()}, settings_)
  };

  if (!result) {
    return false;
  }

  results.emplace_back(result->payload_kind, result->runtime_type,
    std::string{ToUntypedStdSv(src.filename().u8string())}, std::move(result->bytes));
  return true;
}
}
