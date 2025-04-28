#include "pch.h"
#include "Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

namespace View {
    Logger::Logger() : currentLevel(LogLevel::LOG_INFO), initialized(false) {
    }
    
    Logger::~Logger() {
        close();
    }
    
    Logger& Logger::getInstance() {
        static Logger instance;
        return instance;
    }
    
    void Logger::init(const std::wstring& logFilePath, LogLevel level) {
        std::lock_guard<std::mutex> lock(logMutex);
        
        if (initialized) {
            close();
        }
        
        logFile.open(logFilePath.c_str(), std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            std::wcerr << L"Failed to open log file: " << logFilePath << std::endl;
            return;
        }
        
        currentLevel = level;
        initialized = true;
        
    }
    
    void Logger::close() {
        std::lock_guard<std::mutex> lock(logMutex);
        
        if (initialized && logFile.is_open()) {
            logFile.close();
        }
        
        initialized = false;
    }
    
    void Logger::log(LogLevel level, const std::wstring& message) {
        if (!initialized || !logFile.is_open() || level > currentLevel) {
            return;
        }
        std::lock_guard<std::mutex> lock(logMutex);
        
        std::wstring timestamp = getCurrentTimeString();
        std::wstring levelStr = levelToString(level);
        
        logFile << timestamp << L" [" << levelStr << L"] " << message << std::endl;
        logFile.flush();
    }
    
    void Logger::error(const std::wstring& message) {
        log(LogLevel::LOG_ERROR, message);
    }
    
    void Logger::warning(const std::wstring& message) {
        log(LogLevel::LOG_WARNING, message);
    }
    
    void Logger::info(const std::wstring& message) {
        log(LogLevel::LOG_INFO, message);
    }
    
    void Logger::debug(const std::wstring& message) {
        log(LogLevel::LOG_DEBUG, message);
    }
    
    void Logger::setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(logMutex);
        currentLevel = level;
    }
    
    std::wstring Logger::getCurrentTimeString() {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        std::tm now_tm;
        localtime_s(&now_tm, &now_time_t);
        
        std::wostringstream oss;
        oss << std::put_time(&now_tm, L"%Y-%m-%d %H:%M:%S");
        oss << L"." << std::setfill(L'0') << std::setw(3) << now_ms.count();
        
        return oss.str();
    }
    
    std::wstring Logger::levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::LOG_ERROR:
                return L"ERROR";
            case LogLevel::LOG_WARNING:
                return L"WARNING";
            case LogLevel::LOG_INFO:
                return L"INFO";
            case LogLevel::LOG_DEBUG:
                return L"DEBUG";
            default:
                return L"Unknown";
        }
    }
    
    // 全局函数实现
    void initLog(const std::wstring& logFilePath, LogLevel level) {
        Logger::getInstance().init(logFilePath, level);
    }
    
    void closeLog() {
        Logger::getInstance().close();
    }
    
    void setLogLevel(LogLevel level) {
        Logger::getInstance().setLogLevel(level);
    }
    
    void logError(const std::wstring& message) {
        Logger::getInstance().error(message);
    }
    
    void logWarning(const std::wstring& message) {
        Logger::getInstance().warning(message);
    }
    
    void logInfo(const std::wstring& message) {
        Logger::getInstance().info(message);
    }
    
    void logDebug(const std::wstring& message) {
        Logger::getInstance().debug(message);
    }
}
