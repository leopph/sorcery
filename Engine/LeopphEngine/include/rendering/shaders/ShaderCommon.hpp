#pragma once

#include "Types.hpp"

#include <filesystem>
#include <optional>
#include <span>
#include <string_view>
#include <vector>


namespace leopph::internal
{
	enum class ShaderStage : u8
	{
		Vertex = 1,
		Geometry = 2,
		Fragment = 3
	};

	struct ShaderProgramSourceInfo
	{
		std::optional<std::string_view> vertex;
		std::optional<std::string_view> geometry;
		std::optional<std::string_view> fragment;
	};

	struct ShaderStageBinaryInfo
	{
		std::string_view entryPoint;
		std::span<u8 const> binary;
	};

	struct ShaderProgramBinaryInfo
	{
		std::optional<ShaderStageBinaryInfo> vertex;
		std::optional<ShaderStageBinaryInfo> geometry;
		std::optional<ShaderStageBinaryInfo> fragment;
	};

	struct ShaderProgramCachedBinaryInputInfo
	{
		u32 format;
		std::span<u8 const> binary;
	};

	struct ShaderProgramCachedBinaryOutputInfo
	{
		u32 format;
		std::vector<u8> binary;
	};

	struct ShaderSourceFileInfo
	{
		std::filesystem::path absolutePath;
		std::string content;
	};

	auto ProcessShader(ShaderSourceFileInfo const& fileInfo) -> std::string;
}
