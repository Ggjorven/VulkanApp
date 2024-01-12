#pragma once

#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace VkApp
{

	class Log
	{
	public:
		//enum class Level
		//{
		//	None = -1, Trace, Info, Warn, Error, Fatal
		//};

		static void Init();

		//template<typename ... Args>
		//static void LogMessage(Log::Level level, const char* fmt, const Args&... args)
		//{
		//	switch (level)
		//	{
		//	case Level::Trace:
		//		spdlog::trace(fmt, args...);
		//		break;
		//	case Level::Info:
		//		spdlog::info(fmt, args...);
		//		break;
		//	case Level::Warn:
		//		spdlog::warn(fmt, args...);
		//		break;
		//	case Level::Error:
		//		spdlog::error(fmt, args...);
		//		break;
		//	case Level::Fatal:
		//		spdlog::critical(fmt, args...);
		//		break;
		//	}
		//}

		#define VKAPP_LOG_TRACE(...) spdlog::trace(__VA_ARGS__)
		#define VKAPP_LOG_INFO(...) spdlog::info(__VA_ARGS__)
		#define VKAPP_LOG_WARN(...) spdlog::warn(__VA_ARGS__)
		#define VKAPP_LOG_ERROR(...) spdlog::error(__VA_ARGS__)
		#define VKAPP_LOG_FATAL(...) spdlog::critical(__VA_ARGS__)

	private:
		static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> s_Sink;
		static std::shared_ptr<spdlog::logger> s_Logger;
	};

}