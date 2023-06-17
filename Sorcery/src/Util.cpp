#include "Util.hpp"

#include <cctype>
#include <format>
#include <stdexcept>


namespace sorcery {
auto Contains(std::string_view const src, std::string_view const target) -> bool {
  if (target.empty()) {
    return true;
  }

  if (src.empty() || target.size() > src.size()) {
    return false;
  }

  for (std::size_t i{ 0 }; i < src.size() - target.size() + 1; i++) {
    auto match{ true };

    for (std::size_t j{ 0 }; j < target.size(); j++) {
      if (std::tolower(src[i + j]) != std::tolower(target[j])) {
        match = false;
      }
    }

    if (match) {
      return true;
    }
  }

  return false;
}


auto CalculateNormals(std::span<Vector3 const> const positions, std::span<unsigned const> const indices, std::vector<Vector3>& out) -> std::vector<Vector3>& {
  if (indices.size() % 3 != 0) {
    throw std::runtime_error{ std::format("Cannot calculate normals because the number of indices ({}) is not divisible by 3. The calculation is only supported over triangle lists.", indices.size()) };
  }

  out.resize(positions.size());

  for (int i = 0; i < std::ssize(indices); i += 3) {
    Vector3 const& vertex1{ positions[indices[i]] };
    Vector3 const& vertex2{ positions[indices[i + 1]] };
    Vector3 const& vertex3{ positions[indices[i + 2]] };

    Vector3 const edge1{ Normalize(vertex2 - vertex1) };
    Vector3 const edge2{ Normalize(vertex3 - vertex1) };
    Vector3 const normal{ Normalize(Cross(edge1, edge2)) };

    for (int j = 0; j < 3; j++) {
      out[indices[i + j]] = normal;
    }
  }

  return out;
}


auto CalculateTangents(std::span<Vector3 const> const positions, std::span<Vector2 const> const uvs, std::span<unsigned const> const indices, std::vector<Vector3>& out) -> void {
  if (indices.size() % 3 != 0) {
    throw std::runtime_error{ std::format("Cannot calculate tangents because the number of indices ({}) is not divisible by 3. The calculation is only supported over triangle lists.", indices.size()) };
  }

  out.resize(positions.size());

  for (int i = 0; i < std::ssize(indices); i += 3) {
    Vector3 const& vertex1{ positions[indices[i]] };
    Vector3 const& vertex2{ positions[indices[i + 1]] };
    Vector3 const& vertex3{ positions[indices[i + 2]] };

    Vector3 const edge1{ Normalize(vertex2 - vertex1) };
    Vector3 const edge2{ Normalize(vertex3 - vertex1) };

    Vector2 const& uv1{ uvs[indices[i]] };
    Vector2 const& uv2{ uvs[indices[i + 1]] };
    Vector2 const& uv3{ uvs[indices[i + 2]] };

    Vector2 const deltaUv1{ uv2 - uv1 };
    Vector2 const deltaUv2{ uv3 - uv1 };

    float const f{ 1.0f / (deltaUv1[0] * deltaUv2[1] - deltaUv1[1] * deltaUv2[0]) };

    Vector3 tangent;
    for (int j = 0; j < 3; j++) {
      tangent[j] = f * (deltaUv2[1] * edge1[j] - deltaUv1[1] * edge2[j]);
    }

    for (int j = 0; j < 3; j++) {
      out[indices[i + j]] = tangent;
    }
  }
}


auto GenerateUniquePath(std::filesystem::path const& absolutePath) -> std::filesystem::path {
  std::string const originalStem{ absolutePath.stem().string() };
  std::filesystem::path const ext{ absolutePath.extension() };
  std::filesystem::path const parentDir{ absolutePath.parent_path() };

  std::filesystem::path ret{ absolutePath };
  std::size_t fileNameIndex{ 1 };

  while (exists(ret)) {
    ret = (parentDir / originalStem += std::to_string(fileNameIndex)) += ext;
    fileNameIndex += 1;
  }

  return ret;
}


auto Join(std::span<std::string const> const strings, std::string const& delim) -> std::string {
  std::string ret;

  for (std::size_t i = 0; i < std::size(strings); i++) {
    ret += strings[i];

    if (i != std::size(strings) - 1) {
      ret += delim;
    }
  }

  return ret;
}


auto ToLower(std::string_view const str) -> std::string {
  std::string ret{ str };
  std::ranges::transform(ret, std::begin(ret), [](char const c) {
    return static_cast<char>(std::tolower(c));
  });
  return ret;
}
}
