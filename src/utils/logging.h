#pragma once

#include <iostream>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace nvrhi_lab {

/// Simple logging functions that output to both the console and the Visual Studio Output window (on Windows).
/// They will be called by both MessageCallback below (which is internally used by NVRHI) and by the application code itself.

inline void LogFatal(const std::string& message) {
    std::string fullMessage = "[Fatal] " + message + "\n";
#ifdef _WIN32
    OutputDebugStringA(fullMessage.c_str());
#endif
    std::cerr << fullMessage;
}

inline void LogError(const std::string& message) {
    std::string fullMessage = "[Error] " + message + "\n";
#ifdef _WIN32
    OutputDebugStringA(fullMessage.c_str());
#endif
    std::cerr << fullMessage;
}

inline void LogWarning(const std::string& message) {
    std::string fullMessage = "[Warning] " + message + "\n";
#ifdef _WIN32
    OutputDebugStringA(fullMessage.c_str());
#endif
    std::cerr << fullMessage;
}

inline void LogInfo(const std::string& message) {
    std::string fullMessage = "[Info] " + message + "\n";
#ifdef _WIN32
    OutputDebugStringA(fullMessage.c_str());
#endif
    std::cout << fullMessage;
}

/**
 * @brief "IMessageCallback should be implemented by the application."
 */
class MessageCallback : public nvrhi::IMessageCallback {
public:
    void message(nvrhi::MessageSeverity severity, const char* messageText) override {
        switch (severity) {
        case nvrhi::MessageSeverity::Fatal:
            LogFatal(messageText);
            std::terminate();
            break;
        case nvrhi::MessageSeverity::Error:
            LogError(messageText);
            break;
        case nvrhi::MessageSeverity::Warning:
            LogWarning(messageText);
            break;
        case nvrhi::MessageSeverity::Info:
            LogInfo(messageText);
            break;
        }
    }
};

}
