#pragma once

#include "Types.hpp"

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>


namespace leopph
{
	enum class ShaderStage : u8
	{
		Vertex = 1,
		Geometry = 2,
		Fragment = 3
	};

	// Information about a shader stage in binary format
	struct ShaderStageBinaryInfo
	{
		std::string_view entryPoint;
		std::span<u8 const> binary;
	};

	// Information about a shader stage in source file format
	struct ShaderStageSourceFileInfo
	{
		std::filesystem::path absolutePath;
		std::string source;
	};

	// Information about a shader program in source string format
	struct ShaderProgramSourceInfo
	{
		std::optional<std::string_view> vertex;
		std::optional<std::string_view> geometry;
		std::optional<std::string_view> fragment;
	};

	// Information about a shader program in source file format
	struct ShaderProgramSourceFileInfo
	{
		std::optional<ShaderStageSourceFileInfo> vertex;
		std::optional<ShaderStageSourceFileInfo> geometry;
		std::optional<ShaderStageSourceFileInfo> fragment;
	};

	// Information about a shader program in binary format
	struct ShaderProgramBinaryInfo
	{
		std::optional<ShaderStageBinaryInfo> vertex;
		std::optional<ShaderStageBinaryInfo> geometry;
		std::optional<ShaderStageBinaryInfo> fragment;
	};

	// Information about a shader program binary suitable for consumption
	struct ShaderProgramCachedBinaryInputInfo
	{
		u32 format;
		std::span<u8 const> binary;
	};

	// Information about a shader binary suitable for modification
	struct ShaderProgramCachedBinaryOutputInfo
	{
		u32 format;
		std::vector<u8> binary;
	};

	// Information about an extracted shader option
	struct ShaderOption
	{
		u32 min;
		u32 max;
		u32 id;
	};

	// Returns a new string containing the source file with all includes resolved, or an empty string if an error occured.
	auto ResolveShaderIncludes(ShaderStageSourceFileInfo const& fileInfo) -> std::string;
	// Extracts all shader options from the source file and insert them into out, removes the option specifiers the source string, then returns the number of bits required to store all flags.
	auto ExtractShaderOptions(std::string& source, std::unordered_map<std::string, ShaderOption>& out) -> u32;
}
