#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <vector>

namespace View {
    enum class LogLevel {
        LOG_ERROR,  // 错误，最高级别
        LOG_WARNING,    // 警告
        LOG_INFO,       // 信息
        LOG_DEBUG       // 调试，最低级别
    };

    class Logger {
    public:
        // 获取单例实例
        static Logger& getInstance();
        
        // 初始化日志系统
        void init(const std::wstring& logFilePath, LogLevel level = LogLevel::LOG_INFO);
        
        // 基本日志方法
        void log(LogLevel level, const std::wstring& message);
        
        // 不同级别的快捷方法
        void error(const std::wstring& message);
        void warning(const std::wstring& message);
        void info(const std::wstring& message);
        void debug(const std::wstring& message);
        
        // 关闭日志系统
        void close();
        
        // 设置日志级别
        void setLogLevel(LogLevel level);
        
    private:
        Logger();
        ~Logger();
        
        // 禁止拷贝和赋值
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        
        // 获取当前时间字符串
        std::wstring getCurrentTimeString();
        
        // 将日志级别转换为字符串
        std::wstring levelToString(LogLevel level);
        
        std::wofstream logFile;
        LogLevel currentLevel;
        std::mutex logMutex;
        bool initialized;
    };
    
    // 全局访问函数
    void initLog(const std::wstring& logFilePath, LogLevel level = LogLevel::LOG_INFO);
    void closeLog();
    void setLogLevel(LogLevel level);
    void logError(const std::wstring& message);
    void logWarning(const std::wstring& message);
    void logInfo(const std::wstring& message);
    void logDebug(const std::wstring& message);
}

// 宏定义，使用更方便
#define LOG_ERROR(msg) View::logError(msg) // Ensure no conflict with LogLevel::LOG_ERROR
#define LOG_WARNING(msg) View::logWarning(msg)
#define LOG_INFO(msg) View::logInfo(msg)
#define LOG_DEBUG(msg) View::logDebug(msg)
