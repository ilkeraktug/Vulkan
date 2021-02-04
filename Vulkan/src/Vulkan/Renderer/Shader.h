#pragma once

class Shader
{
public:
	Shader(const std::string& fileDir);
private:
	std::string& readFile(const std::string& fileName);
private:
	std::string m_FileDir;
};