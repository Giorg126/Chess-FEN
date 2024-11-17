#include "Log.h"
#include <ctime>
#include <iomanip>
#include <sstream>

LogLevel Log::currentLogLevel = LogLevel::INFO;
std::ofstream Log::logFile;

void Log::initialize(LogLevel level) {
    currentLogLevel = level;

    std::remove("log.txt");
    logFile.open("log.txt", std::ios::out | std::ios::app); 
    
    if (logFile.is_open()) {
        log(LogLevel::INFO, "Logging Initialized");
    } else {
        std::cerr << "Error opening log file" << std::endl;
    }
}

void Log::log(LogLevel level, const std::string& message) {
        // Log to console
        std::cout << message << std::endl;

        // Log to file if the file is open
        if (logFile.is_open()) {
            logFile << message << std::endl;
        }
    
}

void Log::setLevel(LogLevel level) {
    currentLogLevel = level;
}


