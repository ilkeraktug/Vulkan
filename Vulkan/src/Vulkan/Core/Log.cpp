#include "pch.h"
#include "Log.h"

std::shared_ptr<spdlog::logger> Log::m_LoggerClient;
std::shared_ptr<spdlog::logger> Log::m_LoggerCore;

void Log::Init()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");
	
	m_LoggerClient = spdlog::stdout_color_mt("APP");
	spdlog::set_level(spdlog::level::trace);

	m_LoggerCore = spdlog::stdout_color_mt("CORE");
	spdlog::set_level(spdlog::level::trace);
}
