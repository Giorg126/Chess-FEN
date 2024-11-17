#pragma once
#include <fstream>
#include <iostream>
#include <string>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Log {
public:
    static void initialize(LogLevel level);   
    static void log(LogLevel level, const std::string& message); 
    static void setLevel(LogLevel level);     
    static void terminate();                

private:
    static LogLevel currentLogLevel;   
    static std::ofstream logFile;    
    
    static bool shouldLog(LogLevel level);  
};
