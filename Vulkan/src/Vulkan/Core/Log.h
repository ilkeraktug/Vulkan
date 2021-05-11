#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

class Log
{
public:
	static void Init();

	inline static std::shared_ptr<spdlog::logger>& GetClient() { return m_LoggerClient; }
	inline static std::shared_ptr<spdlog::logger>& GetCore() { return m_LoggerCore; }
private:
	static std::shared_ptr<spdlog::logger> m_LoggerClient;
	static std::shared_ptr<spdlog::logger> m_LoggerCore;
};

// Core Loggers
#define VK_CORE_INFO(...)	Log::GetCore()->info(__VA_ARGS__)
#define VK_CORE_TRACE(...)	Log::GetCore()->trace(__VA_ARGS__)
#define VK_CORE_WARN(...)	Log::GetCore()->warn(__VA_ARGS__)
#define VK_CORE_ERROR(...)	Log::GetCore()->error(__VA_ARGS__)
#define VK_CORE_FATAL(...)	Log::GetCore()->critical(__VA_ARGS__)

// Client Loggers
#define VK_INFO(...)	Log::GetClient()->info(__VA_ARGS__)
#define VK_TRACE(...)	Log::GetClient()->trace(__VA_ARGS__)
#define VK_WARN(...)	Log::GetClient()->warn(__VA_ARGS__)
#define VK_ERROR(...)	Log::GetClient()->error(__VA_ARGS__)
#define VK_FATAL(...)	Log::GetClient()->critical(__VA_ARGS__)