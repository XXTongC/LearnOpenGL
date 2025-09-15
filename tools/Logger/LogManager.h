#pragma once
#include "Logger.h"
#include <mutex>
#include <memory>
#include <string>

class LogManager {
public:
    // ��־����ö��
    enum class Level {
        debug,
        info,
        warning,
        error,
        fatal
    };

    // ��ȡ����ʵ��
    static LogManager& getInstance();

    // ��ȡLoggerʵ��
    Logger& getLogger();

    // ���������־����
    void setMinLevel(Level level);

    // ��¼��ͬ�������־
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);

    // ��ֹ���ƺ͸�ֵ
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

private:
    // ˽�й��캯��
    LogManager();

    // ������ʼ������
    static void initInstance();

    // ��ȡ����ǰ׺
    std::string getLevelPrefix(Level level) const;

    // ��̬��Ա����
    static std::once_flag s_onceFlag;
    static std::unique_ptr<LogManager> s_instance;

    // ʵ����Ա����
    std::unique_ptr<Logger> m_logger;
    Level m_minLevel;
};

// ȫ�ֱ�ݺ���
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