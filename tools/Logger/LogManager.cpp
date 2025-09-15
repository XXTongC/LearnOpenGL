#include "LogManager.h"

// ��ʼ����̬��Ա����
std::once_flag LogManager::s_onceFlag;
std::unique_ptr<LogManager> LogManager::s_instance;

LogManager& LogManager::getInstance() {
    // ȷ����ʼ������ִֻ��һ��
    std::call_once(s_onceFlag, &LogManager::initInstance);
    return *s_instance;
}

void LogManager::initInstance() {
    // ����LogManagerʵ��
    s_instance = std::unique_ptr<LogManager>(new LogManager());
}

LogManager::LogManager() : m_minLevel(Level::debug) {
    // ����Loggerʵ��
    m_logger = std::make_unique<Logger>();
    loggerMessge(*m_logger, "LOGGER STARTED");
}

Logger& LogManager::getLogger() {
    return *m_logger;
}

void LogManager::setMinLevel(Level level) {
    m_minLevel = level;
}

void LogManager::debug(const std::string& message) {
    if (m_minLevel <= Level::debug) {
        loggerMessge(*m_logger, getLevelPrefix(Level::debug) + message);
    }
}

void LogManager::info(const std::string& message) {
    if (m_minLevel <= Level::info) {
        loggerMessge(*m_logger, getLevelPrefix(Level::info) + message);
    }
}

void LogManager::warning(const std::string& message) {
    if (m_minLevel <= Level::warning) {
        loggerMessge(*m_logger, getLevelPrefix(Level::warning) + message);
    }
}

void LogManager::error(const std::string& message) {
    if (m_minLevel <= Level::error) {
        loggerMessge(*m_logger, getLevelPrefix(Level::error) + message);
    }
}

void LogManager::fatal(const std::string& message) {
    if (m_minLevel <= Level::fatal) {
        loggerMessge(*m_logger, getLevelPrefix(Level::fatal) + message);
    }
}

std::string LogManager::getLevelPrefix(Level level) const {
    switch (level) {
    case Level::debug:   return "[DEBUG] ";
    case Level::info:    return "[INFO] ";
    case Level::warning: return "[WARNING] ";
    case Level::error:   return "[ERROR] ";
    case Level::fatal:   return "[FATAL] ";
    default:             return "[UNKNOWN] ";
    }
}