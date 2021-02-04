#include "pch.h"
#include "Shader.h"

#include "Vulkan\Core.h"

Shader::Shader(const std::string& fileDir)
	:m_FileDir(fileDir)
{
	//auto hello = readFile("shader.spv");
}

std::string& Shader::readFile(const std::string& fileName)
{
	/*std::ifstream file(m_FileDir + "/" + fileName, std::ios::ate | std::ios::binary);

	VK_ASSERT(!file.is_open(), "Can't open the file!");

	std::stringstream ss;
	std::string line;

	while (std::getline(file, line))
	{
		ss << line;
	}
	*/
	std::string a = "hellpo";
	return a;
}
