#pragma once
#include "Logger.h"
#include <mutex>
#include <memory>
#include <string>

class LogManager {
public:
    // 日志级别枚举
    enum class Level {
        debug,
        info,
        warning,
        error,
        fatal
    };

    // 获取单例实例
    static LogManager& getInstance();

    // 获取Logger实例
    Logger& getLogger();

    // 设置最低日志级别
    void setMinLevel(Level level);

    // 记录不同级别的日志
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);

    // 禁止复制和赋值
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

private:
    // 私有构造函数
    LogManager();

    // 单例初始化函数
    static void initInstance();

    // 获取级别前缀
    std::string getLevelPrefix(Level level) const;

    // 静态成员变量
    static std::once_flag s_onceFlag;
    static std::unique_ptr<LogManager> s_instance;

    // 实例成员变量
    std::unique_ptr<Logger> m_logger;
    Level m_minLevel;
};

// 全局便捷函数
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