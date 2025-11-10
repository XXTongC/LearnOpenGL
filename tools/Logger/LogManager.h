#pragma once
#include "Logger.h"
#include <mutex>
#include <memory>
#include <string>

class LogManager {
public:
    enum class Level {
        debug,
        info,
        warning,
        error,
        fatal
    };

    static LogManager& getInstance();

    Logger& getLogger();

    void setMinLevel(Level level);

    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);

    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

private:
    LogManager();
    static void initInstance();
    std::string getLevelPrefix(Level level) const;

    static std::once_flag s_onceFlag;
    static std::unique_ptr<LogManager> s_instance;

    std::unique_ptr<Logger> m_logger;
    Level m_minLevel;
};

// log function
inline void LogDebug(const std::string& message) {
    LogManager::getInstance().debug(message);
}

inline void LogInfo(const std::string& message) {
    LogManager::getInstance().info(message);
}

inline void LogWarning(const std::string& message) {
    LogManager::getInstance().warning(message);
}

inline void LogError(const std::string& message) {
    LogManager::getInstance().error(message);
}

inline void LogFatal(const std::string& message) {
    LogManager::getInstance().fatal(message);
}