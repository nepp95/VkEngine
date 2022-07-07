#pragma once

namespace Utility
{
	inline std::vector<char> ReadFile(const std::string& filepath)
	{
		std::ifstream file(filepath, std::ios::ate | std::ios::binary);

		CORE_ASSERT(file.is_open(), "Failed to open file!");

		size_t fileSize{ (size_t)file.tellg() };
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}
}