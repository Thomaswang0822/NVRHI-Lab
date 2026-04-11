#pragma once

#include <iostream>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace nvrhi_lab {

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

}
